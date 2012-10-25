#include <Arduino.h>
#include <Wire.h>
#include <EEPROM.h>
#include <ArduinoBASIC.h>
#include <MemoryFree.h>

ArduinoBASIC arduinoBASIC = ArduinoBASIC ();
DebugUtilities debugUtils;

#define NUMBER_OF_STEPS 34
PSTRStrings eProgram = PSTRStrings(NUMBER_OF_STEPS);

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
    
  eProgram.addString ( PSTR ( "setA=1" )); // Add to an internal list of strings
  eProgram.addString ( PSTR ( ":label2" )); // Add to an internal list of strings
  eProgram.addString ( PSTR ( "ifA")); // internal list of strings
  eProgram.addString ( PSTR ( "echoA is set")); // internal list of strings
  eProgram.addString ( PSTR ( "jumplabel1" )); // Add to an internal list of strings
  eProgram.addString ( PSTR ( "endif")); // internal list of strings
  eProgram.addString ( PSTR ( "jumplabel2" )); // Add to an internal list of strings
  eProgram.addString ( PSTR ( ":label1" )); // Add to an internal list of strings
  eProgram.addString ( PSTR ( "echoAll done!"));
    
}

void loop()
{
  if (arduinoBASIC.eepromProgram.testState)
    arduinoBASIC.eepromProgram.continueTest();
  else if (Serial.available())
    arduinoBASIC.handleChar(eProgram, Serial.read()); 
}

