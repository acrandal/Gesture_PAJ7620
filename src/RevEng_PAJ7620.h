/**
  \file RevEng_PAJ7620.h
  \author Aaron S. Crandall

  \copyright
  \parblock
  Copyright (c) 2015 seeed technology inc.
  Website    : www.seeed.cc
  Author     : Wuruibin & Xiangnan
  Modified Time: June 2015

  2017 - Modified by MarcFinns to encapsulate in class without global variables
  2020 - PROGMEM code adapted from Jaycar-Electronics' work
  2020 - Modified by Aaron S. Crandall <crandall@gonzaga.edu>

  Version: 1.3.0

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
  \endparblock

  PAJ7620U2 Sensor data sheet for reference found here:
    https://datasheetspdf.com/pdf-file/1309990/PixArt/PAJ7620U2/1
*/

#ifndef __PAJ7620_H__
#define __PAJ7620_H__

#include <Arduino.h>
#include <Wire.h>

#if defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266) || defined(ARDUINO_ARCH_ESP32)
#define PROGMEM_COMPATIBLE
/*
  #if defined(__AVR__) || defined(ARDUINO_ARCH_ESP8266)
    #include <avr/pgmspace.h>
  #elif defined(ARDUINO_ARCH_ESP32)
    #include <pgmspace.h>
  #endif
  */
#endif

/** @name Device Constants */
/**@{*/

/** DEVICE'S I2C ID - defined by manufacturer */
#define PAJ7620_I2C_BUS_ADDR              0x73


/** Base address for accessing registers */
#define PAJ7620_ADDR_BASE                 0x00

/** Register bank select address */
#define PAJ7620_REGISTER_BANK_SEL         (PAJ7620_ADDR_BASE + 0xEF)  // W
/**@}*/

/** @name ID values 
*   Device's hard coded ID values
*/
/**@{*/
#define PAJ7620_PART_ID_LSB               0x20
#define PAJ7620_PART_ID_MSB               0x76
/**@}*/

/** @name REGISTER BANK 0
 *  Addresses used within register bank 0
 */
/**@{*/
/** \note Readonly */
#define PAJ7620_ADDR_PART_ID_0            (PAJ7620_ADDR_BASE + 0x00)  // R
/** \note Readonly */
#define PAJ7620_ADDR_PART_ID_1            (PAJ7620_ADDR_BASE + 0x01)  // R
/** \note Write only */
#define PAJ7620_ADDR_SUSPEND_CMD          (PAJ7620_ADDR_BASE + 0x03)  // W
/** \note Read/Write */
#define PAJ7620_ADDR_GES_PS_DET_MASK_0    (PAJ7620_ADDR_BASE + 0x41)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_GES_PS_DET_MASK_1    (PAJ7620_ADDR_BASE + 0x42)  // RW
/** \note Readonly */
#define PAJ7620_ADDR_GES_PS_DET_FLAG_0    (PAJ7620_ADDR_BASE + 0x43)  // R
/** \note Readonly */
#define PAJ7620_ADDR_GES_PS_DET_FLAG_1    (PAJ7620_ADDR_BASE + 0x44)  // R
/** \note Readonly */
#define PAJ7620_ADDR_STATE_INDICATOR      (PAJ7620_ADDR_BASE + 0x45)  // R
/** \note Read/Write */
#define PAJ7620_ADDR_PS_HIGH_THRESHOLD    (PAJ7620_ADDR_BASE + 0x69)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_PS_LOW_THRESHOLD     (PAJ7620_ADDR_BASE + 0x6A)  // RW
/** \note Readonly */
#define PAJ7620_ADDR_PS_APPROACH_STATE    (PAJ7620_ADDR_BASE + 0x6B)  // R
/** \note Readonly */
#define PAJ7620_ADDR_PS_RAW_DATA          (PAJ7620_ADDR_BASE + 0x6C)  // R
/** \note Readonly */
#define PAJ7620_ADDR_WAVE_COUNT           (PAJ7620_ADDR_BASE + 0xB7)  // R
/** \note Readonly */
#define PAJ7620_ADDR_GES_RESULT_0         (PAJ7620_ADDR_BASE + 0x43)  // R
/** \note Readonly */
#define PAJ7620_ADDR_GES_RESULT_1         (PAJ7620_ADDR_BASE + 0x44)  // R
/**@}*/

// Cursor Registers - Bank 0
#define PAJ7620_ADDR_CURSOR_X_LOW         (PAJ7620_ADDR_BASE + 0x3B)  // R
#define PAJ7620_ADDR_CURSOR_X_HIGH        (PAJ7620_ADDR_BASE + 0x3C)  // R
#define PAJ7620_ADDR_CURSOR_Y_LOW         (PAJ7620_ADDR_BASE + 0x3D)  // R
#define PAJ7620_ADDR_CURSOR_Y_HIGH        (PAJ7620_ADDR_BASE + 0x3E)  // R
#define PAJ7620_ADDR_CURSOR_INT           (PAJ7620_ADDR_BASE + 0x44)  // R

