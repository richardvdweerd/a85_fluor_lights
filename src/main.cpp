//Includes
#include <Arduino.h>
#include "DccLib.h"
#include "FluorescentLights.h"
#include <Adafruit_NeoPixel.h>
#include "trainsettings.h"

/********************************************************************************************************
 * Version history
 * 
 * 10 feb 2021   v1.0  Stable version - with external trainsettings in header file
 * 14 feb 2021   v1.1  Stable version
 *                              added function for cabin lights on / off
 *                              added support for cabin front and tail cabin (for eg railbus with only one unit)
 *                     
 * 
 *******************************************************************************************************/

//#define DEBUG_MAIN

#define PIN_FORWARD PB3
#define PIN_BACKWARD PB4

const uint16_t numPixels = NUM_PIXELS; 
const uint8_t  neoPin = PB0;

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


Adafruit_NeoPixel strip = Adafruit_NeoPixel(numPixels, neoPin, NEO_GRB + NEO_KHZ800);
// FluorescentLights lights[numPixels - numCabinLights]; // use less pixels for flickering lights when there are cab lights
FluorescentLights lights[numPixels]; 

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
  // for (uint16_t i = 0; i < numPixels; i++)    // set up all pixels, including the cabin lights, if there's one
  //   strip.setPixelColor(i, 0,0,0);
  strip.clear();  // clear all pixels, including cab lights, if there are any
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

  if ( tailActive)
  {
    uint8_t temp = cabinLightFront;
    cabinLightFront = cabinLightTail;
    cabinLightTail = temp;
  }
}

void loop() {
  static bool forceOff=true;
  static uint8_t oldCabinLightOn = 0;
  static bool stripChanged = true;
  static uint8_t oldDir = 0;
  uint8_t coachLightsOn = (uint8_t) dccGetFunction(FUNCTION_COACH_LIGHT);
  uint8_t cabinLightsOn = dccGetFunction(FUNCTION_CABIN_LIGHT);
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
    digitalWrite(PIN_FORWARD, LOW ^ dir ^ tailActive); 
    digitalWrite(PIN_BACKWARD, HIGH ^ dir ^tailActive);
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
  #ifdef CABIN_LIGHTS      // we can skip this entire code if there is not a cabin light
  // handle cabin light
  if (cabinLightsOn && /*dir &&*/ speed <= CABIN_ON_MAX_SPEED)
  {  // turn cabin light on!
    if ( !oldCabinLightOn || dir != oldDir)
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
    if (oldCabinLightOn)
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
  for (uint16_t i = cabinLightFront; i < numPixels - cabinLightTail; i++)
  {
      lights[i].switchLight(coachLightsOn); 
      
      // r, g, b = 22,21,18
      strip.setPixelColor(i /* *stepValue*/, lights[i]._state * COACH_RED, lights[i]._state * COACH_GREEN, lights[i]._state * COACH_BLUE);

      if (lights[i]._on)
        countOn++;
  }
  if (coachLightsOn && !forceOff)
  {
    stripChanged = true;
    if (countOn ==  numPixels - numCabinLights) // minus the cabine lights, if there are any
      forceOff = true;
  }
  if ((!coachLightsOn && forceOff))
  {
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


