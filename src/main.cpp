//Includes
#include <Arduino.h>
#include "DccLib.h"
#include "FluorescentLights.h"
#if defined( MEGATINYCORE_SERIES )
  // #include <tinyNeoPixel.h>
  #error "activate include"
#endif
#if defined( __AVR_ATtiny85__ )
  #include <Adafruit_NeoPixel.h>
#endif

#include "trainsettings.h"


#define __VERSION_FLUOR_LIGHTS__ "01.02"

/********************************************************************************************************
 * Version history
 * 
 * 10 feb 2021   v1.0  Stable version - with external trainsettings in header file
 * 14 feb 2021   v1.1  Stable version
 *                              added function for cabin lights on / off
 *                              added support for cabin front and tail cabin (for eg railbus with only one unit)
 * 06 mar 2021   v1.2  Major adaption for ATtiny1616 - Reworked the dcclib                   
 * 
 *******************************************************************************************************/

//#define DEBUG_MAIN
#if defined( __AVR_ATtiny85__ )
  #define PIN_HEAD_LIGHT  PB3
  #define PIN_TAIL_LIGHT  PB4
  #define PIN_DCC_SIGNAL  PB2
  #define PIN_NEOPIXELS   PB0
  #define ISC_INTERRUPT   EDGE_RISING

#elif defined( __AVR_ATtiny1616__ )

  // for pinout diagram see folder 'ATtiny1616 datasheet'
  // or  https://github.com/SpenceKonde/megaTinyCore/issues/24
  //
  //
  // how to setup/program the T1616
  // https://www.electronics-lab.com/project/using-new-attiny-processors-arduino-ide-attiny412-attiny1614-attiny3216-attiny1616-attiny3217/


  #define PIN_HEAD_LIGHT  PIN_PA5
  #define PIN_TAIL_LIGHT  PIN_PA6
  #define PIN_DCC_SIGNAL  PIN_PA2
  #define PIN_NEOPIXELS   PIN_PA4
  #define ISC_INTERRUPT   EDGE_RISING
  
  #warning "Chip is 1616 - definition is not completed yet"
#else
  #error "Chip is not defined correctly"
#endif

const uint16_t numPixels = NUM_PIXELS; 

#if defined(CABIN_LIGHT_FRONT)
  #define CABIN_LIGHTS
  uint8_t cabinLightFront = 1;
#else
  uint8_t cabinLightFront = 0;
#endif
#if defined(CABIN_LIGHT_TAIL)
  #ifndef CABIN_LIGHTS
    #define CABIN_LIGHTS
  #endif
  uint8_t cabinLightTail = 1;
#else
  uint8_t cabinLightTail = 0;
#endif
uint8_t numCabinLights = cabinLightFront + cabinLightTail;

#if defined(TAIL)
  const uint8_t tailActive = 1;
#else
  const uint8_t tailActive = 0;
#endif

// IMPORTANT: To reduce NeoPixel burnout risk, add 1000 uF capacitor across
// pixel power leads, add 300 - 500 Ohm resistor on first pixel's data input
// and minimize distance between Arduino and first pixel.  Avoid connecting
// on a live circuit...if you must, connect GND first.

// pinMode is set in strip.begin()
#if defined( __AVR_ATtiny85__ )
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, (uint8_t) PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);
#elif defined( __AVR_ATtiny1616__ )
  tinyNeoPixel strip = tinyNeoPixel(numPixels, (uint8_t) PIN_NEOPIXELS, NEO_GRB + NEO_KHZ800);
#endif
// FluorescentLights lights[numPixels - numCabinLights]; // use less pixels for flickering lights when there are cab lights
FluorescentLights lights[numPixels]; 

/****************************************************************************************************
 * 
 * setup()
 * 
 ****************************************************************************************************/
