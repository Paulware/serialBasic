#include <EEPROM.h>
#include <Wire.h>
#include "ScriptEEPROM.h"

#define BLOCK_SIZE 128

#define NUM_COMMANDS 21
prog_char  cmd0[] PROGMEM = "end";
prog_char  cmd1[] PROGMEM = "wait";
prog_char  cmd2[] PROGMEM = "setFail";
prog_char  cmd3[] PROGMEM = "setMin";
prog_char  cmd4[] PROGMEM = "setMax";
prog_char  cmd5[] PROGMEM = "setTest";
prog_char  cmd6[] PROGMEM = "setCase";
prog_char  cmd7[] PROGMEM = "relay";
prog_char  cmd8[] PROGMEM = "debug";
prog_char  cmd9[] PROGMEM = "sendCmd";
prog_char cmd10[] PROGMEM = "waitFor";
prog_char cmd11[] PROGMEM = "setName";
prog_char cmd12[] PROGMEM = "if";
prog_char cmd13[] PROGMEM = "jump";
prog_char cmd14[] PROGMEM = "endif";
prog_char cmd15[] PROGMEM = "lcd";
prog_char cmd16[] PROGMEM = "dump";
prog_char cmd17[] PROGMEM = "wire";
prog_char cmd18[] PROGMEM = "set";
prog_char cmd19[] PROGMEM = "startTimer";
prog_char cmd20[] PROGMEM = "processWire";

PROGMEM const char *commands[NUM_COMMANDS] = { cmd0,  cmd1,  cmd2,  cmd3,  cmd4,  cmd5,  cmd6,  cmd7, 
                                               cmd8,  cmd9, cmd10, cmd11, cmd12, cmd13, cmd14, cmd15,
                                              cmd16, cmd17, cmd18, cmd19, cmd20 };
                                              
ScriptEEPROM::ScriptEEPROM()
{ 
}


// This function stores the data to be passed to the user
// This procedure is necessary because calling s.ES_fill_tcp_data_len
// one character at a time is too slow
void ScriptEEPROM::processWire ()
{
  #define MAX_MTG_LINE 130
  char line[MAX_MTG_LINE];
  char ch = ' ';
  int count = 0;
  
  while ((count < MAX_MTG_LINE - 2) && (ch))
  {
    ch = 0;
    Wire.requestFrom(2, 1);     // request 1 byte from slave device #2
    if(Wire.available())
    {
      ch = Wire.read(); // receive a byte as character
      if (ch)
      {
        // Convert carriage return to &#13;
        if (ch == 13) 
        {
          if (count < MAX_MTG_LINE - 6) 
          {
            line[count++] = '&';
            line[count++] = '#';
            line[count++] = '1';
            line[count++] = '3';
            line[count++] = ';';
            //(void)caseComplete.textFound(ch);
          }  
          else // TODO: Put the carriage return back to be processed later
          {            
          }
        }  
        // Convert " to &#34;
        else if (ch == '"')
        {
          if (count < MAX_MTG_LINE - 6) 
          {
            line[count++] = '&';
            line[count++] = '#';
            line[count++] = '3';
            line[count++] = '4';
            line[count++] = ';';
            //(void)caseComplete.textFound(ch);
          }
          else // TODO : Put the ch back to be processed later
          {
          }
        }
        // Ignore line feeds to tighten up output
        else if (ch != 10)
        {
          line[count++] = ch;
          //(void)caseComplete.textFound(ch);
          //(void)testComplete.textFound(ch);
          //(void)failureText.textFound(ch);
        }  
      }  
    }    
  }
  line[count] = 0; // Terminate the string
}    


                                
char * ScriptEEPROM::findCommand (char * param, int8_t  * cmd)
{
  char * addStep;
  prog_char * pstrPtr;
  *cmd = 0;
  for (int i=1; i<NUM_COMMANDS; i++)
  {
    // Get the pointer to the command 
    pstrPtr = (prog_char *)pgm_read_word(&(commands[i]));
    
    if (addStep = matchPSTR ( param, pstrPtr ) )
    {
      *cmd = i;
      break;
    }
  } 
  
  if (!*cmd)
  {
    debugUtils.printPSTR ( PSTR ( "Could not find a step matching:" ));
    Serial.println ( param );
  }  

  return addStep;
}  

// Convert ch to lower case
char ScriptEEPROM::lcase ( char ch )
{
  char c = ch;
  if ((c >= 'A') && (c <= 'Z')) 
    c += 'a' - 'A';
  return c;    
}

