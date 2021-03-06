#include "DccLib.h"

/********************************************************************************************************
 * Version history
 * 
 * 
 * 06 mar 2021   v1.1  Major adaption for ATtiny1616 - Reworked the dccInit and ISR-functions                   
 * 
 *******************************************************************************************************/


//#define DCC_DEBUG

#ifdef DCC_DEBUG
  #define BOUNCE_BIT_UP(x)    { PORTB |= (1 << x); delayMicroseconds(5); PORTB &= ~(1 << x); delayMicroseconds(2); }
  #define BOUNCE_BIT_DOWN(x)  { PORTB &= ~(1 << x); delayMicroseconds(5); PORTB |= (1 << x); delayMicroseconds(5); }
#else
  #define BOUNCE_BIT_UP(x)
  #define BOUNCE_BIT_DOWN(x)
#endif

Dcc_State dccState;
Dcc_Info  dccInfo;

#if defined( __AVR_ATtiny1616__ )
  // declare global independent port-info for portA, portB and portC for 1-series
  const uint16_t avr1_port_offset       = (&PORTA);  
  const uint16_t avr1_port_size         = sizeof(PORT_t);
  const uint16_t avr1_data_in_offset    = 0x0008;
  const uint16_t avr1_intflags_offset   = 0x0009;
  const uint16_t avr1_pin0ctrl_offset   = 0x0010;

  uint16_t       avr1_intflags_address  = 0;                  // pointer to address of port, initialized in dccInit
  uint8_t        avr1_intflags_bit_mask = 0;                  // bitmask to the interrupt bit
  uint8_t        avr1_intflags_bit_position = 0;
  uint16_t       avr1_port_in_address   = 0;
#endif

//#define NUM_REGISTERS 4 is now part of REG_TYPES
#define DEFAULT_DCC_ADDRESS 42
#define GOLDEN_NUMBER   0x5

Register_Pair dcc_registers[NUM_REGISTERS] =
{
    {REG_Custom_Key,              GOLDEN_NUMBER}, // some random number, to determine there is a CV written to eeprom
    {REG_Primary_Address,         DEFAULT_DCC_ADDRESS},
    {REG_Decoder_Type,            DT_MULTIFUNCTION_DECODER},
    {REG_CV29,                    1},             // 28 steps
};

#ifdef DCC_DEBUG
void flashBit(uint8_t bit, uint16_t times)
{
  for (uint16_t i = 0; i < times; i++)
    { PORTB |= (1 << bit); delayMicroseconds(5); PORTB &= ~(1 << bit); delayMicroseconds(2); }
}
#endif

uint16_t readRegister(REG_TYPES cvNumber)
{
  for (int i = 0; i < NUM_REGISTERS; i++)
  {
    if (dcc_registers[i].regNumber == cvNumber)
      return dcc_registers[i].regValue;
  }
  return 0;
}

void writeEeprom();
void writeRegister(REG_TYPES regNumber, uint16_t regValue) 
{
  for (int i = 0; i < NUM_REGISTERS; i++)
  {
    if (dcc_registers[i].regNumber == regNumber)
    {
      dcc_registers[i].regValue = regValue;
    }
  }
  writeEeprom();
}

void writeEeprom()
{
  eeprom_write_block(dcc_registers, 0 , sizeof(dcc_registers));
}

void readEeprom()
{
    Register_Pair tempRegs[NUM_REGISTERS];
    eeprom_read_block(tempRegs, 0, sizeof(tempRegs));

    if (tempRegs[0].regValue == GOLDEN_NUMBER) {
      eeprom_read_block(dcc_registers, 0, sizeof(dcc_registers));
    }
    else
    { // write the default, initial hardcoded dcc_registers
      writeEeprom();
    }
}

void dccSetDecoder(uint16_t address, uint16_t decoderType)
{
  writeRegister(REG_Primary_Address, address);
  writeRegister(REG_Decoder_Type, decoderType);
}

bool dccGetFunction(uint8_t function)
{ 
  ACC_FUNCTIONGROUPS functionGroup;
  uint8_t bitMask;

  if (function >= 0 && function <= 4)
  {  // FN_F0F4
    functionGroup = FN_F0F4;
    bitMask = (function == 0 ? 0b00010000 : (0b00000001 << (function - 1)));
  }
  else
  {
    if (function >= 5 && function <= 8)
    { // FN_F5F8
      functionGroup = FN_F5F8;
      bitMask = (0b00000001 << (function - 5)); 
    }
    else
    {
      if (function >= 9 && function <= 12)
      { // FN_F9F12
        functionGroup = FN_F9F12;
        bitMask = (0b00000001 << (function - 9)); 
      }
    }
  }
  return ((dccInfo.function[functionGroup] & bitMask) != 0);
}

