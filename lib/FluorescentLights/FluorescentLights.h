/***********************************************************************************************
 * 
 * Fluorescent Lights v2
 * 
 * 
 * 
 **********************************************************************************************/



#ifndef FLUORESCENTLIGHTS_H
#define FLUORESCENTLIGHTS_H

#include <Arduino.h>
// #include "WELL512a.h"
#include "getRandom.h"


class FluorescentLights {
    // bool randomIsInitted = false;
  public:
    bool _on = false;
    uint8_t _state = false;
    uint8_t  _pin = -1;
    unsigned long _delay = 0;
    unsigned long _duration = 0;
    void switchOn();
    void switchOff();
    void toggleLight();

  public:
    FluorescentLights();
    void switchLight(int status);
};


#endif