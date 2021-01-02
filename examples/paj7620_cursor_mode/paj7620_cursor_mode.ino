/*
  Example Script: paj7620_cursor_mode.ino
  Package: RevEng_PAJ7620

  Copyright (c) 2020 Aaron S. Crandall
  Author       : Aaron S. Crandall <crandall@gonzaga.edu>
  Modified Time: December 2020

  Description: This example shows off "cursor mode" of the sensor
    This mode outputs an (X,Y) pair when an object is in view.
    It is effectively a "mouse mode" or "pointer mode"
    The intended use would be a low res non-contact pointer for input
    It would make a really cool game interface!
    This demo also enables the LED_BUILTIN - 
     when a cursor is in view the LED lights up.

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

  sensor.setCursorMode();           // Put sensor into cursor mode
                                    //  stops gestures being detected

  Serial.println("Wave your hand over the sensor to see cursor coordinates.");
  Serial.println("When a cursor (object) is in view, ");
  Serial.println("the LED_BUILTIN should light up too.");
}


// ******************************************************************
void loop()
{
  if( sensor.isCursorInView() )
  {
    Serial.print("Has Cursor in view: ");
    digitalWrite(LED_BUILTIN, HIGH);
    int cursor_x = sensor.getCursorX();
    int cursor_y = sensor.getCursorY();
    Serial.println("(X,Y) (" + String(cursor_x) + "," + String(cursor_y) + ")");
  } else {
    Serial.println("No Cursor in view");
    digitalWrite(LED_BUILTIN, LOW);
  }

  delay(100);
}