// REGISTER BANK 1
/** @name REGISTER BANK 1
 *  Addresses used within register bank 1
 */
/**@{*/
/** \note Read/Write */
#define PAJ7620_ADDR_PS_GAIN              (PAJ7620_ADDR_BASE + 0x44)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_IDLE_S1_STEP_0       (PAJ7620_ADDR_BASE + 0x67)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_IDLE_S1_STEP_1       (PAJ7620_ADDR_BASE + 0x68)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_IDLE_S2_STEP_0       (PAJ7620_ADDR_BASE + 0x69)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_IDLE_S2_STEP_1       (PAJ7620_ADDR_BASE + 0x6A)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_OP_TO_S1_STEP_0      (PAJ7620_ADDR_BASE + 0x6B)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_OP_TO_S1_STEP_1      (PAJ7620_ADDR_BASE + 0x6C)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_OP_TO_S2_STEP_0      (PAJ7620_ADDR_BASE + 0x6D)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_OP_TO_S2_STEP_1      (PAJ7620_ADDR_BASE + 0x6E)  // RW
/** \note Read/Write */
#define PAJ7620_ADDR_OPERATION_ENABLE     (PAJ7620_ADDR_BASE + 0x72)  // RW
/**@}*/

/** @name Register bank IDs */
/**@{*/
#define PAJ7620_BANK0                     0x00
#define PAJ7620_BANK1                     0x01
/**@}*/

/** @name Suspend Control commands
* Written to #PAJ7620_ADDR_SUSPEND_CMD
* */
/**@{*/
/** Wakeup the device */
#define PAJ7620_I2C_WAKEUP                0x01
/** Suspend the device */
#define PAJ7620_I2C_SUSPEND               0x00
/**@}*/

/** @name Enable Control commands
 * Written to #PAJ7620_ADDR_OPERATION_ENABLE
 */
/**@{*/
/** Enable the device to start reading data*/
#define PAJ7620_ENABLE                    0x01
/** Disable the device and stop reading data*/
#define PAJ7620_DISABLE                   0x00
/**@}*/

/** @name Gesture Bit Masks
 * Return values from gesture I2C memory reads in Bank 0 - 0x43 & 0x44
 *
 * \see #PAJ7620_ADDR_GES_RESULT_0
 * \see #PAJ7620_ADDR_GES_RESULT_1
 */
/**@{*/
/** Bit set -> Up gesture detected. \note Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_UP_FLAG                       0x01
/** Bit set -> Down gesture detected. \note Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_DOWN_FLAG                     0x02
/** Bit set -> Left gesture detected. \note Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_LEFT_FLAG                     0x04
/** Bit set -> Right gesture detected. \note Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_RIGHT_FLAG                    0x08
/** Bit set -> Forward gesture detected. \note: Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_FORWARD_FLAG                  0x10
/** Bit set -> Backward gesture detected. \note: Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_BACKWARD_FLAG                 0x20
/** Bit set -> Clockwise gesture detected. \note: Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_CLOCKWISE_FLAG                0x40
/** Bit set -> Anticlockwise gesture detected. \note: Read from #PAJ7620_ADDR_GES_RESULT_0 */
#define GES_ANTI_CLOCKWISE_FLAG           0x80
/** Bit set -> Wave gesture detected. \note: Read from #PAJ7620_ADDR_GES_RESULT_1 */
#define GES_WAVE_FLAG                     0x01      // Read from Bank0 - 0x44
/**@}*/

// Return values for cursor interrupt/status for cursor mode
//  Read from Bank 0, reg 0x44
#define CUR_HAS_OBJECT                    0x04      // Bit 2 - 0000 0100
#define CUR_NO_OBJECT                     0x80      // Bit 7 - 1000 0000


#define INIT_CURSOR_REG_ARRAY_SIZE (sizeof(initCursorRegisterArray)/sizeof(initCursorRegisterArray[0]))

