# Gesture Sensor - PAJ7620 / PAJ7620U2 / PAJ7620F2

## Description ##

The library here is for the PAJ7620 sensor with an I2C interface. The device also has several variants such as the PAJ7620U2 and PAJ7620F2.
The device provides simple motion-based gesture recognition, object tracking (cursor mode), and other (unimplemented so far) modes.

In gesture mode it can recognize 9 hand moving gestures including:
- move up
- move down
- move left
- move right
- circle clockwise
- circle anti-clockwise (counter-clockwise / widdershins)
- forwards / towards
- backards / away
- wave

The device also counts the quantity of hand waves, which is now available via an API call in version 1.2.

The sensor's datasheet is available [here](https://datasheetspdf.com/pdf-file/1309990/PixArt/PAJ7620U2/1).

The PAJ7620U2 is available from various vendors such as Seeed Studio on a breakout board.
For more information about their product, please visit the [wiki](http://wiki.seeedstudio.com/Grove-Gesture_v1.0/).
Breakout boards are also sold under the part number GY-PAJ7620 by various unnamed manufacturers.

---

## Installation ##

**Via ArduinoIDE/CLI**

This library is available in the Arduino extended library collection.
To install:
- Open ArduinoIDE
- Select Tools->Manage Libraries...
- In the search bar input: RevEng PAJ7620
- Hover over the result and click "install"

**Selecting a release from GitHub**

On the GitHub page (https://github.com/acrandal/RevEng_PAJ7620), there is a "releases" section in the right sidebar.
You can click on the release version you would like to use, or just "Releases" for the latest one.
On that page you'll find a "source code" link in various archive formats like tar.gz or zip. Download that and follow the directions in the next section here.

**Manual install from the Git repository**

To download from the repo, choose the branch you want, click the DOWNLOAD ZIP button, rename the uncompressed folder to RevEng_PAJ7620. Check that the RevEng_PAJ7620 folder contains a src directory with RevEng_PAJ7620.cpp and RevEng_PAJ7620.h

Place the RevEng_PAJ7620 library folder into your \<arduinoSketchFolder>/libraries/ folder. You may need to create the libraries subfolder if its your first library. Restart the IDE.

There's a great tutorial on Arduino library installation at: [http://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use](http://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use)

**Installing using arduino-cli**

The arduino-cli (command line interface) tool keeps and maintains the same libraries as your ArduinoIDE environment.
This library can be installed on the terminal with these commands:

- arduino-cli lib update-index  
- arduino-cli lib install "RevEng PAJ7620"  

---

## Example Sketches Provided ##

This package comes with several example scripts to get you rolling quickly.
In all cases, you need to connect the sensor to Vcc and Ground, plus the I2C lines: SDA & SDL.
For the interrupt driven sketch, you also need to connect the sensor's INT pin to pin 2 of your device.  
That should work for most arduino platforms (Uno, Mega).
See the Arduino [attachInterrupt](https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/) documentation for the full list of boards and their interrupt capable pins.

Sketches provided:

- paj7620_up_down       // Polls for only the up and down gestures and sets the builtin LED on and off
- paj7620_9_gestures    // Polls for all 9 gestures
- paj7620_interrupt     // Sets up an interrupt routine to wait for any gestures
- paj7620_wave_count    // Polls for the number of waves the sensor sees
- paj7620_cursor_demo   // Polls the sensor for a 'cursor', which is an object in front of the sensor. It then pulls the cursor location using (x,y) coordinates
- paj7620_object_size_brightness // Polls for an object's size (in pixels) and "brightness" (0..255)
- paj7620_corners       // Polls for which quadrant the object is in - NE/NW/SE/SW (or none/middle)

--- 

## Key code notes ##

Major releases of this library have doxygen pages hosted on GitHub pages [here](https://acrandal.github.io/RevEng_PAJ7620/latest/index.html).
The rest of this section gives a summary and notes of how the library's major features are designed.

To use this library, include the libary's header:
- #include "RevEng_PAJ7620.h"

Create the driver object for the sensor:
- RevEng_PAJ7620 sensor = RevEng_PAJ7620();

In setup(), initialize and configure the driver:
- sensor.begin();

To read any gestures detected, use the readGesture() interface:
- Gesture gesture = sensor.readGesture();


The RevEng_PAJ7620.h header defines an enumerated type called Gesture for the gesture results.
This enum provides a value for all 9 built in gestures and a "no gesture" result GES_NONE.
The types it returns are:
GES_NONE, GES_UP, GES_DOWN, GES_LEFT, GES_RIGHT, GES_FORWARD, GES_BACKWARD, GES_CLOCKWISE, GES_ANTICLOCKWISE, GES_WAVE

(GES stands for gesture)

### Sensor timing/sensitivity controls ###

Reading a gesture has timing issues, notably the time it takes for the person to position their hand before starting a gesture and the time to remove their hand after finishing the gesture.
These values are most important for the forward and backward gestures.
To provide the ability to tune these timing values, two interfaces are provided to set their values in milliseconds:
- void setGestureEntryTime(unsigned long newGestureEntryTime);  
- void setGestureExitTime(unsigned long newGestureExitTime);

These values are defaulted to Entry Time: 0ms and Exit Time: 200ms.
You might need to play with these values for your given application and expected use behaviors.
The entry time is most important to reduce false positive errors when trying for the backwards gesture. The exit time is most important for preventing double gesture detection, especially for the forward and backward gestures.

### Wave Gesture Count ###

The PAJ7620 sensor counts waves (rapid back and forth passes of an object) using a 4 bit register, so it holds values in the range 0..15.
Once the waving stops is when it raises the wave interrupt.
The quantity of waves is available for reading and is exposed using this interface:
- int wave_count = sensor.getWaveCount();

### Enabling and Disabling sensor ###

The PAJ7620 can be enabled and disabled during runtime.
After the call to begin(), the device is already enabled and running.
This library provides an interface to disable the devices and enable it as needed via these calls:
- sensor.disable();  
- sensor.enable();

When disabled, the sensor will not raise any interrupts, either on the interrupt pin or when calling readGesture();  
NOTE: There is a more complete disabling that turns of all I2C messages, but that is not implemented fully here.

### Multiple or Non-default Wire bus use ###

To use other I2C busses than the default Wire, use the begin() initializer by passing a pointer to the bus you want.
The library will store the bus of choice and use it from there on out.
For example:
- sensor.begin(&Wire1);

You can use multiple PAJ7620 devices on an Arduino, with one per I2C bus if your device has multiple busses available.
This would look like:

- RevEng_PAJ7620 sensor = RevEng_PAJ7620();
- sensor.begin(&Wire);
- RevEng_PAJ7620 sensor2 = RevEng_PAJ7620();  
- sensor2.begin(&Wire1);  

### Cursor Mode Summary ###

The PAJ7620 sensor is capable of a 'cursor' mode.
This is essentially a finger/object tracking capability where the center of the closest object in view is reported using (x,y) coordinates.
This mode would be used for higher fidelity applications.
The instantiation of the class is the same, and once it's initialized you can change to cursor mode and start tracking a target.
- sensor.setCursorMode();  
- sensor.isCursorInView();  
- sensor.getCursorX();  
- sensor.getCursorY();  

The program can return to gesture mode with the setGestureMode call:
- sensor.setGestureMode();

### Game Mode Summary ###

The PixArt datasheets refer prominently to "game mode" with the sensor.
The documentation talks about game mode providing 240fps instead of normal mode's 120fps.
After working with PixArt to get the correct values in place, these modes are set with the driver API's:
- sensor.setNormalSpeed();
- sensor.setGameSpeed();

The biggest side effect of using game speed is that if you're polling the sensor for events, especially cursor location updates, you can receive data results at around twice the rate with the game speed set.

NOTE: if you change modes (gesture to cursor or cursor to gesture) the speed shall return to normal speed until you call setGameSpeed() again.

### Object Size and Brightness ###

In Gesture mode, there are metrics about the visible object available.
Notably, you can pull the "brightness" (IR reflected light) and size (pixels) of any object in view.

- sensor.getObjectBrightness();
- sensor.getObjectSize();

Brightness is measured on a scale of 0..255 with no units.
Size is 0..900 where the value is how many pixels of the 30x30 IR camera are picking up an object.

### Object Motion States in Gesture Mode ###

In Gesture mode, you can get some stats on object visibility and motion.

- sensor.getNoObjectCount();
- sensor.getNoMotionCount();
- sensor.getObjectCenterX();
- sensor.getObjectCenterY();
- getObjectVelocityX();
- getObjectVelocityY();
- isObjectInView();

NoObjectCount reports the number of ticks since an object left the view.
It reports a value between 0..255. Each increment represents about 7.2ms in normal mode and 4.1ms in game mode.

Similarly, NoMotionCount is ticks since there was motion (even if there is still an object in view, but it's holding still). For some reason it only counts 0..12.

getObjectCenterX and getObjectCenterY return an int with a coordinate for any object in view. They both return 0 if there's no object. The results are in 0..3712.
NOTE: The X coordinate is reversed in gesture mode by default so 0 is on the right.
This can be flipped with the invertXAxis() interface, but will also reverse the gestures (GES_LEFT becomes GES_RIGHT, Clockwise becomes Anti-Clockwise).

getObjectVelocityX and getObjectVelocityY poll for a velocity of any object crossing the sensor's view.
They return 0 if no object is in view or if the object isn't moving.
They return an int from -63 to 63.
Negative and positive refer to the direction of travel.

isObjectInView returns true if there's an object in view.

### Object in Corner Interface ###

The Corner interface returns which quadrant an object is currently in.
This is called in Gesture Mode, but it works in cursor mode which has the quadrants are flipped.

NW | NE  
--MID--  
SW | SE  

- Corner result = sensor.getCorner();

getCorner() returns a Corner enum. This enum has six values (and synonyms):
- CORNER_NONE : No object in view
- CORNER_NE : Object in upper right quadrant (Quadrant I)
- CORNER_NW : Object in upper left quadrant (Quadrant II)
- CORNER_SW : Object in lower left quadrant (Quadrant III)
- CORNER_SE : Object in lower right quadrant (Quadrant IV)
- CORNER_MIDDLE : Object in buffer region between the corners (to reduce noise)

Note: Width of middle is set by CORNERS_BUFFER_WIDTH_PCT from .h file

---

## Library History ##

**Version 1.5.0**

- Implemented "game speed" and "normal speed" APIs to use values received from PixArt email response by Xavier Liu (Thank you!).
- Added getObjectSize() and getObjectBrightness() interfaces (Issues #58 & #59).
- Added getNoMotionCount() and getNoObjectCount interfaces (Issues #62).
- Added getObjectCenterX() and getObjectCenterY() interfaces in gesture mode.
- Added getObjectVelocityX(), getObjectVelocityY() and isObjectInView() interfaces in gesture mode (Issue #64).
- Added getCorner() interface to return where an object is from a cartesian quadrant perspective (Issue #52).

**Version 1.4.1**

- Fixed serious I2C initialization bug - needed to set memory bank twice to init reliably (see Issue #56)

**Version 1.4.0**

- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu> and [Sean Kallaher](https://github.com/skallaher) with input from [Ian](https://github.com/ianfixes) on arduino_ci use
- Developed cursor mode - an object tracking on a (x,y) coordinate system
- Added cursor mode example sketch
- Doxygen documentation system put in place
- GitHub pages documentation updates on master merges and version tags
- CI build and test framework, currently building on Ubuntu Linux
- Added APIs for inverting the X and Y axes - works in all modes
- Slimmed down the initialization array from 440 bytes to 100 bytes
- Created a proper gesture mode register array to allow returning to gesture mode from cursor mode
- Adjusted README to better align with doxygen formatting/output

**Version 1.3.0**

- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu>.
- Library now has a begin(TwoWire *bus) interface to allow user to pass a chosen TwoWire bus object for handling situations where they're using a second (or third) I2C bus on their device. For example: sensor.begin(&Wire1).
- Files re-arranged to comport with the [Arduino IDE/CLI library specification](https://arduino.github.io/arduino-cli/latest/library-specification/) for 1.5.0+
- Updated library.properties for the RevEng arduino library releases.
- Added keywords.txt for ArduinoIDE syntax highlighting for driver.

**Version 1.2.0**

- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu>.  
- Library is able to return a current count of waves by the user.  
- Library now provides several extra example Arduino programs to showcase using the wave feature and more of the API.  
- Incorporated PROGMEM based approach to storing the extensive initialization configuration adapted from the [Jaycar Electronics](https://www.jaycar.com.au/) fork, saving 21% of SRAM memory on an Arduino Uno (ATMega328P) board.  
- Lots of object refactoring and method layout to make it easier to read and test.
- Plenty of small formatting, commenting updates, and other details.  
- Bugfix: Brought interrupt bits into alignment with official documentation for gesture directions (see v0.8 of PixArt device documentation).

**Version 1.1.0**
- Work done by [Marc Finns](https://github.com/MarcFinns).  
- Based on the original SEEED library but with higher level API and fully encapsulated in a C++ object (no global variables).   
- No need for registry manipulation in user code anymore.  
- A gesture event is returned with an API call.  
- Library provides an interrupt based example using the PAJ7620 object.  

**Version 1.0.0**
- [SEEED](https://www.seeedstudio.com/) released [library](https://github.com/Seeed-Studio/Gesture_PAJ7620) is able to identify and return all 9 gestures and up to 15 calculated gestures in example Arduino code.


**Contributors**
- Original Code from SEEED - Jack Wu, Wuruibin, & Xiangnan, June 2015
- Initial class-based solution - Marc Finns, 2017
- PROGMEM adapted from Jaycar Elctronics' fork, December 2020
- Overhaul, documentation, and current version by Aaron S. Crandall, December 2020
- Pull requests, CI framework, and doxygen materials by Sean Kallaher (skallaher), December 2020
- arduino_ci input and knowledge from Ian / [ianfixes](https://github.com/ianfixes), December 2020

**License**  
MIT - see LICENSE file

**Version**  
1.5.0