void setup() {
  #if defined( __AVR_ATtinyxy16)
    //mcu_init(); // set all pins to low_power input
  #endif
  
  // setup the pins for head and tail lights, pin for ledstrip is set in strip.begin()
  pinMode(PIN_HEAD_LIGHT, OUTPUT);
  pinMode(PIN_TAIL_LIGHT, OUTPUT);

  // setup neopixels 
  strip.begin();
  strip.clear();  // clear all pixels, including cab lights, if there are any
  strip.show();

  // setup dcc
  dccInit(PIN_DCC_SIGNAL, ISC_INTERRUPT);  // default pin 2 and edge rising
  dccSetDecoder(LOK_ADDRESS, DT_MULTIFUNCTION_DECODER); // set decoder address and type

  // swap front and back when it's the tail
  if ( tailActive)
  {
    uint8_t temp = cabinLightFront;
    cabinLightFront = cabinLightTail;
    cabinLightTail = temp;
  }
}

/****************************************************************************************************
 * 
 * loop()
 * 
 ****************************************************************************************************/
void loop() {
  static bool     forceOff        = true;
  static uint8_t  oldCabinLightOn = 0;
  static bool     stripChanged    = true;
  static uint8_t  oldDir          = 0;
  uint8_t         coachLightsOn   = (uint8_t) dccGetFunction(FUNCTION_COACH_LIGHT);
  uint8_t         cabinLightsOn   = dccGetFunction(FUNCTION_CABIN_LIGHT);
  uint8_t         dir             = dccGetDirection();
  uint8_t         speed           = dccGetSpeed();
  uint8_t         f0              = dccGetFunction(0);

  /****************************************************************************************************
   * 
   * handle front and tail lights
   * 
   ****************************************************************************************************/
  if ( f0 ) 
  { 
    // light is on. switch on head or tail light, depending on direction
    digitalWrite(PIN_HEAD_LIGHT, LOW ^ dir ^ tailActive); 
    digitalWrite(PIN_TAIL_LIGHT, HIGH ^ dir ^tailActive);
  }
  else
  {  // light is off. switch of head and tail light
    digitalWrite(PIN_HEAD_LIGHT, LOW);
    digitalWrite(PIN_TAIL_LIGHT, LOW);
  }

  /****************************************************************************************************
   * 
   * handle cabin lights
   * 
   ****************************************************************************************************/
  #ifdef CABIN_LIGHTS      // we can skip this entire code if there is no cabin light
    // handle cabin light
    if ( cabinLightsOn && /*dir &&*/ speed <= CABIN_ON_MAX_SPEED )
    {  // turn cabin light on!
      if ( !oldCabinLightOn || dir != oldDir )
      {  // light was off, or direction was changed turn on
        if ( !( dir ^ tailActive ) )
        {
          if ( cabinLightFront ) strip.setPixelColor(             0, CABIN_RED, CABIN_GREEN, CABIN_BLUE );
          if ( cabinLightTail  ) strip.setPixelColor( numPixels - 1, 0, 0, 0 );
        }
        else
        {
          if ( cabinLightTail  ) strip.setPixelColor( numPixels - 1, CABIN_RED, CABIN_GREEN, CABIN_BLUE );
          if ( cabinLightFront ) strip.setPixelColor(             0, 0, 0, 0 );
        }
        oldCabinLightOn = true;
        oldDir = dir;
        stripChanged = true;
      }
    }
    else
    { // turn cabin light off
      if ( oldCabinLightOn )
      {
        if ( cabinLightFront ) strip.setPixelColor(             0, 0, 0, 0 ); // turn tail cabin front off
        if ( cabinLightTail  ) strip.setPixelColor( numPixels - 1, 0, 0, 0 ); // turn tail cabin front off
        oldCabinLightOn = false;
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
  for ( uint16_t i = cabinLightFront; i < numPixels - cabinLightTail; i++ )
  {
      lights[i].switchLight( coachLightsOn ); 
      
      strip.setPixelColor( i , lights[i]._state * COACH_RED, lights[i]._state * COACH_GREEN, lights[i]._state * COACH_BLUE );

      if ( lights[i]._on )
        countOn++;
  }

  if ( coachLightsOn && !forceOff )
  {
    stripChanged = true;
    if ( countOn ==  numPixels - numCabinLights ) // minus the cabine lights, if there are any
      forceOff = true;
  }

  if ( (!coachLightsOn && forceOff) )
  {
    stripChanged = true;
    forceOff = false;
  }

  // handle changes to the strip
  if (stripChanged)
  {
    strip.show( );
    stripChanged = false;
  }
}


