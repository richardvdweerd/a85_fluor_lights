#ifndef DCCLIB_H
#define DCCLIB_H

#include <Arduino.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>



#define DCC_MAX_MSG_LEN 6

// #define CV_Extended_Address_LSB     17
// #define CV_Extended_Address_MSB     18

// https://www.nmra.org/sites/default/files/s-9.2.1_2012_07.pdf
// Instruction Packets for Multi Function Decoders
#define Mask_Instruction_Type       0b11100000
#define IT_Decoder_Consist_Control  0b00000000
#define IT_Advanced_Operation       0b00100000
#define IT_Speed_Direction_Reverse  0b01000000
#define IT_Speed_Direction_Forward  0b01100000
#define IT_Function_Group_One       0b10000000
#define IT_Function_Group_Two       0b10100000
#define IT_Feature_Expansion        0b11000000
#define IT_Config_Variable_Access   0b11100000

// Decoder Consist Control (IT_Decoder_Consist_Control 000)
#define Mask_Decoder_Consist        0b00001111
#define Mask_Decoder_Control        0b00001110
#define DC_Reset                    0b00000000
#define DC_Factory_Test             0b00000010
#define DC_Future_Use_1             0b00000100
#define DC_Set_Decoder_Flags        0b00000110
#define DC_Future_Use_2             0b00001000
#define DC_Set_Advanced_Addressing  0b00001010
#define DC_Future_Use_3             0b00001110

// Advanced Operations (IT_Advanced_Operation 001)
#define Mask_Advanced_Operation     0b00011111
// format:001CCCCC  0  DDDDDDDD
// bit 7 of D: direction ('1' forward, '0' reverse)
#define AO_Speed_Step_Control_128   0b00011111
// bit 7 0f D: enable (0) or disable (1) restricted speed
#define AO_Restricted_Speed_Step    0b00011110
#define AO_Analog_Function_Group    0b00011101

// IT_Speed_Direction_Reverse 010
// IT_Speed_Direction_Forward 011
#define Mask_Speed_Direction        0b00111111
// format: for Reverse Operation   010DDDDD
// format: for Forward Operation   011DDDDD 
// bit 4 and CV29=0: controls FL
// bit 4 and CV29=1: intermediate step
// speed 0 / 1: stop / emergency brake
// speed 2-15: speed step 1 - 14

#define SD_Direction                0b00100000
#define SD_Speed_Steps              0b00001111
#define SD_Intermediate_Step        0b00010000  // CV29 = 1
#define SD_Function_Light           0b00010000  // cv29 = 0

// Feature Expansion (IT_Feature_Expansion Instruction 110)
#define Mask_Feature_Expansion      0b00011111
#define FE_Binary_State_Long        0b00000000
#define FE_Binary_State_Short       0b00011101
#define FE_FN_F13F20                0b00011110
#define FE_FN_F21F28                0b00011111

// Config Variable Access (IT_Config_Variable_Access 111)
#define Mask_Config_Variable_Access 0b00011111
// short form
#define CVA_Acceleration_Value      0b00010010 // cv23
#define CVA_Deceleration_Value      0b00010011 // cv34

// long form
// format: 1110CCVV   0   VVVVVVVV   0   DDDDDDDD
// V: 10 bit cv-number; example: CV1 = 00 00000000
// D: 8 bit value
#define CVA_Verify_byte             0b00000100  
#define CVA_Write_byte              0b00001100
#define CVA_Bit_Manipulation        0b00001000



// timing constants
#define US_HALF_BIT_ONE             58
#define US_FULL_BIT_ONE             116
#define US_MAX_BIT_ONE              122
#define US_HALF_BIT_NULL            116
#define US_FULL_BIT_NULL            232
#define US_MIN_BIT_NULL             200

#define MIN_PREAMBLE                14


// custom registers
enum REG_TYPES
{
    REG_Custom_Key,
    REG_Decoder_Type,
    REG_CV29,
    REG_Primary_Address,
    REG_FUNCION_COACH_LIGHT, 
    REG_CAB_ON_MAX_SPEED,       
    REG_COACH_RED,
    REG_COACH_GREEN,
    REG_COACH_BLUE,
    REG_CABIN_RED,
    REG_CABIN_GREEN,
    REG_CABIN_BLUE,
    NUM_REGISTERS,                   // auto count number of registers
};

#define FIRST_PROGRAMMABLE_REGISTER REG_Primary_Address

struct Register_Pair
{
    REG_TYPES   regNumber;           // valid numbers 0 - 255
    uint16_t    regValue;            // valid numbers 0 - 65535
};

enum DCC_STATES
{
    WAIT_PREAMBLE,
    WAIT_START_BIT,
    // WAIT_STOP_BIT,
    WAIT_START_STOP_BIT,
    WAIT_DATA_BYTE
};

enum DCC_TYPES
{
    DT_ACCESSORY_DECODER,
    DT_MULTIFUNCTION_DECODER
};

enum DCC_EDGES
{
    EDGE_FALLING,
    EDGE_RISING
};

enum ACC_FUNCTIONGROUPS
{
    FN_F0F4,
    FN_F5F8,
    FN_F9F12,
    FN_F13F20,
    FN_F21F28,
    FN_COUNT,
};

struct Dcc_State {
    DCC_STATES  state    = WAIT_PREAMBLE;
    DCC_EDGES   edge     = EDGE_RISING;
    int         bitCount = 0;
    int         pin      = PINB2;       // default PINB2
    bool        ready    = false;
    unsigned long timer;
    unsigned long oldTimer;
    unsigned long interval;
    uint8_t       data;
    uint8_t       msg[DCC_MAX_MSG_LEN];
    uint8_t       msgLen=0;
};

struct Dcc_Info {
    DCC_TYPES   type;               // accessory or multifunction
    uint16_t    address;            // address for accessory or multifunction decoder
    byte        controlOutput;      // for accessory: 0-3
    byte        aspect;             // aspect info for extended accessory decoder
    bool        output;             // on / off
    uint16_t    speed;
    bool        emergencyStop;
    bool        direction;
    uint8_t     function[FN_COUNT];   	    //[0] = light, F0-F4, [1] = F5-F8, [2] = F9-F12, [3] = F13-20, [4] = F21-F28
};

//void dccInit(int pin = PINB2, DCC_EDGES edge = EDGE_RISING);
void dccInit(int pin, DCC_EDGES edge);
void dccSetDecoder(uint16_t address, uint16_t decoderType);
bool dccGetFunction(uint8_t function);
uint16_t dccGetSpeed();
bool dccGetDirection();
void flashBit(uint8_t bit, uint16_t times);
uint16_t readRegister(REG_TYPES cvNumber);
#endif
