#include <ArduinoBASIC.h>
#include <EEPROM.h>

ArduinoBASIC arduinoBASIC = ArduinoBASIC ();
DebugUtilities debugUtils;
unsigned long runTimeout;

#define NUMBER_OF_STEPS 10
PSTRStrings eProgram = PSTRStrings(NUMBER_OF_STEPS);
boolean paused;

void callback (int value)
{
  Serial.print ( "From callback value:" );
  Serial.println ( value );
}


void setup()
{  
  Serial.begin (115200);
  debugUtils.printPSTR ( PSTR ( "Arduino BASIC...enter ? to display keywords\nTo use the Arduino IDE\n select 'Carriage return' rather than 'no line ending' in the Serial Monitor\n" ));
  arduinoBASIC.init();
  arduinoBASIC.eepromProgram.callback = callback;
    
  eProgram.addString ( PSTR ( "setA=1" )); 
  eProgram.addString ( PSTR ( "ifA")); 
  eProgram.addString ( PSTR ( "echoA is set"));
  eProgram.addString ( PSTR ( "callback1"));
  eProgram.addString ( PSTR ( "jumplabelA" )); 
  eProgram.addString ( PSTR ( "endif")); 
  eProgram.addString ( PSTR ( "callback2"));
  eProgram.addString ( PSTR ( "echoA is not set"));
  eProgram.addString ( PSTR ( ":labelA" )); 
  eProgram.addString ( PSTR ( "echoAll done!"));

  runTimeout = millis() + 10000;
  Serial.println ( "Program will run in 10 seconds unless a key is pressed" );    
  paused = true;
}

void loop()
{
  if (runTimeout)
  {
    if (millis() > runTimeout)
    {
      arduinoBASIC.eepromProgram.run();
      runTimeout = 0;
      paused = false;
    }
  }
  
  if (!paused)  
    arduinoBASIC.eepromProgram.continueTest();
    
  if (Serial.available())
  {
    runTimeout = 0;
    paused = false;
    arduinoBASIC.handleChar(eProgram, Serial.read()); 
  }  
}