uint16_t dccGetSpeed()
{
  return dccInfo.speed;
}

bool dccGetDirection()
{
  return dccInfo.direction;
}

/****************************************************************************************************
 * 
 * dccInit
 * 
 ****************************************************************************************************/
void dccInit(int pin = 2, DCC_EDGES edge = EDGE_RISING)
{

    readEeprom();

    cli();//disable interrupts during setup
    #if defined( __AVR_ATtiny85__ )
     
      PCMSK |= (1 << pin);    //sbi(PCMSK,INTERRUPTPIN) also works but I think this is more clear // tell pin change mask to listen to pin2 /pb3 //SBI
      GIMSK |= (1 << PCIE);   // enable PCINT interrupt in the general interrupt mask //SBI
      // MCUCR = (MCUCR & 0b11111100) | RISING; // set interrupt to RISING

      DDRB &= ~(1 << pin);    // set up as input  - clear bit  - set to zero
      PORTB &= ~(1 << pin);   // disable pull-up.
      
    #elif defined( MEGATINYCORE_MCU )
      uint8_t port_number = digital_pin_to_port[pin];
      uint8_t bit         = digital_pin_to_bit_position[pin];

      /* calculate the memory address of  pinxctrl, using port and pin positions */
      uint16_t address = avr1_port_offset + port_number * avr1_port_size + 1 + bit;

      /* set the interrupt according to given edge, pullupen is false*/
      _SFR_MEM8( address )  = (edge == EDGE_RISING ? PORT_ISC_RISING_gc : PORT_ISC_FALLING_gc);

      /* store INTFLAGS address and bitmask in global variables */
      avr1_intflags_address  = avr1_port_offset + port_number * avr1_port_size + avr1_intflags_offset;
      avr1_intflags_bit_mask = (1 << bit);
      avr1_intflags_bit_position = bit;
      avr1_port_in_address       = avr1_port_offset + port_number * avr1_port_size + avr1_data_in_offset;
      #warning "1616 is probably not defined completely yet"

    #endif


    sei(); //last line of setup - enable interrupts after setup - same for 1616 and 85

    dccState.state = WAIT_PREAMBLE;
    dccState.bitCount = 0;
    dccState.pin = pin;
    dccState.msgLen = 0;
    dccState.edge = edge;


}

bool checksumDccPacket(byte *buffer, int length)
{
  int check = 0;
  for (int i = 0; i < length; i++)
    check ^= buffer[i];
  return (check == 0x00);
}