// Return the address of the end of the match (pointing to the null terminator)
// Command must be a null terminated string located in program memory (Flash)
char * ScriptEEPROM::matchPSTR (char * param, const prog_char * command)
{  
  char * endMatch = 0;
  boolean match = true;
  char ch1;
  char ch2 = pgm_read_byte (command);
  
  while (match && ch2)
  {
     ch1 = *param++;
     
     if (lcase(ch1) != lcase(ch2))
       match = false;
       
     command ++;
     ch2 = pgm_read_byte(command);
  }
  if (match)
    endMatch = param;
  
  return endMatch;
}

void ScriptEEPROM::reset()
{
  int value;
  
  testNumber = 0;
  headEEPROM = 0;
  
  value = EEPROM.read ( headEEPROM );
  while (value) // Read all steps
  {
    while (value = EEPROM.read ( headEEPROM++) ) // Read one step
    { 
    }  
    value = EEPROM.read ( headEEPROM);  
  }  
  
  if (debugUtils.debugging()) 
  {
    debugUtils.printPSTR ( PSTR ( "headEEPROM initialized to: " ) );
    Serial.println ( headEEPROM );
  }
  
  testIndex = 0;
  testState = 0;
  currentCommand = 0;  
}

void ScriptEEPROM::clear()
{
  debugUtils.printPSTR ( PSTR ( "EEPROM cleared\n" ));
  testIndex = 0;
  EEPROM.write (0,0);
  headEEPROM = 0;
}

void ScriptEEPROM::init()
{
  debugUtils.printPSTR ( PSTR ( "ScriptEEPROM instantiation\n" ) );
  reset();
  currentStep = 0;
}


char * ScriptEEPROM::getCh ( char * parameter, char * ch )
{
  char * param = parameter;
  int value;
  char c;
  
  *ch = *param;
  param++;
  if (*ch == '%') // Read %20 for example
  {
    param = readHexFromChar (param, &value);
    c = (char) value;
    *ch = c;
  }
  return param;  
}

/* 
  Parameter will be in this format: 
    nnparameter 
  where nn is some number 00..99 and 
  parameter is the rest of the command
  
  There is no "smart" processing of the command to optimize eeprom
*/
boolean ScriptEEPROM::addStep ( uint8_t command, char * parameter )
{
  boolean ok = false;
  int len = strlen ( parameter );
  int head = headEEPROM;
  char ch;
  
  if (headEEPROM < MAX_EEPROM_BUFFER-1)
  {
    EEPROM.write (headEEPROM++, command); 
    while (*parameter != ' ')
    {
      if (headEEPROM == MAX_EEPROM_BUFFER-1)
        break;
      else
      {
        parameter = getCh ( parameter, &ch );
        EEPROM.write (headEEPROM++, ch );
      }  
    }
    // if have already filled the buffer, back out the change
    if (headEEPROM == MAX_EEPROM_BUFFER-1)
    {
      headEEPROM = head; // Restore to old value
      EEPROM.write (headEEPROM, 0); // Restore end of test
    }  
    else // We have enough room for test Termination
    {
      EEPROM.write ( headEEPROM++, 0); // Command termination
      EEPROM.write ( headEEPROM, 0); // Test termination  
      ok = true;
    }
  }

  if (debugUtils.debugging() && ok)
  {  
    debugUtils.printPSTR ( PSTR ( "Command" ) );
    Serial.print ( command );
    debugUtils.printPSTR ( PSTR ( " added headEEPROM:" ) );
    Serial.println ( headEEPROM );
  }
  return ok;  
}

  
PROGMEM const char * ScriptEEPROM::decodeCommand (uint8_t command)
{
  prog_char * index = (prog_char *)pgm_read_word(&(commands[command]));
  if ((command <0) || (command > NUM_COMMANDS))
    index = 0;
  
  return index;
}

int ScriptEEPROM::showDecimal (int index)
{
  uint8_t value = EEPROM.read ( index++);
  debugUtils.printPSTR ( PSTR( "show decimal" ));
  while (value)
  {
    Serial.print ( (char)value );
    value = EEPROM.read(index++);
  }
  return index;
}

