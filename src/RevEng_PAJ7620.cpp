/*
  RevEng_PAJ7620.cpp

  - Copyright (c) 2015 seeed technology inc.
  - Website    : www.seeed.cc
  - Author     : Wuruibin & Xiangnan
  - Modified Time: June 2015

  Additional contributions:
  - 2017 - Modified by MarcFinns to encapsulate in class without global variables  
  - 2020 - PROGMEM code adapted from Jaycar-Electronics' work  
  - 2020 - Modified by Aaron S. Crandall <crandall@gonzaga.edu>  
  - 2020 - Modified by Sean Kallaher (GitHub: skallaher) 
   
  Version: 1.4.0

  Description: This demo can recognize 9 gestures and output the result,
        including move up, move down, move left, move right,
        move forward, move backward, circle-clockwise,
        circle-anti (counter) clockwise, and wave.

  License: The MIT License (MIT)

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.
*/

#include "RevEng_PAJ7620.h"


/****************************************************************
   \details PAJ7620 device I2C connect and initialize on default Wire bus
   \param none
   \return error code: 0 (false); success: return 1 (true)
****************************************************************/
uint8_t RevEng_PAJ7620::begin()
{
  return begin(&Wire);
}

/****************************************************************
   \details PAJ7620 device I2C connect and initialize

   Override version:
   \par
   Takes a TwoWire pointer allowing the user to pass
      in a specified I2C bus for devices using alternatives to bus 0 such
      as: begin(&Wire1) or begin(&Wire2)

   \param chosenWireHandle A pointer to the Wire handle that should be
     used to communicate with the PAJ7620
   \return error code: 0 (false); success: return 1 (true)
****************************************************************/
uint8_t RevEng_PAJ7620::begin(TwoWire *chosenWireHandle)
{
  // Reasonable timing delay values to make algorithm insensitive to
  //  hand entry and exit moves before and after detecting a gesture
  gestureEntryTime = 0;
  gestureExitTime = 200;

  wireHandle = chosenWireHandle;      // Save selected I2C bus for our use

  delayMicroseconds(700);	            // Wait 700us for PAJ7620U2 to stabilize
                                      // Reason: see v0.8 of 7620 documentation
  wireHandle->begin();
  selectRegisterBank(BANK0);          // Default operations on BANK0

  if( !isPAJ7620UDevice() ) {
    return 0;                         // Return false - wrong device found
  }

  initializeDeviceSettings();         // Set registers up
  setGestureMode();                   // Specifically set to gesture mode

  return 1;
}


/****************************************************************
   PAJ7620 Write register cmd
   \param addr register address
   \param cmd data (byte) to write
   \return error code; success: return 0
****************************************************************/
uint8_t RevEng_PAJ7620::writeRegister(uint8_t addr, uint8_t cmd)
{
  uint8_t result_code = 0;
  wireHandle->beginTransmission(PAJ7620_I2C_BUS_ADDR);   // start transmission
  wireHandle->write(addr);                               // send register address
  wireHandle->write(cmd);                                // send value to write
  result_code = wireHandle->endTransmission();           // end transmission
  return result_code;
}


/****************************************************************
   Function Name: readRegister
   Description:  PAJ7620 read reg data
   Parameters: addr:reg address;
           qty:number of data to read, addr continuously increase;
           data[]:storage memory start address
   Return: error code; success: return 0
****************************************************************/
uint8_t RevEng_PAJ7620::readRegister(uint8_t addr, uint8_t qty, uint8_t data[])
{
  uint8_t result_code;
  wireHandle->beginTransmission(PAJ7620_I2C_BUS_ADDR);
  wireHandle->write(addr);
  result_code = wireHandle->endTransmission();

  if (result_code)            //return error code - if not zero
    { return result_code; }

  wireHandle->requestFrom((int)PAJ7620_I2C_BUS_ADDR, (int)qty);

  while (wireHandle->available())
  {
    *data = wireHandle->read();
    data++;
  }

  return 0;
}