void decodeDccInfo()
{
  byte msgIndex = 0;
  dccInfo.address = 0;

  if (!checksumDccPacket(dccState.msg, dccState.msgLen))
  { // checksum does not match, reject this message
    return;
  }
  
  if (dccState.msgLen >= 3)
  { // minimum packet size = 3


    // decode address

    dccInfo.type = ((dccState.msg[msgIndex] & 0b11000000) == 0b10000000 ? DT_ACCESSORY_DECODER : DT_MULTIFUNCTION_DECODER);

    switch(dccInfo.type)
    {
      case DT_MULTIFUNCTION_DECODER:
        if ((dccState.msg[msgIndex] & 0b10000000) == 0b00000000)
        { // short address
          dccInfo.address = (uint16_t) dccState.msg[msgIndex];
          msgIndex++;
        }
        else
        { // long address
          // https://www.nmra.org/sites/default/files/s-9.2.1_2012_07.pdf
          if (dccState.msg[msgIndex] >= 192 && dccState.msg[msgIndex] <= 231)
          { // long address confirmed

            dccInfo.address = ((uint16_t)dccState.msg[msgIndex] & 0b00111111) << 8 | ( (uint16_t)dccState.msg[msgIndex + 1]);
            msgIndex += 2;         
          }
        }
        break;
      case DT_ACCESSORY_DECODER:
        if ((dccState.msg[msgIndex + 1] & 0b10000000) == 0b10000000 )
        { // bit 7 of second byte = '1': basic accessory decoder, 9 bit address
          dccInfo.address = (dccState.msg[msgIndex] & 0b00111111);// | (~((dccState.msg[msgIndex + 1] & 0b01110000) & 0b01110000) << 6);
          dccInfo.controlOutput = (dccState.msg[msgIndex + 1] & 0b00000110) >> 1;
          dccInfo.output = (dccState.msg[msgIndex + 1] & 0b00000001);
          
        }
        else
        {             //  bit 7 of second byte = '0': extended accessory decoder, 11 bit address

          if (dccState.msgLen >= 3)
          {
            dccInfo.address = (dccState.msg[msgIndex] & 0b00111111) | ((dccState.msg[msgIndex + 1] & 0b01110000) << 4) | ((dccState.msg[msgIndex + 1] & 0b00000110) << 5);
            dccInfo.aspect  = (dccState.msg[msgIndex + 2] & 0b00011111);
          }
        }
      break;
    }

    if (dccInfo.address != readRegister(REG_Primary_Address) || dccInfo.type != readRegister(REG_Decoder_Type))
      return;


    // address is handled, move to the instructions
    if (dccInfo.type == DT_MULTIFUNCTION_DECODER) 
    {
      // for (int i=1; i < dccState.msgLen - 1; i++) // omit address and checksum
      // {
      //   flashBit(PB3, 1); // reset counter on oscillosscope
      //   flashBit(PB4, dccState.msg[i]);
      // }
      
      // https://www.nmra.org/sites/default/files/s-9.2.1_2012_07.pdf
      switch((dccState.msg[msgIndex] & Mask_Instruction_Type))
      {
        case IT_Function_Group_One: // function group FL, F1-F4
          dccInfo.function[FN_F0F4]  = dccState.msg[msgIndex] & 0b00011111;
          break;
        case IT_Function_Group_Two: // function group F5-F8 and F9-F12
          if (dccState.msg[msgIndex] & 0b00010000)
            dccInfo.function[FN_F5F8] = dccState.msg[msgIndex] & 0b00001111;
          else
            dccInfo.function[FN_F9F12] = dccState.msg[msgIndex] & 0b00001111;
          break;
        case IT_Speed_Direction_Reverse:
        case IT_Speed_Direction_Forward:
        {
          // flashBit(PB1, 1);
          // handle reverse and forward in one go
          dccInfo.direction = ((dccState.msg[msgIndex] & SD_Direction) == SD_Direction);
          dccInfo.emergencyStop = ((dccState.msg[msgIndex] & SD_Speed_Steps) == 1);
          uint16_t speed = (uint16_t) (dccState.msg[msgIndex] & SD_Speed_Steps);
          if (speed > 1) 
          { 
            if (readRegister(REG_CV29))
            {
              if (speed < 2)
                speed = 0;
              else 
              {
                speed -= 2;
                speed = speed * 2 + ((dccState.msg[msgIndex] & SD_Intermediate_Step) / SD_Intermediate_Step) + 1;
              }
            }
            else
            { 
              if (speed < 2)
                speed = 0;
              else 
                speed--;
              dccInfo.function[FN_F0F4] = (dccInfo.function[FN_F0F4] & 0b00001111) | (dccState.msg[msgIndex] & SD_Function_Light);
            }
          }
          dccInfo.speed = speed;

              // flashBit(PB1, 2);
              // flashBit(PB3, 2);
              // flashBit(PB4, speed);
              // flashBit(PB3, 2);
              // flashBit(PB4, dccInfo.direction);
              // flashBit(PB3, 2);

          break;
        }
        case IT_Feature_Expansion:
          switch(dccState.msg[msgIndex] & Mask_Feature_Expansion)
          {
            case FE_FN_F13F20:
              dccInfo.function[FN_F13F20] = dccState.msg[msgIndex + 1];
              break;
            case FE_FN_F21F28:
              dccInfo.function[FN_F21F28] = dccState.msg[msgIndex + 1];
              break;
          }
          break;
      }
    }
  }

}

/****************************************************************************************************
 * 
 * support routines for interrupt handlers
 * 
 ****************************************************************************************************/
