#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoBASIC.h>
#include <MemoryFree.h>

ArduinoBASIC arduinoBASIC = ArduinoBASIC ();
DebugUtilities debugUtils;

void callback (int value)
{
  Serial.println ( "From callback" );
}


void setup()
{  
  Serial.begin (115200);
  debugUtils.printPSTR ( PSTR ( "Arduino BASIC...enter ? to display keywords\nTo use the Arduino IDE\n select 'Carriage return' rather than 'no line ending' in the Serial Monitor\n" ));
  debugUtils.printPSTR ( PSTR ( "free RAM: " ) );
  Serial.println ( freeMemory () );
  arduinoBASIC.init();
  arduinoBASIC.eepromProgram.callback = callback;
  if (arduinoBASIC.eepromProgram.numSteps() > 0) 
    arduinoBASIC.eepromProgram.run();
}

void loop()
{
  if (arduinoBASIC.eepromProgram.testState)
    arduinoBASIC.eepromProgram.continueTest();
  else if (Serial.available())
    arduinoBASIC.handleChar(Serial.read()); 
}

