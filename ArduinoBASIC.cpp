#include "ArduinoBASIC.h"
#define NUMBER_OF_COMMANDS 3

ArduinoBASIC::ArduinoBASIC():commands(NUMBER_OF_COMMANDS), debugUtils()
{
  commands.addString ( PSTR ( "?"));
  commands.addString ( PSTR ( "showSteps" ) ); 
  commands.addString ( PSTR ( "clear" ) ); 
  commands.addString ( PSTR ( "set") );

}
void ArduinoBASIC::handleChar ( char ch )
{
  int match = commands.matchCommand (ch);
  switch (match)
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
    case 3: // addStep
      //if (param = eepromTest.findCommand ( param , &cmd))  
      //  (void) eepromTest.addStep(cmd, param);
      //else
      //  Serial.println ( "Bad step" );  
      break;
    default:
      break;
  }
}
