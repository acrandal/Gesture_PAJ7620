/**
  \file RevEng_PAJ7620.cpp
  \author Aaron S. Crandall

  \version 1.5.0

  \copyright
  \parblock
  - Copyright (c) 2015 seeed technology inc.
  - Website    : www.seeed.cc
  - Author     : Wuruibin & Xiangnan
  - Modified Time: June 2015

  Additional contributions:
  - 2017 - Modified by MarcFinns to encapsulate in class without global variables  
  - 2020 - PROGMEM code adapted from Jaycar-Electronics' work  
  - 2020 - Modified by Aaron S. Crandall <crandall@gonzaga.edu>  
  - 2020 - Modified by Sean Kallaher (GitHub: skallaher) 
  
  Description: This driver class can recognize 9 gestures and output the result,
        including move up, move down, move left, move right,
        move forward, move backward, circle-clockwise,
        circle-anti (counter) clockwise, and wave.
        The driver also allows changing the sensor to 'cursor mode' where it
        tracks the closest object in view on an (X,Y) coordinate system.

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
  \endparblock

  PAJ7620U2 Sensor data sheet for reference found here:
    https://datasheetspdf.com/pdf-file/1309990/PixArt/PAJ7620U2/1

  Driver sources, latest code, and authors available at:
    https://github.com/acrandal/RevEng_PAJ7620
*/

#include "RevEng_PAJ7620.h"


/**
 * PAJ7620 device initialization and I2C connect to default Wire bus
 * 
 * \param none
 * \return error code: 0 (false); success: return 1 (true)
 */
uint8_t RevEng_PAJ7620::begin()
{
  return begin(&Wire);
}

/**
 * PAJ7620 device initialization and I2C connect on specified Wire bus
 *
 * Override version:
 * \par
 * Takes a TwoWire pointer allowing the user to pass
 *    in a specified I2C bus for devices using alternatives to bus 0 such
 *    as: begin(&Wire1) or begin(&Wire2)
 *
 * \param chosenWireHandle A pointer to the Wire handle that should be
 *   used to communicate with the PAJ7620
 * \return error code: 0 (false); success: return 1 (true)
 */
uint8_t RevEng_PAJ7620::begin(TwoWire *chosenWireHandle)
{
  // Reasonable timing delay values to make algorithm insensitive to
  //  hand entry and exit moves before and after detecting a gesture
  gestureEntryTime = 0;
  gestureExitTime = 200;

  wireHandle = chosenWireHandle;      // Save selected I2C bus for our use

  delayMicroseconds(700);	            // Wait 700us for PAJ7620U2 to stabilize
                                      // Reason: see v0.8 of 7620 documentation
  wireHandle->begin();                // Start the I2C bus via wire library


  /* There's two register banks (0 & 1) to be selected between.
   * BANK0 is where most data collection operations happen, so it's default.
   * Selecting the bank is done here twice for a reason. When the 7620 turns
   *  on, the I2C bus is sleeping. When you first read/write to the bus
   *  the 7620 wakes up, but it sometimes misses that first message.
   * Running the 7620 on an arduino with the USB power, a single call here
   *  usually works, but as soon as you use an external power bus it often
   *  fails to properly initialize and begin returns an error.
   */
  selectRegisterBank(BANK0);          // This is done twice on purpose
  selectRegisterBank(BANK0);          // Default operations on BANK0

  if( !isPAJ7620UDevice() ) {
    return 0;                         // Return false - wrong device found
  }

  initializeDeviceSettings();         // Set registers up
  setGestureMode();                   // Specifically set to gesture mode

  return 1;
}


/**
 * Write memory register over I2C
 * \param i2cAddress register address
 * \param dataByte data (byte) to write
 * \return error code; success: return 0
 */
uint8_t RevEng_PAJ7620::writeRegister(uint8_t i2cAddress, uint8_t dataByte)
{
  uint8_t resultCode = 0;
  wireHandle->beginTransmission(PAJ7620_I2C_BUS_ADDR);   // start transmission
  wireHandle->write(i2cAddress);                         // send register address
  wireHandle->write(dataByte);                           // send value to write
  resultCode = wireHandle->endTransmission();            // end transmission
  return resultCode;
}

/**
 * Read memory register over I2C
 * \param i2cAddress : register address
 * \param byteCount : quantity of bytes to read into data
 * \param data : array of uint8_t to read data into
 * \return error code; success: return 0
 */