/****************************************************************
   Description:  Read the gestures interrupt vector #0
   Parameters: &uint8_t for storing value
   Return: error code (0 means no error)
****************************************************************/
uint8_t RevEng_PAJ7620::getGesturesReg0(uint8_t data[])
  { return readRegister(PAJ7620_ADDR_GES_RESULT_0, 1, data); }


/****************************************************************
   Function Name: getGesturesReg1
   Description:  Read the gestures interrupt vector #1 (wave gesture)
   Parameters: &uint8_t for storing value
   Return: error code (0 means no error)
****************************************************************/
uint8_t RevEng_PAJ7620::getGesturesReg1(uint8_t data[])
  { return readRegister(PAJ7620_ADDR_GES_RESULT_1, 1, data); }


/****************************************************************
   Function Name: selectRegisterBank
   Description:  PAJ7620 select register bank
   Parameters: BANK0, BANK1 - see Bank_e enum
   Return: none
****************************************************************/
void RevEng_PAJ7620::selectRegisterBank(Bank_e bank)
{
  if( bank == BANK0 )
    { writeRegister(PAJ7620_REGISTER_BANK_SEL, PAJ7620_BANK0); }
  else if( bank == BANK1 )
    { writeRegister(PAJ7620_REGISTER_BANK_SEL, PAJ7620_BANK1); }
}


/***************************************************************
   Function Name: isPAJ7620UDevice
   Description: See if it truly a PAJ7620 at the I2C address by checking its ID
   Parameters: none
   Return: true/false
****************************************************************/
bool RevEng_PAJ7620::isPAJ7620UDevice()
{
  uint8_t data0 = 0, data1 = 0;

  // Device ID is stored in BANK0
  selectRegisterBank(BANK0);

  // Read PartID LSB[7:0] from Bank0, 0x00 - Should read 0x20
  // Read PartID MSB[15:8] from Bank0, 0x01 - Should read 0x76
  readRegister(PAJ7620_ADDR_PART_ID_0, 1, &data0);
  readRegister(PAJ7620_ADDR_PART_ID_1, 1, &data1);

  // Test if part ID is corect for PAJ7620U2
  //  See: PAJ7620U2 datasheet page 24 - 5.16 Chip/Version ID
  if ( (data0 != PAJ7620_PART_ID_LSB ) || (data1 != PAJ7620_PART_ID_MSB) )
    { return false; }

  return true;
}


/****************************************************************
****************************************************************/
void RevEng_PAJ7620::writeRegisterArray(unsigned short array[], int arraySize)
{
  for (unsigned int i = 0; i < arraySize; i++)
  {
    #ifdef PROGMEM_COMPATIBLE
      uint16_t word = pgm_read_word(&array[i]);
    #else
      uint16_t word = array[i];
    #endif

    uint8_t address, value;
    address = (word & 0xFF00) >> 8;
    value = (word & 0x00FF);
    writeRegister(address, value);
  }
  selectRegisterBank(BANK0);        // Guarantee parking in BANK0
}


/****************************************************************
   Function Name: initializeDeviceSettings
   Description: Write settings to enable I2C gesture recognition
      See header for initRegisterArray definition
   Parameters: none
   Return: none
****************************************************************/
void RevEng_PAJ7620::initializeDeviceSettings()
{
  writeRegisterArray(initRegisterArray, INIT_REG_ARRAY_SIZE);
}


/****************************************************************
****************************************************************/
void RevEng_PAJ7620::setGestureMode()
{
  writeRegisterArray(setGestureModeRegisterArray, SET_GES_MODE_REG_ARRAY_SIZE);
}


/****************************************************************
****************************************************************/
void RevEng_PAJ7620::setCursorMode()
{
  writeRegisterArray(setCursorModeRegisterArray, SET_CURSOR_MODE_REG_ARRAY_SIZE);
}


/****************************************************************
****************************************************************/
int RevEng_PAJ7620::getCursorX()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_CURSOR_X_LOW, 1, &data0);
  readRegister(PAJ7620_ADDR_CURSOR_X_HIGH, 1, &data1);
  data1 &= 0x0F;      // Mask off high bits (unused)
  result |= data1;
  result = result << 8;
  result |= data0;

  return result;
}