int ScriptEEPROM::indexToStep ( int targetIndex )
{
  int index = 0;
  uint8_t value = EEPROM.read (index);
  int stepCount = 0;
  
  while (value)
  {
    stepCount++;
    if (targetIndex == index)
      break;
      
    index++;      
    while (value)
      value = EEPROM.read (index++);
      
    // skip the end of command zero terminator  
    value = EEPROM.read (index);    
  }
   
  if (!value) // We are at the end of the program
    stepCount++; // virtual step
    
  if (targetIndex != index)
  {
    if (debugUtils.debugging())
    {
      debugUtils.printPSTR ( PSTR ( "Could not find index: " ) );
      Serial.println ( targetIndex );
    }
    stepCount = 0;
  }  
    
  return stepCount;
}

int ScriptEEPROM::findStep ( int stepNum ) 
{
  int index = 0;
  uint8_t value = EEPROM.read (index++);
  int stepCount = 0;
  
  while (value)
  {
    stepCount++;
    if (stepCount==stepNum)
      break;
      
    while (value)
      value = EEPROM.read (index++);
      
    // skip the end of command zero terminator  
    value = EEPROM.read (index++);    
  }
  index = index - 1; // Point to last processed data
    
  if (stepCount != stepNum)
  {
    if (debugUtils.debugging())
    {
      debugUtils.printPSTR ( PSTR ( "Could not find step: " ) );
      Serial.println ( stepNum );;
    }
    index = 0;
  }  
    
  return index;
}

void ScriptEEPROM::showSteps ()
{
  int index = 0;
  uint8_t value = EEPROM.read (index++);
  int step = 0;
  
  if (debugUtils.debugging())
  {
    while (value)
    {
      if (!step) // This is the first one
        debugUtils.printPSTR ( PSTR ( "\nScript Steps: \n" ) );
      step++;
      if (step < 10) 
        debugUtils.printPSTR ( PSTR ( " " ));
        
      Serial.print ( step );
      debugUtils.printPSTR ( PSTR ( ":" ) );
      Serial.print ( index - 1);
      debugUtils.printPSTR ( PSTR ( ")" ) );
      debugUtils.printPSTR ( decodeCommand ( value ) );  
      debugUtils.printPSTR ( PSTR ( " " ) );
      while (value)
      {
        value = EEPROM.read (index++);
        if (value)
          Serial.print ( (char)value );
      }
      Serial.println();
      // skip the end of command zero terminator  
      value = EEPROM.read (index++);  
    }
    if (!step)
      debugUtils.printPSTR ( PSTR("Program is empty\r\n") );
  }  
}

// Return an EEPROM pointer to the specified command
int ScriptEEPROM::skipTo (uint8_t cmd, int index )
{
  uint8_t value = EEPROM.read (index); // Read the first command 
  int steps = 0;
  if (!value)
    debugUtils.printPSTR ( PSTR ( "value is already 0 at start of skipTo\n" ));
    
  while ((value != cmd) && value)
  {
    // skip to end of command
    while (value)
      value = EEPROM.read (index++);
    // value is now zero indicated the end of the test step

    // Read the next command
    value = EEPROM.read (index);
    steps++;
  }
  if (value != cmd)
    index = 0;
  return index;  
}

// Count the number of 0s in the eeprom, stop on double zero
// Not sure if this is necessary
int ScriptEEPROM::numSteps ( )
{
  int index = 0;
  uint8_t value = EEPROM.read (index++);
  int steps = 0;
  while (value)
  {
    // skip to end of command
    while (value)
      value = EEPROM.read (index++);
    // skip the end of command zero terminator
    value = EEPROM.read (index++);
    steps++;
  }
  return steps;  
}

char * ScriptEEPROM::readHexFromChar (char * hex, int * total)
{
  *total = 0;
  char ch;
  
  ch = *hex;
  if ((ch >= 'A') && (ch <= 'E')) 
    *total += ch + 10 - 'A';
  else if ((ch >= '0') && (ch <= '9')) 
    *total += ch - '0';
  hex++;
  
  ch = *hex;
  if ((ch >= 'A') && (ch <= 'E')) 
    *total += ch + 10 - 'A';
  else if ((ch >= '0') && (ch <= '9')) 
    *total += ch - '0';
  hex++;

  return hex;
}
    
// Do not consume the terminating 0
int ScriptEEPROM::readDecimal (int * testPointer)
{
  int value = 0;
  int total = 0;
  while (true)
  {
    value = EEPROM.read (*testPointer);
    if (!value)
      break;
    else
    {
      *testPointer += 1;
      total *= 10;
      total += value - '0';
    }    
  }
    
  return total;
}

