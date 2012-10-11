#include "ArduinoBASIC.h"
#define NUMBER_OF_COMMANDS 4
#define NUMBER_OF_STATEMENTS 4

ArduinoBASIC::ArduinoBASIC():commands(NUMBER_OF_COMMANDS), statements(NUMBER_OF_STATEMENTS), debugUtils()
{
  commands.addString ( PSTR ( "?"));           
  commands.addString ( PSTR ( "showSteps" ) );
  commands.addString ( PSTR ( "clear" ) );  
  commands.addString ( PSTR ( "help" ) );
  
  statements.addString ( PSTR ( "set") );
  statements.addString ( PSTR ( "jump" ));
  statements.addString ( PSTR ( "if" ) );
  statements.addString ( PSTR ( "endif" ) );
}

void ArduinoBASIC::handleChar ( char ch )
{
  int gotCommand = commands.matchCommand (ch);
  int gotStatement = statements.matchCommand (ch);
  static int lastStatement = 0;
  
  if (gotCommand > -1)
    debugUtils.printPSTR ( PSTR ( "\n" ) );
    
  switch (gotCommand)
  {
    case 0:
      debugUtils.printPSTR ( PSTR ( "Recognized commands: \n" ));
      for (int i=1; i<NUMBER_OF_COMMANDS; i++)
      {
        Serial.print ( i );
        debugUtils.printPSTR ( PSTR ( ")" ) );
        commands.printString(i);
        debugUtils.printPSTR ( PSTR ( "\n" ) );
      }
      break;
    case 1:
      eepromProgram.showSteps();
      break;
    case 2:
      eepromProgram.clear();
      break;
    case 3:
      debugUtils.printPSTR ( PSTR ( "Recognized statements: \n" ));
      for (int i=0; i<NUMBER_OF_STATEMENTS; i++)
      {
        Serial.print ( i );
        debugUtils.printPSTR ( PSTR ( ")" ) );
        statements.printString(i);
        debugUtils.printPSTR ( PSTR ( "\n" ) );
      }
      break;
  
    default:
      break;
  }
  
  if (lastStatement > -1) // Saving data to eeprom
  {
    if (ch == 10) 
      debugUtils.printPSTR ( PSTR ( "done") );
    lastStatement = -1;
  }
  else if (gotStatement > -1)
  { 
    switch(gotStatement)
    {
      case 3: // addStep
        //if (param = eepromTest.findCommand ( param , &cmd))  
        //  (void) eepromTest.addStep(cmd, param);
        //else
        //  Serial.println ( "Bad step" );  
        break;
      default:
        break;
    }
    lastStatement = gotStatement;
  }
}