uint8_t RevEng_PAJ7620::readRegister(uint8_t i2cAddress, uint8_t byteCount, uint8_t data[])
{
  uint8_t result_code;
  wireHandle->beginTransmission(PAJ7620_I2C_BUS_ADDR);
  wireHandle->write(i2cAddress);
  result_code = wireHandle->endTransmission();

  if (result_code)            //return error code - if not zero
    { return result_code; }

  wireHandle->requestFrom((int)PAJ7620_I2C_BUS_ADDR, (int)byteCount);

  while (wireHandle->available())
  {
    *data = wireHandle->read();
    data++;
  }

  return 0;
}


/**
 * Read the gestures interrupt vector #0 - all gestures except wave
 * \param data : &uint8_t for storing value read
 * \return error code; success: return 0
 */
uint8_t RevEng_PAJ7620::getGesturesReg0(uint8_t data[])
  { return readRegister(PAJ7620_ADDR_GES_RESULT_0, 1, data); }


/**
 * Read the gestures interrupt vector #1 - only holds wave
 * \param data : &uint8_t for storing value read
 * \return error code; success: return 0
 */
uint8_t RevEng_PAJ7620::getGesturesReg1(uint8_t data[])
  { return readRegister(PAJ7620_ADDR_GES_RESULT_1, 1, data); }


/**
 * Select memory bank to read/write to
 * \par
 * The PAJ7620 has two memory banks. The user must select which bank to use
 * when reading and writing over I2C.
 * \note This driver defaults to operations resetting to BANK0 for general operation.
 * \param bank : \link Bank_e \endlink to select (BANK0, BANK1)
 * \return none
 */
void RevEng_PAJ7620::selectRegisterBank(Bank_e bank)
{
  if( bank == BANK0 )
    { writeRegister(PAJ7620_REGISTER_BANK_SEL, PAJ7620_BANK0); }
  else if( bank == BANK1 )
    { writeRegister(PAJ7620_REGISTER_BANK_SEL, PAJ7620_BANK1); }
}


/**
 * Reads device memory to check for the PAJ7620 hardware identifier (ID)
 * \par
 * At memory address BANK0, 0x00 the device returns 0x20.
 * At memory address BANK0, 0x01 the device returns 0x76.
 * If this is not true, a non-PAJ7620 I2C device is attached at this I2C address.
 * See: PAJ7620U2 datasheet page 24 - 5.16 Chip/Version ID
 * \param none
 * \return bool: True means it is a PAJ7620, False means failure to read or ID match
 */
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


/**
 * Writes an array of values to the device memory
 * 
 * \par
 * Writes over I2C to the memory banks a set of default values for operation.
 * The values are taken from the PAJ7620U2 v0.8 documentation and encoded
 * in the \link initRegisterArray \endlink from the driver's header file
 * 
 * \note Expects array[] to be stored in PROGMEM if it is available on your microcontroller
 * 
 * \param array : array of const unsigned shorts - first byte is address, second byte is data
 * \param arraySize : quantity of elements in array to write
 * \return none
 */
void RevEng_PAJ7620::writeRegisterArray(const unsigned short array[], int arraySize)
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


//***************************************************************************************


/**
 * Initializes registers for device to default values
 * 
 * \par
 * Writes over I2C to the memory banks a set of default values for operation.
 * The values are taken from the PAJ7620U2 v0.8 documentation and encoded
 * in the \link initRegisterArray \endlink from the driver's header file
 * 
 * \param none
 * \return none
 */
void RevEng_PAJ7620::initializeDeviceSettings()
{
  writeRegisterArray(initRegisterArray, INIT_REG_ARRAY_SIZE);
}


/**
 * Puts device into Gesture mode
 * 
 * \par
 * Initializes registers for Gesture mode and enables only the gesture interrupts
 * 
 * \param none
 * \return none
 */
void RevEng_PAJ7620::setGestureMode()
{
  writeRegisterArray(setGestureModeRegisterArray, SET_GES_MODE_REG_ARRAY_SIZE);
}


/**
 * Puts device into Cursor mode
 * 
 * \par
 * Initializes registers for Cursor mode and enables only the cursor interrupts
 * 
 * \param none
 * \return none
 */
void RevEng_PAJ7620::setCursorMode()
{
  writeRegisterArray(setCursorModeRegisterArray, SET_CURSOR_MODE_REG_ARRAY_SIZE);
}


/**
 * Gets cursor object's current X location
 * 
 * \note Only works in cursor mode
 * \param none
 * \return int : X coordinate of cursor
 */
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


/**
 * Gets cursor object's current Y location
 * 
 * \note Only works in cursor mode
 * \param none
 * \return int : Y coordinate of cursor
 */
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


