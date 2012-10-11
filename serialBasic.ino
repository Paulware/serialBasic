#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include "ArduinoBASIC.h"

ArduinoBASIC arduinoBASIC;
DebugUtilities debugUtils;

void setup()
{  
  Serial.begin (115200);
  debugUtils.printPSTR ( PSTR ( "Arduino BASIC...enter ? to display keywords\nTo use the Arduino IDE\n select 'Carriage return' rather than 'no line ending' in the Serial Monitor\n" ));
}

void loop()
{
  if (Serial.available())
    arduinoBASIC.handleChar(Serial.read()); 
}