void ScriptEEPROM::executeStep()
{
  static unsigned long waitTimeout = 0;
  uint8_t value;
  uint8_t op;
  boolean done = true;
  int newStep;
  char varName;
  int * var;
  int index;
  int jumpOffset;
  unsigned long elapsedTime;

  if (!testIndex) 
    startTime = millis();
  
  switch (currentCommand)
  {
    case 0: // end Test
      debugUtils.printPSTR ( PSTR ("Test is ended\n" ));
      break;
      
    case 1: // Wait X seconds (1 byte)
      if (waitTimeout)
      {
        if (millis() > waitTimeout) 
        {
          waitTimeout = 0;
          debugUtils.printPSTR ( PSTR ( "Wait is complete\n" ) );
        }
        else
          done = false;  
      }
      else // read variable number of seconds
      {
        waitTimeout = readDecimal ( &testIndex );
        testIndex++; // Consume the terminating 0
        if (debugUtils.debugging())
        {
          debugUtils.printPSTR ( PSTR ( "Wait for " ) );
          Serial.print ( waitTimeout );
          debugUtils.printPSTR ( PSTR ( " seconds\n" ) );
        }
        waitTimeout *= 1000; // Convert to milliseconds        
        waitTimeout += millis(); // Offset from current time
        
        done = false;
      }      
    break;
    
    case 7: // Set relay n = relay number, n=(1:on, 0:off)
      testIndex++; // Skip the relay number
      testIndex++; // Skip the oneOff
      testIndex++; // Skip the command terminator
      // TODO energize the relay
      debugUtils.printPSTR ( PSTR ( "Set relay X on/off\n" ) );
      break;
     
    case 8: // Debug output text to serial monitor 
      while (true)
      {
        value = EEPROM.read ( testIndex++);
        if (!value)
          break;
        Serial.print ( (char) value );
      } 
      Serial.println();        
      break;
     
    case 12: // Evaluate the if statement
      varName = EEPROM.read ( testIndex++);
      var = 0;
      switch (varName)
      {
        case 'A':
          var = &A;
          break;
        case 'E':
          // Compute elapsed time
          elapsedTime = (millis() - startTime) / 1000;
          E = totalTime - elapsedTime;
          if (E < 0)
            E = 0;
          var = &E;
          debugUtils.printPSTR ( PSTR ( "E=" ));
          Serial.println ( E);
          break;  
        default:
          debugUtils.printPSTR ( PSTR ( "***ERROR*** Variable not recognized: " ));
          Serial.println ( (int)varName );
          break;
      }
      if (var)
      {
        testIndex++; // Skip null terminator of if statement 
        if (*var) // If statement evaluates true
          debugUtils.printPSTR ( PSTR ( "If statement evaluated true\n" ) );
        else
        {
          debugUtils.printPSTR ( PSTR ( "If statement evaluates false\n" ) );
          if (skipTo (14, testIndex ) )
            testIndex = skipTo (14, testIndex );
          else
          {
            debugUtils.printPSTR ( PSTR ( "Error...could not find an endif after " ) );
            Serial.println ( testIndex );  
          }  
        }  
      }  
      else
      {
        debugUtils.printPSTR ( PSTR ( "Could not find varname in if statement" ) );
        testIndex++; // Skip null terminator of if statement 
      }
      break;  
      
    case 13: // Jump to step  
      // Handled Below; 
      value = EEPROM.read ( testIndex );
      index = testIndex;
      if (value == '-') // Jump relative
      {
        index ++; // skip '-'
        jumpOffset = 0 - readDecimal (&index);
      }
      else if (value == '+') // Jump relative
      {
        index++; // skip '+'
        jumpOffset = readDecimal ( &index );
      }
      else // Jump absolute
      {        
        jumpOffset = 0;
        currentStep = readDecimal (&index); 
        Serial.println ( currentStep );
      }  
      testIndex++; // Consume the terminating 0      
      newStep = currentStep + jumpOffset; // Plus 1 because jump is complete
      done = false; // New step will not be completed until later

      debugUtils.printPSTR ( PSTR ("Jumping from CurrentStep: " ));
      Serial.print ( currentStep);
      debugUtils.printPSTR ( PSTR ( " to Next Step: " ));
      Serial.print ( newStep); 
      testIndex = findStep ( newStep);
      currentCommand = EEPROM.read ( testIndex );
      debugUtils.printPSTR ( PSTR ( " " ) );
      debugUtils.printPSTR ( decodeCommand (currentCommand) );
      debugUtils.printPSTR ( PSTR ( "\n" ) );
      break;
      
    case 14: // End If
      testIndex++; // Skip the 0 terminator
      break;
      
    case 15: // output text to lcd
      if (debugUtils.debugging())
      {
        debugUtils.printPSTR ( PSTR ( "Output: " ) );
        while (true)
        {
          value = EEPROM.read ( testIndex++);
          if (!value)
            break;
          Serial.print ( (char) value );
        }
        debugUtils.printPSTR ( PSTR ( " to LCD display\n" ) );
      }  
      break;
    
    case 16: // dump EEPROM
      debugUtils.printPSTR ( PSTR ( "Dump EEPROM device\n" ) );
      testIndex++; // Skip the 0 terminator
      break;
      
    case 17: // wire NN Read wire slave device nn, it will be appended to log file
      debugUtils.printPSTR ( PSTR ( "Read Wire readDecimal\n" ) );
      testIndex++; // Skip the 0 terminator
      break;
      
    case 18: // Set A {+,-,=} Number
      varName = EEPROM.read ( testIndex++);
      op = EEPROM.read ( testIndex++);
      var = 0;
      switch (varName)
      {
        case 'A':
          var = &A;
          break;
        default:
          debugUtils.printPSTR ( PSTR ( "***ERROR*** Variable not recognized: " ));
          Serial.println ( varName );
          break;
      }
      if (var)
      {
        switch (op)
        {
          case '=': 
            *var = readDecimal ( &testIndex );
            break;
          case '-':
            *var = *var - readDecimal ( &testIndex );
            break;
          case '+':
            *var = *var + readDecimal ( &testIndex );
            break;
          default:
            debugUtils.printPSTR ( PSTR ( "operator not understood: " ) );
            Serial.println ( (char ) op );
            break;
        }
        testIndex++; // Consume the terminating 0
      }
      debugUtils.printPSTR ( PSTR ( "Set command complete\n" ) );
      break;
      
    case 19: // startTimer
      startTime = millis();
      totalTime = readDecimal ( &testIndex ); 
      testIndex++; // Consume the terminating 0    
      if (debugUtils.debugging())
      {
        debugUtils.printPSTR ( PSTR("Got a totaltime of : " ));
        Serial.println ( totalTime );
      }  
      break;  
      
    // Read from slave 2, and write to external eeprom
    case 20: // processWire
      processWire();
      break;  
      
    default:
      done = false;
    break;
  }  
  
}

