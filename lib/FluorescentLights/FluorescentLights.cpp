/***********************************************************************************************
 * 
 * Fluorescent Lights v2
 * 
 * 
 * 
 **********************************************************************************************/

#include "FluorescentLights.h"


// unsigned int wellInitBuffer[] = {
//   0xFB34, 0x563A, 0x67d8, 0x7777, 0xF755, 0xFAAA, 0xFFBB, 0x1234, 0xFEDC, 0x6A9B, 0xEE11, 0xDD12, 0xCE13, 0xAB14, 0xCC15, 16, 17
// };
// bool randomIsInitted = false;




FluorescentLights::FluorescentLights()
{
  _pin = -1;
  // _state = 0;
  _delay = millis();
  _duration = millis();
  switchOff();
  
  if (_pin >= 0)
  {
    pinMode(_pin, OUTPUT);
  }
  switchOff();
  // if (!randomIsInitted) 
  // {
  //     initWELLRNG512a(wellInitBuffer);
  //     randomIsInitted = true;
  // }
}

void FluorescentLights::switchOff()
{
  _state = 0;
  if (_pin >= 0 )
    digitalWrite(_pin, _state);
  #ifdef ESP8266
  Serial.printf("%u %d %u: off\n", millis(), _pin, _delay);
  #endif
}

void FluorescentLights::switchOn()
{
  _state = 1;
  if (_pin >= 0)
    digitalWrite(_pin, _state);
  #ifdef ESP8266
  Serial.printf("%u %d %u: on\n", millis(), _pin, _delay);
  #endif
}

void FluorescentLights::toggleLight()
{
  _state = (_state ? 0 : 1 );
  if (_pin >= 0)
  {
    if (_state)
      switchOn();
    else
      switchOff();  
  }
}

void FluorescentLights::switchLight(int status)
{
  if (!status)
  { // switch light off
    switchOff();
    _on = false;
    return;
  }

  // turning light on
  if (!_on)
  {
    if (_delay == 0)
    {  // make random delay for light on 
      #ifdef ESP8266
        _delay = millis() + random(50, 250);
        _duration = millis() + random(1000,2000);
      #else
          _delay  = millis() + getRandom(250) + 50;
          _duration = millis() + getRandom(2000) + 1000;
      #endif
      #ifdef ESP8266
        Serial.printf("%u %d %u: new on\n", millis(), _pin, _delay);
      #endif
    }
    if (_delay < millis())
    {
      if (_duration < millis())
      {
        _on = true;
        _delay = 0;
        switchOn();
      }
      else
      {
        toggleLight();
        #ifdef ESP8266
            _delay = millis() + random(50,250);
        #else
            _delay = millis() + getRandom(250) + 50;
        #endif
      }
    }
  }
}