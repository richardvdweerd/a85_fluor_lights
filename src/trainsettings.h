#ifndef TRAINSETTINGS_H
#define TRAINSETTINGS_H


/******************************************************************
 * Definition file for the train settings
 * 
 * #define LOK_ADDRESS              3   // defines the dcc lok address
 * #define FUNCTION_COACH_LIGHT     3   // defines the function where the light are connected to
 * #define FUNCTION_CABIN_LIGHT     4   // defines the function number for controlling the cabin lights
 * #define NUM_PIXELS               9   // defines the total number of pixels, thus including the cabin light
 * #define CAB_LIGHT                    // defines the last pixel as the cab_light, comment out if there is no cabin light
 * #define CAB_ON_MAX_SPEED         2   // defines max speed when light in cabine turns on; when 0 light turns on only at stop
 * #define TAIL                  // defines this is the back of the train, reversing front and tail lights, comment out when front
 * #define FL_RED            22  // RGB-codes for the fluorescent lights 
 * #define FL_GREEN          21
 * #define FL_BLUE           18
 * #define CAB_RED           5   // RGB-codes for the cabin
 * #define CAB_GREEN         5
 * #define CAB_BLUE          5
 * 
 *********************************************************************/

// select one of the trains
#define TRAIN_942_FRONT
// #define TRAIN_942_BACK
// #define TRAIN_RW_1617_ASSEN
// #define TRAIN_RW_1638_GRONINGEN

// 942 is a MAT'64 electric passenger train, with 2 (motorized) carriages
#ifdef TRAIN_942_FRONT
    #define LOK_ADDRESS             942 // defines the dcc lok address
    #define NUM_PIXELS              9   // defines the total number of pixels, thus including the cabin light
    #define FUNCTION_COACH_LIGHT    3   // defines the function where the light are connected to
    #define FUNCTION_CABIN_LIGHT    4   // defines the function number for controlling the cabin lights
    // #define CABIN_LIGHT_FRONT           // defines the last pixel to be a cabin light, comment out to use this pixel as coach light
    #define CABIN_LIGHT_TAIL            // defines the first pixel to be a cabin light, comment out to use this pixel as coach light
    #define CABIN_ON_MAX_SPEED      2   // max speed when light in cabine turns on; when 0 light turns on only at stop
    // #define TAIL                        // reverses front and tail lights, as well as the front and tail cabin lights
    #define COACH_RED               11  //22  // RGB-codes for the fluorescent lights 
    #define COACH_GREEN             10  //21
    #define COACH_BLUE              9   //18
    #define CABIN_RED               5   // RGB-codes for the cabin
    #define CABIN_GREEN             5
    #define CABIN_BLUE              5
#endif // TRAIN_942_FRONT

#ifdef TRAIN_942_BACK
    #define LOK_ADDRESS         9   // defines the dcc lok address
    #define NUM_PIXELS          9   // defines the total number of pixels, thus including the cabin light
    #define FL_LIGHT_FUNCTION   3   // defines the function where the light are connected to
    #define CAB_LIGHT               // defines the last pixel to be a cab-light, comment out to use all NUM_PIXELS for flickering lights
    #define CAB_ON_MAX_SPEED    2   // max speed when light in cabine turns on; when 0 light turns on only at stop
    #define TAIL                    // determines whether to show front or back light when driving forward and vice versa, default = FRONT
    #define FL_RED              22  // RGB-codes for the fluorescent lights 
    #define FL_GREEN            21
    #define FL_BLUE             18
    #define CAB_RED             5   // RGB-codes for the cabin
    #define CAB_GREEN           5
    #define CAB_BLUE            5
#endif // TRAIN_942_FRONT


#endif // TRAINSETTINGS_H