void int_dealer(uint8_t pinState)
{
  static uint8_t old_itsaOne = 0;
  static uint8_t itsaOne = 0;  // make this static as well, to save time creating it over and over
  static uint8_t skipOne = 0;  // whether to skip a pulse or not, applicable after edge change

  // look at the pin state on the pin PINB register- returns 1 if high and trigger on the edge of dccState.edge
  if (pinState == dccState.edge)
  {
    if( skipOne ) 
    {
      skipOne = 0;
      return;
    }
    dccState.timer = micros();
    dccState.interval = dccState.timer - dccState.oldTimer;
    dccState.oldTimer = dccState.timer;
    if (dccState.interval > US_MAX_BIT_ONE && dccState.interval < US_MIN_BIT_NULL) 
    {
      // we got one and a half bit??? try reversing the interrupt edge
      dccState.edge = (dccState.edge == EDGE_RISING ? EDGE_FALLING : EDGE_RISING);
      itsaOne = !old_itsaOne;
      dccState.oldTimer += (itsaOne ? US_HALF_BIT_ONE : US_HALF_BIT_NULL);
      skipOne = 1; // there's a edge change, skip the next interrupt
    }
    else
    {
      itsaOne = (dccState.interval < US_MAX_BIT_ONE);
    }

	  old_itsaOne = itsaOne;   // save this for the next round
    switch (dccState.state)
    {
        case WAIT_PREAMBLE:
            if (itsaOne)
            {
                 if (++dccState.bitCount >= MIN_PREAMBLE)
                {
                    dccState.state = WAIT_START_BIT;
                }
            }
            else
            {
              dccState.bitCount = 0;
              dccState.msgLen = 0;
            }
            break;
        case WAIT_START_BIT:
            if (!itsaOne) 
            {  // it's a '0': a start bit or separator
                dccState.state = WAIT_DATA_BYTE;
                dccState.bitCount = 0;
                dccState.msgLen = 0;
            }
            break;
        case WAIT_START_STOP_BIT:
            if (itsaOne)
            { // this was a '1': a stop bit
                dccState.ready = true;
                dccState.state = WAIT_PREAMBLE;
                dccState.bitCount = 0;
                cli();
                decodeDccInfo();
                sei();
                dccState.msgLen = 0;
            }
            else
            { // this was a '0', assume this was a start bit, new data will be received
                dccState.state = WAIT_DATA_BYTE;
                dccState.bitCount = 0;
            }
            break;
        case WAIT_DATA_BYTE:
            ++dccState.bitCount;
            dccState.data <<= 1;
            dccState.data |= itsaOne;

            if (dccState.bitCount == 8)
            { // we've got 8 data bits. save it if there's room in the msg buffer.
                if (dccState.msgLen < DCC_MAX_MSG_LEN)
                {
                    dccState.msg[dccState.msgLen++] = dccState.data;
                    dccState.state = WAIT_START_STOP_BIT;
                    dccState.bitCount = 0;
                }
                else
                { // no room in the msg buffer. wait for a new preamble
                    // dccState.msgLen = 0;
                    dccState.state = WAIT_PREAMBLE;
                    dccState.bitCount = 0;
                }
            }
    } // end switch
  } // if
}


/****************************************************************************************************
 * 
 * interrupt handler(s)
 * 
 ****************************************************************************************************/

#if defined( __AVR_ATtiny85__ )
ISR(PCINT0_vect)
{

  // determine the RISING or FALLING state of the interrupt
  uint8_t pinState = (PINB >> dccState.pin) & 1; //PINB is the register to read the state of the pins
  int_dealer(pinState);

  //int_dealer();


}

#elif defined( MEGATINYCORE_SERIES )


ISR(PORTA_PORT_vect)
{
  uint8_t flags = _SFR_MEM8(avr1_intflags_address);
  _SFR_MEM8(avr1_intflags_address) = flags; // reset the flags

  		
  uint8_t pinState = ( _SFR_MEM8(avr1_port_in_address) & avr1_intflags_bit_mask) >> avr1_intflags_bit_position;
  int_dealer(pinState);
}

ISR(PORTB_PORT_vect)
{
  uint8_t flags = _SFR_MEM8(avr1_intflags_address);
  _SFR_MEM8(avr1_intflags_address) = flags; // reset the flags

  		
  uint8_t pinState = ( _SFR_MEM8(avr1_port_in_address) & avr1_intflags_bit_mask) >> avr1_intflags_bit_position;
  int_dealer(pinState);
}

ISR(PORTC_PORT_vect)
{
  uint8_t flags = _SFR_MEM8(avr1_intflags_address);
  _SFR_MEM8(avr1_intflags_address) = flags; // reset the flags

  		
  uint8_t pinState = ( _SFR_MEM8(avr1_port_in_address) & avr1_intflags_bit_mask) >> avr1_intflags_bit_position;
  int_dealer(pinState);
}

#endif