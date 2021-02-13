//Includes
#include <Arduino.h>
#include "DccLib.h"
#include "FluorescentLights.h"
#include <Adafruit_NeoPixel.h>
#include "trainsettings.h"

/********************************************************************************************************
 * Version history
 * 
 * 10-02-2021   v1.0  Stable version - with external trainsettings
 * 
 * 
 *******************************************************************************************************/


// #define INTERRUPTPIN PCINT2 //this is PB1 per the schematic
// #define PCINT_VECTOR PCINT0_vect  //this step is not necessary
// #define DATADIRECTIONPIN DDB2 //Page 64 of data sheet
// #define PORTPIN PB2 //Page 64
// #define READPIN PINB2 //page 64
// #define LEDPIN 1 //PB1

//#define DEBUG_MAIN

#define PIN_FRONT_LIGHT PB3
#define PIN_REAR_LIGHT  PB4

#define PIXEL_PROGRAMMING 1
#define PIXEL_CHANGE      0
// const uint16_t maxNumPixels = 9;
// const uint8_t stepValue = 1;
const uint16_t numPixels = NUM_PIXELS; 
const uint8_t  neoPin = PB0;
#ifdef CABIN_LIGHT
    const bool cab_light = true;
#else
    const bool cab_light = false;
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, neoPin, NEO_GRB + NEO_KHZ800);
FluorescentLights lights[numPixels - (cab_light ? 1 : 0)]; // use 1 less pixel for flickering lights when there is a cab light
// FluorescentLights lights[numPixels]; // use 1 less pixel for flickering lights when there is a cab light



/*static*/ bool stripChanged = true;



// remote programming vars
bool digits[9];
bool programming_mode = false;
uint8_t command[] = { 0, 0, 0, 0 };
uint8_t command_functions[] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
uint8_t command_length = 0;
uint8_t command_count_nine = 0;    // 5 times 9 is reset command buffer


void flash_light(uint8_t pinNumber, uint8_t numberOfFlashes = 1)
{
  for( uint8_t i = 0; i < numberOfFlashes; i++ )
  {
    digitalWrite( pinNumber, HIGH );
    delay(250);
    digitalWrite( pinNumber, LOW );
    // if ( i < ( numberOfFlashes - 1 ) )
    // {
      delay(250);
    // }
  }
}

void flash_command_buffer()
{
  
  for ( uint8_t i = 0; i < sizeof( command ); i++ )
  {
    flash_light( PIN_REAR_LIGHT, i+1);
    flash_light( PIN_FRONT_LIGHT, command[ i ]);
    delay(500);
  }
}

void flash_number(uint16_t number)
{
  command[ 0 ] = number / 1000;
  number -= command[ 0 ] * 1000;
  command[ 1 ] = number / 100;
  number -= command[ 1 ] * 100;
  command[ 2 ] = number / 10;
  number -= command[ 2 ] * 10;
  command[ 3 ] = number;
  flash_command_buffer();
}

void command_append(uint8_t numberToInsert = 0)
{
  uint8_t lastIndex = sizeof( command ) - 1;
  for ( uint8_t i = 0; i < lastIndex; i++ )
    command[ i ] = command[ i + 1 ];
  command[ lastIndex ] = numberToInsert;
}

void command_reset()
{
  programming_mode = false;
  command_length = command_count_nine = 0;
  for ( uint8_t i = 0; i < sizeof( command ); i++ )
  {
    command[ i ] = 0;
  }
  for ( uint8_t i = 0; i < sizeof( command_functions ); i++ )
  {
    command_functions[ i ] = 0;
  }
  command_append();
  // strip.setPixelColor(PIXEL_PROGRAMMING, 0);
  // strip.show();
  // for ( int i = 0; i < NUM_PIXELS - (cab_light ? 1 : 0 ); i++)
  //   lights[ i ]._state = 0;
  // stripChanged = true;
}

uint16_t command_value()
{
  uint16_t sum = 0;
  for (uint8_t i = 0; i < sizeof( command ); i++ )
  {
    sum = sum * 10 + command[ i ];
  }
  return sum;
}

void flash_pixel(uint16_t pixelNumber, uint32_t color, uint16_t numberFlashes = 1 )
{
  for ( uint16_t i = 0; i < numberFlashes; i++ )
  {
    strip.setPixelColor(pixelNumber, color);
    strip.show();
    delay(200);
    strip.setPixelColor(pixelNumber, 0);
    strip.show();
    delay(200);
  }
}


void command_read_change()
{
  static unsigned long last_time_function_key = 0UL;

  for( uint8_t i = 0; i <= (uint8_t) 10; i++)
  {
    uint8_t functionState = dccGetFunction( i );
    if ( functionState != command_functions[ i ] )
    {
      if(programming_mode)
        flash_pixel(PIXEL_CHANGE, 0x800000);
      command_append( i % 10 );  // function 'i' has changed, add function as number to the command buffer
      // command[3] = i % 10;  // function 'i' has changed, add function as number to the command buffer
      command_functions[ i ] = functionState; // keep track of current state of function
      last_time_function_key = millis();    
      if ( command_value() == LOK_ADDRESS )
      {
        flash_light(PIN_FRONT_LIGHT, 1);
        programming_mode = true;
        // strip.clear();
        // strip.setPixelColor(PIXEL_PROGRAMMING, 0x0000FF);
        // stripChanged = true;
      }

      // 5 times a pressed 9 is reset
      if ( i != 9 )  
      {
        command_count_nine = 0;
      }
      else
      {
        command_count_nine++;
        // flash_light(PIN_REAR_LIGHT);
      }
    
      if ( command_count_nine == 5)
      {
        command_reset();
        flash_light(PIN_REAR_LIGHT, 3);
      }
    //   // flash_light(PIN_REAR_LIGHT, command_length);
    //   // if ( command_length == 4 )
    //   // {
        

        
    //   // }
    }
  }

  // if ( programming_mode && ( millis() - last_time_function_key > 5000UL ) )
  // {
  //   programming_mode = false;
  //   command_reset();
  //   strip.clear();
  //   stripChanged = true;
  // }
}