const prog_char * ScriptEEPROM::testStatus ()
{
  const prog_char * progmem;
  switch (testState)
  {
    case 0:
      progmem = PSTR ( "Idle" );
      break;
    case 1:
      progmem = PSTR ( "Running" );
      break;
    case 2:
      progmem = PSTR ( "Completed Successfully" );
      break;
    case 3:
      progmem = PSTR ( "Test Failure Detected!");
      break;
    default:
      progmem = PSTR ( "Unknown test state" );
      break;
  }
  return progmem;
}

void ScriptEEPROM::dumpEEPROM()
{
  int index = 0;
  uint8_t value = EEPROM.read(index);
  
  while (value || EEPROM.read(index+1))
  {
    Serial.print ( index++ );
    debugUtils.printPSTR ( PSTR(")") );
    Serial.println ( value );
    value = EEPROM.read (index);
  }
}

// This call will do nothing unless the test has been started
// and the test script has been initialized
void ScriptEEPROM::continueTest()
{
  static int lastTestState = 0;
  static int lastCommand = -1;
  
  if ((testState == 1) && (lastTestState != 1))
  {
    testIndex = 0; // Start test over
    currentStep = 1;
    debugUtils.printPSTR( PSTR ( "Starting test\n" ) );
  }
  
  if (lastCommand)
  { 
    currentCommand = EEPROM.read ( testIndex);
    debugUtils.printPSTR ( PSTR ( "Just read a " ));
    Serial.print ( currentCommand );
    debugUtils.printPSTR( PSTR ( " command from index: " ) );
    Serial.println ( testIndex );
  }  
  
  if ((lastCommand != currentCommand ) && (currentCommand == 0))
      debugUtils.printPSTR ( PSTR ( "Test completed successfully\n" ) );
  
  lastCommand = currentCommand;
  lastTestState = testState;
  
  if (!currentCommand)
    testState = 2; // Test complete
  else
  {
    testIndex++; // Command consumed
    executeStep ();
  }    
    
    
}
