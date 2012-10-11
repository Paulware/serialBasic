#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "ArduinoBASIC.h"

ArduinoBASIC arduinoBASIC;
DebugUtilities debugUtils;

void setup()
{  
  Serial.begin (115200);
  Serial.println ( "Arduino BASIC, enter ? to display commands" );
}

void loop()
{
  if (Serial.available())
    arduinoBASIC.handleChar(Serial.read()); 
}