/**
 * Returns whether an object is in view as a cursor
 * 
 * \note Only works in cursor mode
 * \param none
 * \return bool : True if object in view, False if no object in view
 */
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


/**
 * Inverts the X (horizontal) axis
 * 
 * \par
 * Allows you to choose the orientation of your coordinate system.
 * In all modes, the X axis is inverted. Left becomes Right, etc.
 * For cursor mode, the X values will flip
 * 
 * \param none
 * \return none
 */
void RevEng_PAJ7620::invertXAxis()
{
  uint8_t data = 0x00;
  selectRegisterBank(BANK1);
  readRegister(PAJ7620_ADDR_LENS_ORIENTATION, 1, &data);
  data ^= 1UL << 0;               // Bit[0] controls X axis
  writeRegister(PAJ7620_ADDR_LENS_ORIENTATION, data);
  selectRegisterBank(BANK0);
}


/**
 * Inverts the Y (vertical) axis
 * 
 * \par
 * Allows you to choose the orientation of your coordinate system.
 * In all modes, the Y axis is inverted. Up becomes Down, etc.
 * For cursor mode, the Y values will flip
 * 
 * \param none
 * \return none
 */
void RevEng_PAJ7620::invertYAxis()
{
  uint8_t data = 0x00;
  selectRegisterBank(BANK1);
  readRegister(PAJ7620_ADDR_LENS_ORIENTATION, 1, &data);
  data ^= 1UL << 1;                 // Bit[1] controls Y axis
  writeRegister(PAJ7620_ADDR_LENS_ORIENTATION, data);
  selectRegisterBank(BANK0);
}


/**
 * Disables sensor for reading & interrupts
 * \note This is the light disable state, not the full I2C shutdown state
 * \param none
 * \return none
 */
void RevEng_PAJ7620::disable()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_OPERATION_ENABLE, PAJ7620_DISABLE);
  selectRegisterBank(BANK0);
}


/**
 * Enables sensor for reading & interrupts
 * 
 *  \param none
 *  \return none
 */
void RevEng_PAJ7620::enable()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_OPERATION_ENABLE, PAJ7620_ENABLE);
  selectRegisterBank(BANK0);
}

/**
 * Sets time sensor waits between getGesture call to reading gesture from sensor
 * \par
 *  This time is most important in hardware interrupt driven use of the driver.
 *  The PAJ7620's interrupt pin will raise when a gesture is first recognized.
 *  If the user is trying to move their hand to do a Backward gesture, they will
 *  first trip a lateral (up, down, left, right) gesture, which will immediately
 *  raise the interrupt.
 *  By increasing this value, the user shall have more time to reach in and complete
 *  their intended gesture before the interrupt is handled.
 * \note Default value for entry time is 0
 * \param newGestureEntryTime : milliseconds (ms) for delay
 * \return none
 */
void RevEng_PAJ7620::setGestureEntryTime(unsigned long newGestureEntryTime)
{
  gestureEntryTime = newGestureEntryTime;
}


/**
 * Sets time sensor waits during getGesture() after gesture value read
 * \par
 *  This value represents the time the user has to exit the sensor's field of view
 *  before the next gesture might be read, which is most important in the Z axis gestures
 *  (forward and backward).
 *  Setting this lower makes the driver delay less so the main program can control
 *  more of the global timing, but puts responsibility on the coder to take this higher
 *  sensitivity into account.
 * \note Default value for exit time is 200
 * \param newGestureEntryTime : milliseconds (ms) for delay
 * \return none
 */
void RevEng_PAJ7620::setGestureExitTime(unsigned long newGestureExitTime)
{
  gestureExitTime = newGestureExitTime;
}


/**
 * Set sensor to "game mode" sampling speed of 240fps
 * \note Value of 0x30 for setting comes from PixArt contact
 *
 * \param none
 * \return none
 */
void RevEng_PAJ7620::setGameSpeed()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_R_IDLE_TIME_0, PAJ7620_GAME_SPEED);
  selectRegisterBank(BANK0);
}


/**
 * Set sensor to "normal" sampling speed of 120fps
 * \note Value of 0xAC for setting comes from PixArt contact
 *
 * \param none
 * \return none
 */
void RevEng_PAJ7620::setNormalSpeed()
{
  selectRegisterBank(BANK1);
  writeRegister(PAJ7620_ADDR_R_IDLE_TIME_0, PAJ7620_NORMAL_SPEED);
  selectRegisterBank(BANK0);
}
=======
/*
void RevEng_PAJ7620::setGameMode()
{
*/
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

