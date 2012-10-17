#include "ArduinoBASIC.h"
#define NUMBER_OF_COMMANDS 6
#define NUMBER_OF_STATEMENTS 13

  
ArduinoBASIC::ArduinoBASIC(): 
  commands(NUMBER_OF_COMMANDS), 
  statements(NUMBER_OF_STATEMENTS), 
  eepromProgram (&statements),
  debugUtils()
{
  lastStatement = -1;
  eepromProgram.callback = 0;
  
  commands.addString ( PSTR ( "?"));            //  0
  commands.addString ( PSTR ( "list" ));        //  1
  commands.addString ( PSTR ( "clear" ));       //  2
  commands.addString ( PSTR ( "dump" ));        //  3
  commands.addString ( PSTR ( "run" ));         //  4
  commands.addString ( PSTR ( "del"));          //  5
  
  statements.addString ( PSTR ( "endtest" ));   //  0
  statements.addString ( PSTR ( "wait" ));      //  1
  statements.addString ( PSTR ( "relay" ));     //  2
  statements.addString ( PSTR ( "echo" ));      //  3
  statements.addString ( PSTR ( "endif" ));     //  4
  statements.addString ( PSTR ( "if" ) );       //  5
  statements.addString ( PSTR ( "jump" ));      //  6
  statements.addString ( PSTR ( "lcd" ));       //  7
  statements.addString ( PSTR ( "read" ));      //  8   
  statements.addString ( PSTR ( "set"));        //  9 
  statements.addString ( PSTR ( "callback"));   // 10
  statements.addString ( PSTR ( "thermistor")); // 11
  statements.addString ( PSTR ( "startTimer")); // 12
}

void ArduinoBASIC::init ()
{
  // callback = _callback;
  eepromProgram.init();
}

void ArduinoBASIC::continueStatement ( char ch )
{
  if (ch == 13)
  {
    eepromProgram.addCh (0, true);
    eepromProgram.addCh (0, false);
  }  
  else  
    eepromProgram.addCh (ch, true);
}

void ArduinoBASIC::handleChar ( char ch )
{
  int gotStatement = statements.matchCommand (ch, false);
  int gotCommand = commands.matchCommand (ch, false);
  char c;
  int value;
  
  //statements.show(0,12);
  //commands.show(0,4);
      
  if (lastStatement > -1)
  { // Save uninterpretted parameters to EEPROM
    continueStatement (ch); 
    if (ch == 13) 
      lastStatement = -1; // done
  }
  else if (gotStatement > -1)
  { 
    // statements.printString ( gotStatement );
    eepromProgram.addCh (gotStatement, true);
    lastStatement = gotStatement;
  }
  else // Check for command
  {
      
    if (gotCommand > -1) 
    {    
      commands.printString ( gotCommand ); 
      debugUtils.printPSTR ( PSTR ( "\n" ) );
    }
    switch (gotCommand)
    {
      case 0:
        debugUtils.printPSTR ( PSTR ( "Commands: \n" ));
        commands.show(1,NUMBER_OF_COMMANDS);
        debugUtils.printPSTR ( PSTR ( "BASIC-like statements: \n" ));
        statements.show(1,NUMBER_OF_STATEMENTS);
        break;
      case 1:
        eepromProgram.showSteps();
        break;
      case 2:
        eepromProgram.clear();
        break;
      case 3:
        eepromProgram.dump();
        break;
      case 4: 
        eepromProgram.run();
        break;    
      case 5: 
        value = eepromProgram.readDec ( &c);
        eepromProgram.del(value);
        break;
      /*
      case 6: 
        value = eepromProgram.readDec ( &c);
        eepromProgram.change(value, c);
        break;        
      */  
      default:
        break;
    }
  }
}