/****************************************************************
****************************************************************/
int RevEng_PAJ7620::getCursorY()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_CURSOR_Y_LOW, 1, &data0);
  readRegister(PAJ7620_ADDR_CURSOR_Y_HIGH, 1, &data1);
  data1 &= 0x0F;      // Mask off high bits (unused)
  result |= data1;
  result = result << 8;
  result |= data0;

  return result;
}


/****************************************************************
****************************************************************/
bool RevEng_PAJ7620::isCursorInView()
{
  bool result = false;
  uint8_t data = 0x00;
  readRegister(PAJ7620_ADDR_CURSOR_INT, 1, &data);
  switch(data)
  {
    case CUR_NO_OBJECT:   result = false;   break;
    case CUR_HAS_OBJECT:  result = true;    break;
    default:              result = false;   break;
  }
  return result;
}


/****************************************************************
****************************************************************/
void RevEng_PAJ7620::invertXAxis()
{
  uint8_t data = 0x00;
  selectRegisterBank(BANK1);
  readRegister(PAJ7620_ADDR_LENS_ORIENTATION, 1, &data);
  data ^= 1UL << 0;               // Bit[0] controls X axis
  writeRegister(PAJ7620_ADDR_LENS_ORIENTATION, data);
  selectRegisterBank(BANK0);
}


/****************************************************************
****************************************************************/
void RevEng_PAJ7620::invertYAxis()
{
  uint8_t data = 0x00;
  selectRegisterBank(BANK1);
  readRegister(PAJ7620_ADDR_LENS_ORIENTATION, 1, &data);
  data ^= 1UL << 1;                 // Bit[1] controls Y axis
  writeRegister(PAJ7620_ADDR_LENS_ORIENTATION, data);
  selectRegisterBank(BANK0);
}


/****************************************************************
   Disables sensor for reading & interrupts
   \param none
   \return none
****************************************************************/
void RevEng_PAJ7620::disable()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_OPERATION_ENABLE, PAJ7620_DISABLE);
  selectRegisterBank(BANK0);
}


/****************************************************************
   Enables sensor for reading & interrupts
   \param none
   \return none
****************************************************************/
void RevEng_PAJ7620::enable()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_OPERATION_ENABLE, PAJ7620_ENABLE);
  selectRegisterBank(BANK0);
}


/****************************************************************
   Sets the delay on gesture reads for forward/backward gestures
   \param newGestureEntryTime The time to wait before detecting
     a forward/backward gesture
   \return none
****************************************************************/
void RevEng_PAJ7620::setGestureEntryTime(unsigned long newGestureEntryTime)
{
  gestureEntryTime = newGestureEntryTime;
}


/****************************************************************
   Sets the delay after gesture reads to 
      allow the person to withdraw their hand and not cause a second
      gesture event to be thrown. 
   \param newGestureExitTime The time to wait after a gesture
     read
   \return none
****************************************************************/
void RevEng_PAJ7620::setGestureExitTime(unsigned long newGestureExitTime)
{
  gestureExitTime = newGestureExitTime;
}


// void RevEng_PAJ7620::setGameMode()
// {
  /*
    NOTE: No version of the PixArt documentation says how to enable game mode
      If you know, please let me know so we can get it added here
      This code below comes from unknown sources, but was patched into various
      forks of the Seeed version on GitHub.
        -- Aaron S. Crandall <crandall@gonzaga.edu>
  */
   /*
   * Setting normal mode or gaming mode at BANK1 register 0x65/0x66 R_IDLE_TIME[15:0]
   * T = 256/System CLK = 32us, 
   * Ex:
   * Far Mode: 1 report time = (77+R_IDLE_TIME)T
   * Report rate 120 fps:
   * R_IDLE_TIME=1/(120*T)-77=183
   * 
   * Report rate 240 fps:
   * R_IDLE_TIME=1/(240*T)-77=53
   * 
   * Near Mode: 1 report time = (112+R_IDLE_TIME)T
   * 
   * Report rate 120 fps:
   * R_IDLE_TIME=1/(120*T)-120=148
   * 
   * Report rate 240 fps:
   * R_IDLE_TIME=1/(240*T)-112=18
   * 
   */  
  // Serial.println("Set up gaming mode.");
  // paj7620SelectBank(BANK1);  //gesture flage reg in Bank1
  // paj7620WriteReg(0x65, 0xB7); // far mode 120 fps
  //paj7620WriteReg(0x65, 0x12);  // near mode 240 fps

  // paj7620SelectBank(BANK0);  //gesture flage reg in Bank0
