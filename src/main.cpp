//Includes
#include <Arduino.h>
#include "DccLib.h"
#include "FluorescentLights.h"
// #include "WELL512a.h"
#include <Adafruit_NeoPixel.h>

// #define INTERRUPTPIN PCINT2 //this is PB1 per the schematic
// #define PCINT_VECTOR PCINT0_vect  //this step is not necessary
// #define DATADIRECTIONPIN DDB2 //Page 64 of data sheet
// #define PORTPIN PB2 //Page 64
// #define READPIN PINB2 //page 64
// #define LEDPIN 1 //PB1

//#define DEBUG_MAIN
#define PIN_FORWARD PB3
#define PIN_BACKWARD PB4

#define USE_NEOPIXEL

#ifdef USE_NEOPIXEL
  const uint16_t maxNumPixels = 9;
  // const uint8_t stepValue = 1;
  const uint16_t numPixels = 9; 
  const uint8_t  neoPin = PB0;
  Adafruit_NeoPixel strip = Adafruit_NeoPixel(maxNumPixels, neoPin, NEO_GRB + NEO_KHZ800);
  FluorescentLights lights[numPixels];
#else
  uint8_t pins[]
  {
    PB0,
//    PB1,
    PB3,
    PB4,
  };
  const int numPins = 3;
  FluorescentLights lights[numPins];
#endif

void setup() {
  // pinMode(neoPin, OUTPUT); // is set in strip.begin()

  // pinMode(PIN_FORWARD, OUTPUT);
  // pinMode(PIN_BACKWARD, OUTPUT);
  DDRB |= 0b00011000;

  #ifdef DEBUG_MAIN
    // signalling board is ready
    pinMode(PB1, OUTPUT);
    for(int i = 0; i < 5; i++)
    {
      digitalWrite(PB1, HIGH);
      delay(100);
      digitalWrite(PB1, LOW);
      delay(100);
    }
  #endif

  // setup lights
  #ifdef USE_NEOPIXEL
    strip.begin();
    for (uint16_t i = 0; i < maxNumPixels; i++)
      strip.setPixelColor(i, 0,0,0);
    strip.show();
  #else  
    for (int i = 0; i < numPins; i++)
      lights[i]._pin = pins[i];
  #endif

 
  dccInit(PB2, EDGE_RISING);  // default pin 2 and edge rising
  dccSetDecoder(9, DT_MULTIFUNCTION_DECODER); // set decoder address and type

  #ifdef DEBUG_MAIN
  // for (int j = PIN_FORWARD; j <= PIN_BACKWARD; j++)
  // {
  //   for (int i = 0; i < 4; i++)
  //   {
  //     digitalWrite(j, HIGH);
  //     delay(500);
  //     digitalWrite(j, LOW);
  //     delay(500);
  //   }
  // }
  #endif
}

void loop() {
  static bool forceOff=true;
  static bool cabinLight = false, stripChanged = true;
  int lightsOn = (int) dccGetFunction(4);
  uint8_t dir = dccGetDirection();
  uint8_t speed = dccGetSpeed();
  uint8_t f0 = dccGetFunction(0);

  #ifdef USE_NEOPIXEL
    uint16_t countOn = 0;
    for (uint16_t i = 0; i < numPixels; i++)
    {
        lights[i].switchLight(lightsOn); 

        
        // only pixels 0, 4, 8, 12, 16, 20, 24, 28 are used, cabin light = 30
        // r, g, b = 22,21,18
        strip.setPixelColor(i /* *stepValue*/, lights[i]._state * 22, lights[i]._state * 21, lights[i]._state * 18);

        if (lights[i]._on)
          countOn++;
    }
    // if((lightsOn && (countOn < ((numPixels+1)/2))) || (!lightsOn && (countOn > 0)))
    if (lightsOn && !forceOff)
    {
      //strip.show();
      stripChanged = true;
      if (countOn ==  numPixels)
        forceOff = true;
    }
    if ((!lightsOn && forceOff))
    {
      //strip.show();
      stripChanged = true;
      forceOff = false;
    }


    // handle cabin light
    if (f0 && dir && speed < 2)
    {  // turn cabin light on!
      if (!cabinLight)
      {  // light was off, turn on
        strip.setPixelColor(maxNumPixels-1, 5,5,5);
        cabinLight = true;
        stripChanged = true;
      }
    }
    else
    { // turn cabin light off
      if (cabinLight)
      {
        strip.setPixelColor(maxNumPixels-1, 0,0,0);
        cabinLight = false;
        stripChanged = true;
      }
    }

    if (stripChanged)
    {
      strip.show();
      stripChanged = false;
    }

  #else
    for (int i = 0; i < numPins; i++)
    {
      lights[i].switchLight(lightsOn);  
    }
  #endif
  
  // handle front and rear lights
  if (f0) 
  { 
    // light is on
    if (dir)
    {
      digitalWrite(PIN_FORWARD, HIGH); 
      digitalWrite(PIN_BACKWARD, LOW);
      // PORTB |= 0b00001000;
      // PORTB &= ~(1 << PIN_BACKWARD);
    }
    else
    {
      digitalWrite(PIN_FORWARD, LOW);
      digitalWrite(PIN_BACKWARD, HIGH);
    }
  }
  else
  {  // light is off
    digitalWrite(PIN_FORWARD, LOW);
    digitalWrite(PIN_BACKWARD, LOW);
  }
}



// todo: test F13 - F28