#ifdef PROGMEM_COMPATIBLE
const unsigned short initCursorRegisterArray[] PROGMEM = {
#else
const unsigned short initCursorRegisterArray[] = {
#endif
    0xEF00,   // Set Bank 0
    0x3229,   // Default  29  [0] Cursor use top - def 1
              //              [1] Cursor Use BG Model - def 0
              //              [2] Cursor Invert Y - def 0       -- Not sure, doesn't seem to work
              //              [3] Cursor Invert X - def 1
              //              [5:4] Cursor top Ratio - def 0x2
    0x3301,   // Default  01  R_PositionFilterStartSizeTh [7:0]
    0x3400,   // Default  00  R_PositionFilterStartSizeTh [8]
    0x3501,   // Default  01  R_ProcessFilterStartSizeTh [7:0]
    0x3600,   // Default  00  R_ProcessFilterStartSizeTh [8]
    0x3703,   // Default  09  R_CursorClampLeft [4:0]
    0x381B,   // Default  15  R_CursorClampRight [4:0]
    0x3903,   // Default  0A  R_CursorClampUp [4:0]
    0x3A1B,   // Default  12  R_CursorClampDown [4:0]
    0x4100,   // Interrupt enable mask - Should be 00 (disable gestures)
              //              All gesture flags [7:0]
    0x4284,   // Interrupt enable mask - Should be 84 (0b 1000 0100)
              //              bit 0: Wave, wave mode use only
              //              bit 1: Proximity, proximity mode use only
              //              bit 2: Has Object, cursor mode use only
              //              bit 3: Wake up trigger, trigger mode use only
              //              bit 4: Confirm, confirm mode use only
              //              bit 5: Abort, confirm mode use only
              //              bit 6: N/A
              //              bit 7:No Object, cursor mode use only
    0x8B01,   // Default  10  R_Cursor_ObjectSizeTh [7:0]
    0x8C07,   // Default  07  R_PositionResolution [2:0]
    0xEF01,   // Set Bank 1
    0x0403,   // Invert X&Y Axes in lens for GUI coordinates 
              //  Where (0,0) is in upper left, positive down (Y) and right (X)
    0x7403,   // Enable cursor mode 0 - gesture, 3 - cursor, 5 - proximity
    0xEF00    // Set Bank 0 (parking it)

/** 
  Gesture result definitions.
  Used as return value from readGesture call
 */
enum Gesture {
  GES_NONE = 0,      /**< No gesture */
  GES_UP,            /**< Upwards gesture */
  GES_DOWN,	     /**< Downward gesture */
  GES_LEFT,          /**< Leftward gesture */
  GES_RIGHT,         /**< Rightward gesture */
  GES_FORWARD,       /**< Forward gesture */
  GES_BACKWARD,      /**< Backward gesture */
  GES_CLOCKWISE,     /**< Clockwise circular gesture */
  GES_ANTICLOCKWISE, /**< Anticlockwise circular gesture */
  GES_WAVE           /**< Wave gesture */
};

/**
  Used for selecting PAJ7620 memory bank to read/write from
  \author Aaron S. Crandall
 */
typedef enum {
  BANK0 = 0,
  BANK1,
} Bank_e;

/** Generated size of the register init array */
#define INIT_REG_ARRAY_SIZE (sizeof(initRegisterArray)/sizeof(initRegisterArray[0]))

/*******************************************************
* Initial Gesture I2C mode register addresses and values
* \note Has not been tested for SPI bus image mode.
*
* Changed to JayCar-Electronics PROGMEM approach from <a href="https://github.com/Jaycar-Electronics">their fork</a>.
* 
* Saves about 21% of SRAM on an Arduino Uno - Around 440 bytes
*
* Values taken from PixArt reference documentation v0.8
*******************************************************/
#ifdef PROGMEM_COMPATIBLE
const unsigned short initRegisterArray[] PROGMEM = {
#else
const unsigned short initRegisterArray[] = {
#endif
    0xEF00,
    0x3229,
    0x3301,
    0x3400,
    0x3501,
    0x3600,
    0x3707,
    0x3817,
    0x3906,
    0x3A12,
    0x3F00,
    0x4002,
    0x41FF,
    0x4201,
    0x462D,
    0x470F,
    0x483C,
    0x4900,
    0x4A1E,
    0x4B00,
    0x4C20,
    0x4D00,
    0x4E1A,
    0x4F14,
    0x5000,
    0x5110,
    0x5200,
    0x5C02,
    0x5D00,
    0x5E10,
    0x5F3F,
    0x6027,
    0x6128,
    0x6200,
    0x6303,
    0x64F7,
    0x6503,
    0x66D9,
    0x6703,
    0x6801,
    0x69C8,
    0x6A40,
    0x6D04,
    0x6E00,
    0x6F00,
    0x7080,
    0x7100,
    0x7200,
    0x7300,
    0x74F0,
    0x7500,
    0x8042,
    0x8144,
    0x8204,
    0x8320,
    0x8420,
    0x8500,
    0x8610,
    0x8700,
    0x8805,
    0x8918,
    0x8A10,
    0x8B01,
    0x8C37,
    0x8D00,
    0x8EF0,
    0x8F81,
    0x9006,
    0x9106,
    0x921E,
    0x930D,
    0x940A,
    0x950A,
    0x960C,
    0x9705,
    0x980A,
    0x9941,
    0x9A14,
    0x9B0A,
    0x9C3F,
    0x9D33,
    0x9EAE,
    0x9FF9,
    0xA048,
    0xA113,
    0xA210,
    0xA308,
    0xA430,
    0xA519,
    0xA610,
    0xA708,
    0xA824,
    0xA904,
    0xAA1E,
    0xAB1E,
    0xCC19,
    0xCD0B,
    0xCE13,
    0xCF64,
    0xD021,
    0xD10F,
    0xD288,
    0xE001,
    0xE104,
    0xE241,
    0xE3D6,
    0xE400,
    0xE50C,
    0xE60A,
    0xE700,
    0xE800,
    0xE900,
    0xEE07,
    0xEF01,
    0x001E,
    0x011E,
    0x020F,
    0x0310,
    0x0402,
    0x0500,
    0x06B0,
    0x0704,
    0x080D,
    0x090E,
    0x0A9C,
    0x0B04,
    0x0C05,
    0x0D0F,
    0x0E02,
    0x0F12,
    0x1002,
    0x1102,
    0x1200,
    0x1301,
    0x1405,
    0x1507,
    0x1605,
    0x1707,
    0x1801,
    0x1904,
    0x1A05,
    0x1B0C,
    0x1C2A,
    0x1D01,
    0x1E00,
    0x2100,
    0x2200,
    0x2300,
    0x2501,
    0x2600,
    0x2739,
    0x287F,
    0x2908,
    0x3003,
    0x3100,
    0x321A,
    0x331A,
    0x3407,
    0x3507,
    0x3601,
    0x37FF,
    0x3836,
    0x3907,
    0x3A00,
    0x3EFF,
    0x3F00,
    0x4077,
    0x4140,
    0x4200,
    0x4330,
    0x44A0,
    0x455C,
    0x4600,
    0x4700,
    0x4858,
    0x4A1E,
    0x4B1E,
    0x4C00,
    0x4D00,
    0x4EA0,
    0x4F80,
    0x5000,
    0x5100,
    0x5200,
    0x5300,
    0x5400,
    0x5780,
    0x5910,
    0x5A08,
    0x5B94,
    0x5CE8,
    0x5D08,
    0x5E3D,
    0x5F99,
    0x6045,
    0x6140,
    0x632D,
    0x6402,
    0x6596,
    0x6600,
    0x6797,
    0x6801,
    0x69CD,
    0x6A01,
    0x6BB0,
    0x6C04,
    0x6D2C,
    0x6E01,
    0x6F32,
    0x7100,
    0x7201,
    0x7335,
    0x7400,
    0x7533,
    0x7631,
    0x7701,
    0x7C84,
    0x7D03,
    0x7E01};


/**
 * PAJ7620 Device API class - As developed by RevEng Devs
 *
 * Provides a user friendly interface to the PAJ7620 device
 * \author Aaron S. Crandall
 */
class RevEng_PAJ7620
{
  public:
    uint8_t begin();
    uint8_t begin(TwoWire * chosenWireHandle);    // Ex: begin(&Wire1)

    /** @name Gesture mode controls */
    /**@{*/
    Gesture readGesture();
    void clearGesture();

    void setGestureEntryTime(unsigned long newGestureEntryTime);
    void setGestureExitTime(unsigned long newGestureExitTime);

    int getWaveCount();
    /**@}*/

    // void setGameMode(); // No documentation for this mode is available (yet)
    void disable();                 // Suspend interrupts (both pin and registers)
    void enable();                  // Resume interrupts (both pin and registers)

    void setGestureMode();          // Put sensor into gesture mode
    void setCursorMode();           // Put sensor into cursor mode
    bool isCursorInView();          // Cursor object in view

    int getCursorX();               // Get cursor's X axis location
    int getCursorY();               // Get cusors's Y axis location

  private:
    unsigned long gestureEntryTime; // User set gesture entry delay in ms (default: 0)
    unsigned long gestureExitTime;  // User set gesture exit delay in ms (default 200)

    TwoWire *wireHandle;            // User selected Wire bus (default: Wire)

    uint8_t writeRegister(uint8_t addr, uint8_t cmd);
    uint8_t readRegister(uint8_t addr, uint8_t qty, uint8_t data[]);

    void selectRegisterBank(Bank_e bank);

    uint8_t getGesturesReg0(uint8_t data[]);
    uint8_t getGesturesReg1(uint8_t data[]);

    Gesture forwardBackwardGestureCheck(Gesture initialGesture);

    bool isPAJ7620UDevice();
    void initializeDeviceSettings();
};

#endif

