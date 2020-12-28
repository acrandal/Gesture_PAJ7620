# Gesture Sensor - PAJ7620 / PAJ7620U2 / PAJ7620F2

## Description ##

The library here is for the PAJ7620 sensor with an I2C interface. The device also has several variants such as the PAJ7620U2 and PAJ7620F2.
The device provides simple motion-based gesture recognition.
It can recognize 9 hand moving gestures including:
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

There's a great tutorial on Arduino library installation at: http://learn.adafruit.com/adafruit-all-about-arduino-libraries-install-use

**Installing using arduino-cli**

The arduino-cli (command line interface) tool keeps and maintains the same libraries as your ArduinoIDE environment.
This library can be installed on the terminal with these commands:

> arduino-cli lib update-index  
> arduino-cli lib install "RevEng PAJ7620"  

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

--- 

## Key code notes ##

Major releases of this library have doxygen pages hosted on GitHub pages [here](https://acrandal.github.io/RevEng_PAJ7620/).
The rest of this section gives a summary and notes of how the library's major features are designed.

To use this library, include the libary's header:
> #include "RevEng_PAJ7620.h"

Create the driver object for the sensor:
> RevEng_PAJ7620 sensor = RevEng_PAJ7620();

In setup(), initialize and configure the driver:
> sensor.begin();

To read any gestures detected, use the readGesture() interface:
> Gesture gesture = sensor.readGesture();


The RevEng_PAJ7620.h header defines an enumerated type called Gesture for the gesture results.
This enum provides a value for all 9 built in gestures and a "no gesture" result GES_NONE.
The types it returns are:
GES_NONE, GES_UP, GES_DOWN, GES_LEFT, GES_RIGHT, GES_FORWARD, GES_BACKWARD, GES_CLOCKWISE, GES_ANTICLOCKWISE, GES_WAVE

(GES stands for gesture)

### Sensor timing/sensitivity controls ###

Reading a gesture has timing issues, notably the time it takes for the person to position their hand before starting a gesture and the time to remove their hand after finishing the gesture.
These values are most important for the forward and backward gestures.
To provide the ability to tune these timing values, two interfaces are provided to set their values in milliseconds:
> void setGestureEntryTime(unsigned long newGestureEntryTime);  
> void setGestureExitTime(unsigned long newGestureExitTime);

These values are defaulted to Entry Time: 0ms and Exit Time: 200ms.
You might need to play with these values for your given application and expected use behaviors.

### Wave Gesture Count ###

The PAJ7620 sensor counts waves (rapid back and forth passes of an object) using a 4 bit register.
Once the waving stops is when it raises the wave interrupt.
The quantity of waves is available for reading and is exposed using this interface:
> int wave_count = sensor.getWaveCount();

### Enabling and Disabling sensor ###

The PAJ7620 can be enabled and disabled during runtime.
After the call to begin(), the device is already enabled and running.
This library provides an interface to disable the devices and enable it as needed via these calls:
> sensor.disable();  
> sensor.enable();

When disabled, the sensor will not raise any interrupts, either on the interrupt pin or when calling readGesture();  
NOTE: There is a more complete disabling that turns of all I2C messages, but that is not implemented fully here.

### Multiple or Non-default Wire bus use ###

To use other I2C busses than the default Wire, use the begin() initializer by passing a pointer to the bus you want.
The library will store the bus of choice and use it from there on out.
For example:
> sensor.begin(&Wire1);

You can use multiple PAJ7620 devices on an Arduino, with one per I2C bus if your device has multiple busses available.
This would look like:

> RevEng_PAJ7620 sensor = RevEng_PAJ7620();  
> sensor.begin(&Wire);  
> RevEng_PAJ7620 sensor2 = RevEng_PAJ7620();  
> sensor2.begin(&Wire1);  

### Cursor Mode Summary ###

The PAJ7620 sensor is capable of a 'cursor' mode.
This is essentially a finger/object tracking capability where the center of the closest object in view is reported using (x,y) coordinates.
This mode would be used for higher fidelity applications.
The instantiation of the class is the same, and once it's initialized you can change to cursor mode and start tracking a target.
> sensor.setCursorMode();  
> sensor.isCursorInView();  
> sensor.getCursorX();  
> sensor.getCursorY();  

---

## Library History ##

#### Version 1.4.0 ####

- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu> and [Sean Kallaher](https://github.com/skallaher).
- Developed cursor mode - an object tracking on a (x,y) coordinate system
- Doxygen documentation system put in place
- GitHub pages documentation updates on master merges and version tags
- CI build and test framework, currently building on Linux, MacOS, and Windows
- Added APIs for inverting the X and Y axes - works in all modes

#### Version 1.3.0 ####
- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu>.
- Library now has a begin(TwoWire *bus) interface to allow user to pass a chosen TwoWire bus object for handling situations where they're using a second (or third) I2C bus on their device. For example: sensor.begin(&Wire1).
- Files re-arranged to comport with the [Arduino IDE/CLI library specification](https://arduino.github.io/arduino-cli/latest/library-specification/) for 1.5.0+
- Updated library.properties for the RevEng arduino library releases.
- Added keywords.txt for ArduinoIDE syntax highlighting for driver.

#### Version 1.2.0 ####
- Work done by [Aaron S. Crandall](https://github.com/acrandal) \<crandall@gonzaga.edu>.  
- Library is able to return a current count of waves by the user.  
- Library now provides several extra example Arduino programs to showcase using the wave feature and more of the API.  
- Incorporated PROGMEM based approach to storing the extensive initialization configuration adapted from the [Jaycar Electronics](https://www.jaycar.com.au/) fork, saving 21% of SRAM memory on an Arduino Uno (ATMega328P) board.  
- Lots of object refactoring and method layout to make it easier to read and test.
- Plenty of small formatting, commenting updates, and other details.  
- Bugfix: Brought interrupt bits into alignment with official documentation for gesture directions (see v0.8 of PixArt device documentation).

#### Version 1.1.0 ####
- Work done by [Marc Finns](https://github.com/MarcFinns).  
- Based on the original SEEED library but with higher level API and fully encapsulated in a C++ object (no global variables).   
- No need for registry manipulation in user code anymore.  
- A gesture event is returned with an API call.  
- Library provides an interrupt based example using the PAJ7620 object.  

#### Version 1.0.0 ####
- [SEEED](https://www.seeedstudio.com/) released [library](https://github.com/Seeed-Studio/Gesture_PAJ7620) is able to identify and return all 9 gestures and up to 15 calculated gestures in example Arduino code.


**Contributors**
- Original Code from SEEED - Jack Wu, Wuruibin, & Xiangnan, June 2015
- Initial class-based solution - Marc Finns, 2017
- PROGMEM adapted from Jaycar Elctronics' fork, December 2020
- Overhaul, documentation, and current version by Aaron S. Crandall, December 2020
- Pull requests, CI framework, and doxygen materials by Sean Kallaher (skallaher), December 2020

**License**  
MIT - see LICENSE file

**Version**  
1.4.0