void setup() {
  // pinMode(neoPin, OUTPUT); // is set in strip.begin()

  // pinMode(PIN_FRONT_LIGHT, OUTPUT);
  // pinMode(PIN_REAR_LIGHT, OUTPUT);
  //DDRB |= 0b00011000;
  DDRB |= (( 1 << PIN_FRONT_LIGHT ) | ( 1 << PIN_REAR_LIGHT )); // set pins for front and back to output
  #ifdef DEBUG_MAIN
    // signalling board is ready
    // pinMode(PB1, OUTPUT);
    DDRB |= ( 1 << PB1 ); // set pin PB1 to output
    for(int i = 0; i < 5; i++)
    {
      digitalWrite( PB1, HIGH );
      delay( 100 );
      digitalWrite( PB1, LOW );
      delay( 100 );
    }
  #endif

  // setup lights
  strip.begin();
  for (uint16_t i = 0; i < numPixels; i++)    // set up all pixels, including the cabin lights, if there's one
    strip.setPixelColor(i, 0,0,0);
  strip.show();

  dccInit(PB2, EDGE_RISING);  // default pin 2 and edge rising
  dccSetDecoder(LOK_ADDRESS, DT_MULTIFUNCTION_DECODER); // set decoder address and type

  #ifdef DEBUG_MAIN
  for (int j = PIN_FRONT_LIGHT; j <= PIN_REAR_LIGHT; j++)
  {
    for (int i = 0; i < 4; i++)
    {
      digitalWrite(j, HIGH);
      delay(500);
      digitalWrite(j, LOW);
      delay(500);
    }
  }
  #endif
  // flash_light(PIN_REAR_LIGHT, 5);
}

void loop() {
  static bool forceOff = true;
  static bool cabinLight = false;
  
  int lightsOn = (int) dccGetFunction(COACH_LIGHT_FUNCTION);
  uint8_t dir = dccGetDirection();
  uint8_t speed = dccGetSpeed();
  uint8_t f0 = dccGetFunction(0);

  /****************************************************************************************************
   * 
   * read changes in function status
   * 
   ****************************************************************************************************/
  command_read_change();

  if (!programming_mode)
  {

    /****************************************************************************************************
     * 
     * handle front and tail lights
     * 
     ****************************************************************************************************/
    if (f0) 
    { 
      // light is on
      if (dir)
      {
        #ifdef TAIL
          digitalWrite(PIN_FRONT_LIGHT, LOW);
          digitalWrite(PIN_REAR_LIGHT, HIGH);
        #else
          digitalWrite(PIN_FRONT_LIGHT, HIGH); 
          digitalWrite(PIN_REAR_LIGHT, LOW);
        #endif
      }
      else
      {
        #ifdef TAIL
          digitalWrite(PIN_FRONT_LIGHT, HIGH); 
          digitalWrite(PIN_REAR_LIGHT, LOW);
        #else
          digitalWrite(PIN_FRONT_LIGHT, LOW);
          digitalWrite(PIN_REAR_LIGHT, HIGH);
        #endif
      }
    }
    else
    {  // light is off
      digitalWrite(PIN_FRONT_LIGHT, LOW);
      digitalWrite(PIN_REAR_LIGHT, LOW);
    }

    /****************************************************************************************************
     * 
     * handle cabin light
     * 
     ****************************************************************************************************/
    #ifdef CABIN_LIGHT      // we can skip this entire code if there is not a cabin light
    // handle cabin light
    if (f0 && dir && speed <= CABIN_ON_MAX_SPEED)
    {  // turn cabin light on!
      if (!cabinLight)
      {  // light was off, turn on
        strip.setPixelColor(numPixels-1, CABIN_RED, CABIN_GREEN, CABIN_BLUE);
        cabinLight = true;
        stripChanged = true;
      }
    }
    else
    { // turn cabin light off
      if (cabinLight)
      {
        strip.setPixelColor(NUM_PIXELS-1, 0,0,0);
        cabinLight = false;
        stripChanged = true;
      }
    }
    #endif // CAB_LIGHT

    /****************************************************************************************************
     * 
     * handle fluorenscent lights
     * 
     ****************************************************************************************************/
    uint16_t countOn = 0;
    // loop through all pixels except cabin light
    for ( uint16_t i = 0; i < (numPixels - (cab_light ? 1 : 0)); i++ )
    {
        lights[ i ].switchLight( lightsOn ); 
        
        // r, g, b was 22,21,18
        strip.setPixelColor( i , lights[i]._state * COACH_RED, lights[i]._state * COACH_GREEN, lights[i]._state * COACH_BLUE );

        if (lights[i]._on)
          countOn++;
    }
    // if((lightsOn && (countOn < ((numPixels+1)/2))) || (!lightsOn && (countOn > 0)))
    if (lightsOn && !forceOff)
    {
      //strip.show();
      stripChanged = true;
      if (countOn == numPixels - (cab_light ? 1 : 0)) // minus the cabine light, if there is one
        forceOff = true;
    }
    if ((!lightsOn && forceOff))
    {
      //strip.show();
      stripChanged = true;
      forceOff = false;
    }

    // handle changes to the strip
    if (stripChanged)
    {
      strip.show();
      stripChanged = false;
    }
  }
  else
  // programming mode
  {
    // if (command_value == readRegister( REG_Primary_Address ) )
    // {
    //   command_reset();
    // }

  }
}