/*
  selectRegisterBank(BANK1);
  writeRegister(0x65, 0x12);
  selectRegisterBank(BANK0);
}
*/


/**
 * Clear current gesture interrupt vectors without returning gesture value
 * \note The gesture interrupt vectors are reset in hardware after any reads
 *
 * \param none
 * \return none
 */
void RevEng_PAJ7620::clearGestureInterrupts()
{
    uint8_t data = 0, data1 = 0;
    getGesturesReg0(&data);
    getGesturesReg1(&data1);
}


/**
 * Get current count of waves by user
 * \param none
 * \return int : current count of "waves" over the sensor
 */
int RevEng_PAJ7620::getWaveCount()
{
  uint8_t waveCount = 0;
  readRegister(PAJ7620_ADDR_WAVE_COUNT, 1, &waveCount);
  waveCount &= 0x0F;      // Count is [3:0] bits - values in 0..15
  return waveCount;
}


/**
 * Double check to see if user is executing a Z-axis gesture 
 * 
 * \par
 *  This is there the gestureEntryTime and gestureExitTime delays are executed
 *  to buffer high speed polling & return against human gesture speeds.
 * \param initialGesture : The gesture initially found when getGesture() was called
 * \return \link Gesture \endlink : Either the initialGesture or the updated one if the user does another one
 */
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


/**
 * Reads the latest gesture from the device
 * 
 * \par
 *  This is the central method for reading and calculating the main 9 gestures
 *  the PAJ7620 can recognize. It returns a Gesture enum with the read gesture,
 *  which can by GES_NONE if no gesture was currently found.
 * \note Clears interrupt vector of gestures when called
 * \param none
 * \return \link Gesture \endlink found or \link GES_NONE Gesture::GES_NONE \endlink if no gesture found
 */
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
        getGesturesReg1(&data1);      // Bank 1 (Reg 0x44) has wave flag
        if (data1 == GES_WAVE_FLAG)
          { result = GES_WAVE; }
        break;
    }
  }
  return result;
}


/**
 * Read object's "brightness"
 * 
 * \par
 * Objects in view have their IR reflection measured. This interface returns a
 *  measure of this brightness from 0..255
 * \return int brightness: brightness value 0..255
 */
int RevEng_PAJ7620::getObjectBrightness()
{
  uint8_t brightness = 0x00;
  readRegister(PAJ7620_ADDR_OBJECT_BRIGHTNESS, 1, &brightness);
  return brightness;
}


/**
 * Read object's size (in pixels)
 * 
 * \par
 * The sensor has a 30x30 IR LED array. This interface returns a count of
 *  how many pixels are part of the object in view being tracked.
 * \return int size: value 0..900
 */
int RevEng_PAJ7620::getObjectSize()
{
  uint8_t data0, data1 = 0x00;
  int result = 0;
  readRegister(PAJ7620_ADDR_OBJECT_SIZE_LSB, 1, &data0);
  readRegister(PAJ7620_ADDR_OBJECT_SIZE_MSB, 1, &data1);
  result = data1;
  result = result << 8;
  result |= data0;
  return result;
}


/**
 * Get how long since the object left the view in gesture mode
 * 
 * \par
 * When an object has left the sensor's view, this register starts counting up.
 *  It's counting in ticks, roughly one per 7.2ms. It maxes out at 255, which
 *  happens at about 1830ms
 * \return int : ticks value 0..255
 */
int RevEng_PAJ7620::getNoObjectCount()
{
  uint8_t data0 = 0x00;
  readRegister(PAJ7620_ADDR_NO_OBJECT_COUNT, 1, &data0);
  return (int)data0;
}


/**
 * Get how long no motion has been seen in gesture mode
 * 
 * \par
 * This counts how long it has been since motions has occurred in front of the sensor.
 * This counts even if there's an object in view when it isn't moving.
 * Eratta: This *should* return 0..255, but seems to stop at 12.
 * Each "count" is probably 7.2ms, but it's been tough to figure out.
 * \return int : ticks value 0..12
 */
int RevEng_PAJ7620::getNoMotionCount()
{
  uint8_t data0 = 0x00;
  readRegister(PAJ7620_ADDR_NO_MOTION_COUNT, 1, &data0);
  return (int)data0;
}


/**
 * Gets Gesture object's current X location
 * 
 * \note Only works in gesture mode - default coordinates are 0 on right
 * \note Range seems to be 0..3712 in default gesture mode
 * \param none
 * \return int : X coordinate of cursor
 */