// }


/****************************************************************
   Clear current gesture interrupt vectors
   \note These vectors are set to zero in hardware after any reads

   \param none
   \return none
****************************************************************/
void RevEng_PAJ7620::clearGesture()
{
    uint8_t data = 0, data1 = 0;
    getGesturesReg0(&data);
    getGesturesReg1(&data1);
}


/****************************************************************
   Get current count of waves by user
   \param none
   \return quantity of waves (passes) over the sensor
****************************************************************/
int RevEng_PAJ7620::getWaveCount()
{
  uint8_t waveCount = 0;
  readRegister(PAJ7620_ADDR_WAVE_COUNT, 1, &waveCount);
  waveCount &= 0x0F;      // Count is [3:0] bits - values in 0..15
  return waveCount;
}


/****************************************************************
   Function Name: forwardBackwardGestureCheck
   Description: Used to double check a lateral gesture (up, down, left, right)
     to see if it was actually a vertical gesture (forward, backward)
   Parameters: Gesture initialGesture
   Return: Gesture - the double checked gesture
****************************************************************/
Gesture RevEng_PAJ7620::forwardBackwardGestureCheck(Gesture initialGesture)
{
  uint8_t data1 = 0;
  Gesture result = initialGesture;

  delay(gestureEntryTime);
  getGesturesReg0(&data1);
  if (data1 == GES_FORWARD_FLAG)
  {
    delay(gestureExitTime);
    result = GES_FORWARD;
  }
  else if (data1 == GES_BACKWARD_FLAG)
  {
    delay(gestureExitTime);
    result = GES_BACKWARD;
  }
  return result;
}


/****************************************************************
   Read the latest gesture from the sensor.
   Clears interrupt vector of gestures upon read
   \param none
   \return Gesture found or \link GES_NONE Gesture::GES_NONE \endlink if no gesture found
****************************************************************/
Gesture RevEng_PAJ7620::readGesture()
{
  uint8_t data = 0, data1 = 0, readCode = 0;
  Gesture result = GES_NONE;

  readCode = getGesturesReg0(&data);
  if (readCode)
  {
    return GES_NONE;
  }
  else
  {
    switch (data)
    {
      case GES_RIGHT_FLAG:
        result = forwardBackwardGestureCheck(GES_RIGHT);
        break;

      case GES_LEFT_FLAG:
        result = forwardBackwardGestureCheck(GES_LEFT);
        break;

      case GES_UP_FLAG:
        result = forwardBackwardGestureCheck(GES_UP);
        break;

      case GES_DOWN_FLAG:
        result = forwardBackwardGestureCheck(GES_DOWN);
        break;

      case GES_FORWARD_FLAG:
        delay(gestureExitTime);
        result = GES_FORWARD;
        break;

      case GES_BACKWARD_FLAG:
        delay(gestureExitTime);
        result = GES_BACKWARD;
        break;

      case GES_CLOCKWISE_FLAG:
        result = GES_CLOCKWISE;
        break;

      case GES_ANTI_CLOCKWISE_FLAG:
        result = GES_ANTICLOCKWISE;
        break;

      default:
        getGesturesReg1(&data1);      // Reg1 (0x44) has wave flag
        if (data1 == GES_WAVE_FLAG)
          { result = GES_WAVE; }
        break;
    }
  }
  return result;
}
