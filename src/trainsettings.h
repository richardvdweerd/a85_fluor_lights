#ifndef TRAINSETTINGS_H
#define TRAINSETTINGS_H


// select one of the trains
#define TRAIN_942_FRONT
// #define TRAIN_942_BACK
// #define TRAIN_RW_1617_ASSEN
// #define TRAIN_RW_1638_GRONINGEN

#ifdef TRAIN_942_FRONT
    #define LOK_ADDRESS 9
    #define NUM_PIXELS 9
    #define CAB_LIGHT           // defines the last pixel to be a cab-light, comment out to use all NUM_PIXELS for flickering lights
    #define CAB_ON_MAX_SPEED 2  // max speed when light in cabine turns on; when 0 light turns on only at stop
    //#define FRONT             // determines whether to show front or back light when driving forward and vice versa, default = FRONT
#endif // TRAIN_942_FRONT

#ifdef TRAIN_942_BACK
    #define LOK_ADDRESS 9
    #define NUM_PIXELS 9
    // #define CAB_LIGHT        // defines the last pixel to be a cab-light, comment out to use all NUM_PIXELS for flickering lights
    #define BACK                // determines whether to show front or back light when driving forward and vice versa
#endif // TRAIN_942_FRONT


#endif // TRAINSETTINGS_H