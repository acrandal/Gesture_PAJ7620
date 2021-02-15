/*
  Example Script: paj7620_object_size_brightness.ino
  Package: RevEng_PAJ7620

  Copyright (c) 2021 Aaron S. Crandall
  Author       : Aaron S. Crandall <crandall@gonzaga.edu>
  Modified Time: February 2021

  Description: This demo demonstrates the ability to measure an object’s size and IR reflected brightness to the sensor.
   These APIs run in Gesture Mode. Size is in the range 0..900, which is the number of pixels reflecting IR back to
   the sensor within the 30x30 image array.
   “Brightness” is a scaled value of 0..255, but doesn’t seem to have any units documented. 

  License: Same as package under MIT License (MIT)
*/

// Includes enum definition of GES_* return values from readGesture()
#include "RevEng_PAJ7620.h"

// Create gesture sensor driver object
RevEng_PAJ7620 sensor = RevEng_PAJ7620();


// ******************************************************************
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);     // Configure LED for output

  Serial.begin(115200);

  if( !sensor.begin() )             // return value of 1 == success
  {
    Serial.print("PAJ7620 init error -- device not found -- halting");
    while(true) {}
  }

  Serial.println("PAJ7620U2 init: OK.");
  Serial.println("Place an object in front of the sensor to see it's IR brightness (0..255) and size (0..900)");
  delay(2000);
}


// ******************************************************************
void loop()
{
  Serial.println("**************************************");
  
  int brightness = 0;
  brightness = sensor.getObjectBrightness();
  Serial.print("Brightness:  ");
  Serial.println(brightness);

  int objectSize = 0;
  objectSize = sensor.getObjectSize();
  Serial.print("Object Size: ");
  Serial.println(objectSize);

  delay(100);
}
