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

#define PIN_FORWARD PB3
#define PIN_BACKWARD PB4

// #define USE_NEOPIXEL


// const uint16_t maxNumPixels = 9;
// const uint8_t stepValue = 1;
const uint16_t numPixels = NUM_PIXELS; 
const uint8_t  neoPin = PB0;
#ifdef CAB_LIGHT
    const bool cab_light = true;
#else
    const bool cab_light = false;
#endif

Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, neoPin, NEO_GRB + NEO_KHZ800);
//FluorescentLights lights[numPixels - (cab_light ? 1 : 0)]; // use 1 less pixel for flickering lights when there is a cab light
FluorescentLights lights[numPixels]; // use 1 less pixel for flickering lights when there is a cab light

void setup() {
  // pinMode(neoPin, OUTPUT); // is set in strip.begin()

  // pinMode(PIN_FORWARD, OUTPUT);
  // pinMode(PIN_BACKWARD, OUTPUT);
  //DDRB |= 0b00011000;
  DDRB |= ((1 << PIN_FORWARD) | (1 << PIN_BACKWARD)); // set pins for front and back to output
  #ifdef DEBUG_MAIN
    // signalling board is ready
    // pinMode(PB1, OUTPUT);
    DDRB |= (1 << PB1); // set pin PB1 to output
    for(int i = 0; i < 5; i++)
    {
      digitalWrite(PB1, HIGH);
      delay(100);
      digitalWrite(PB1, LOW);
      delay(100);
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
  for (int j = PIN_FORWARD; j <= PIN_BACKWARD; j++)
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
}

void loop() {
  static bool forceOff=true;
  static bool cabinLight = false;
  static bool stripChanged = true;
  int lightsOn = (int) dccGetFunction(FL_LIGHT_FUNCTION);
  uint8_t dir = dccGetDirection();
  uint8_t speed = dccGetSpeed();
  uint8_t f0 = dccGetFunction(0);

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
        digitalWrite(PIN_FORWARD, LOW);
        digitalWrite(PIN_BACKWARD, HIGH);
      #else
        digitalWrite(PIN_FORWARD, HIGH); 
        digitalWrite(PIN_BACKWARD, LOW);
      #endif
    }
    else
    {
      #ifdef TAIL
        digitalWrite(PIN_FORWARD, HIGH); 
        digitalWrite(PIN_BACKWARD, LOW);
      #else
        digitalWrite(PIN_FORWARD, LOW);
        digitalWrite(PIN_BACKWARD, HIGH);
      #endif
    }
  }
  else
  {  // light is off
    digitalWrite(PIN_FORWARD, LOW);
    digitalWrite(PIN_BACKWARD, LOW);
  }

  /****************************************************************************************************
   * 
   * handle cabin light
   * 
   ****************************************************************************************************/
  #ifdef CAB_LIGHT      // we can skip this entire code if there is not a cabin light
  // handle cabin light
  if (f0 && dir && speed <= CAB_ON_MAX_SPEED)
  {  // turn cabin light on!
    if (!cabinLight)
    {  // light was off, turn on
      strip.setPixelColor(numPixels-1, CAB_RED, CAB_GREEN, CAB_BLUE);
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
  for (uint16_t i = 0; i < numPixels - (cab_light ? 1 : 0); i++)
  {
      lights[i].switchLight(lightsOn); 
      
      // r, g, b = 22,21,18
      strip.setPixelColor(i /* *stepValue*/, lights[i]._state * FL_RED, lights[i]._state * FL_GREEN, lights[i]._state * FL_BLUE);

      if (lights[i]._on)
        countOn++;
  }
  // if((lightsOn && (countOn < ((numPixels+1)/2))) || (!lightsOn && (countOn > 0)))
  if (lightsOn && !forceOff)
  {
    //strip.show();
    stripChanged = true;
    if (countOn ==  numPixels - (cab_light ? 1 : 0)) // minus the cabine light, if there is one
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