int RevEng_PAJ7620::getObjectCenterX()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_OBJECT_CENTER_X_LSB, 1, &data0);
  readRegister(PAJ7620_ADDR_OBJECT_CENTER_X_MSB, 1, &data1);
  data1 &= 0x1F;      // Mask off high bits (unused)
  result |= data1;
  result = result << 8;
  result |= data0;

  return result;
}


/**
 * Gets Gesture object's current Y location
 * 
 * \note Only works in gesture mode - default coordinates are 0 on top
 * \note Range seems to be 0..3712 in default gesture mode
 * \param none
 * \return int : Y coordinate of cursor
 */
int RevEng_PAJ7620::getObjectCenterY()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_OBJECT_CENTER_Y_LSB, 1, &data0);
  readRegister(PAJ7620_ADDR_OBJECT_CENTER_Y_MSB, 1, &data1);
  data1 &= 0x1F;      // Mask off high bits (unused)
  result |= data1;
  result = result << 8;
  result |= data0;

  return result;
}

/**
 * Gets object's current X velocity's raw value
 * 
 * \note Range seems to be -63..63
 * \param none
 * \return int : X velocity -63..63
 */
int RevEng_PAJ7620::getObjectVelocityX_raw()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_OBJECT_VEL_X_LSB, 1, &data0);
  readRegister(PAJ7620_ADDR_OBJECT_VEL_X_MSB, 1, &data1);

  data0 &= 0x3F;        // Yup, see wiki for reason
  result = data0;
  if(data1) { result *= -1; }

  return result;
}


/**
 * Gets object's current Y velocity's raw value
 * 
 * \note Range seems to be -63..63
 * \param none
 * \return int : Y velocity -63..63
 */
int RevEng_PAJ7620::getObjectVelocityY_raw()
{
  int result = 0;
  uint8_t data0 = 0x00;
  uint8_t data1 = 0x00;

  readRegister(PAJ7620_ADDR_OBJECT_VEL_Y_LSB, 1, &data0);
  readRegister(PAJ7620_ADDR_OBJECT_VEL_Y_MSB, 1, &data1);
  data0 &= 0x3F;        // Yup, see wiki for reason
  result = data0;
  if(data1) { result *= -1; }

  return result;
}


/**
 * Gets object's current X velocity's value
 * 
 * \par
 * Value filtered to zero if object not in view
 * \note Range seems to be -63..63
 * \param none
 * \return int : X velocity -63..63
 */
int RevEng_PAJ7620::getObjectVelocityX()
{
  if(!isObjectInView()) {
    return 0;
  } else {
    return getObjectVelocityX_raw();
  }
}


/**
 * Gets object's current Y velocity's value
 * 
 * \par
 * Value filtered to zero if object not in view
 * \note Range seems to be -63..63
 * \param none
 * \return int : Y velocity -63..63
 */
int RevEng_PAJ7620::getObjectVelocityY()
{
  if(!isObjectInView()) {
    return 0;
  } else {
    return getObjectVelocityY_raw();
  }
}


/**
 * Gets whether an object is in view or not
 * 
 * \param none
 * \return bool : true if object in view
 */
bool RevEng_PAJ7620::isObjectInView()
{
  if(getNoObjectCount())
  {
    return false;
  }
  return true;
}


/**
 * Gets which quadrant an object is in
 * 
 * \param none
 * \return Corner : [NW, NW, SW, SE] quadrants, middle/buffer, NONE for no object in view
 */
Corner RevEng_PAJ7620::getCorner()
{
  Corner ret = CORNER_NONE;
  int object_x, object_y = 0;

  if( !isObjectInView() ) {   // Bail if no object in view
    return CORNER_NONE;
  }

  object_x = getObjectCenterX();
  object_y = getObjectCenterY();

  if( object_x < CORNERS_BUFFER_LOWER && object_y < CORNERS_BUFFER_LOWER ) {
    return CORNER_NE;
  }
  else if( object_x > CORNERS_BUFFER_UPPER && object_y < CORNERS_BUFFER_LOWER ) {
    return CORNER_NW;
  }
  else if( object_x > CORNERS_BUFFER_UPPER && object_y > CORNERS_BUFFER_UPPER ) {
    return CORNER_SW;
  }
  else if( object_x < CORNERS_BUFFER_LOWER && object_y > CORNERS_BUFFER_UPPER ) {
    return CORNER_SE;
  }
  else {
    return CORNER_MIDDLE;     // Is in view, but not fully in a corner yet
  }
}
