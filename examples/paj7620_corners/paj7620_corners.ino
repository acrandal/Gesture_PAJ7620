/*
  Example Script: paj7620_corners.ino
  Package: RevEng_PAJ7620

  Copyright (c) 2021 Aaron S. Crandall
  Author       : Aaron S. Crandall <crandall@gonzaga.edu>
  Modified Time: February 2021

  Description: This demo demonstrates the ability to return when an object in view is in various quadrants.
    NW | NE
    --MID--
    NW | SE

  License: Same as package under MIT License (MIT)
*/

// Includes enum definition of CORNER_* return values from readGesture()
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
  Serial.println("Demo of Corner Interface Features");

  delay(2000);
}


// ******************************************************************
void loop()
{
  Corner curr_corner;
  curr_corner = sensor.getCorner();

  switch(curr_corner)
  {
    case CORNER_NONE:
      Serial.println("No corner");
      break;
    case CORNER_NE:
      Serial.println("NE corner");
      break; 
    case CORNER_NW:
      Serial.println("NW corner");
      break; 
    case CORNER_SE:
      Serial.println("SE corner");
      break; 
    case CORNER_SW:
      Serial.println("SW corner");
      break; 
    case CORNER_MIDDLE:
      Serial.println("Middle area");
      break;
  }

  delay(100);
}
