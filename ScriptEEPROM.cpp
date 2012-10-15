#include <EEPROM.h>
#include <Wire.h>
#include "ScriptEEPROM.h"

ScriptEEPROM::ScriptEEPROM(PSTRStrings * _statements):components()
{ 
  statements = _statements;
  callback = 0;
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
    if (headEEPROM)
    {
      //debugUtils.printPSTR ( PSTR ( "headEEPROM initialized to: " ) );
      //Serial.println ( headEEPROM );
    }  
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
  reset();
  currentStep = 0;
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

// Convert ch to lower case
char ScriptEEPROM::lcase ( char ch )
{
  char c = ch;
  if ((c >= 'A') && (c <= 'Z')) 
    c += 'a' - 'A';
  return c;    
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

boolean ScriptEEPROM::addCh ( char ch, boolean incrementHead )
{
  if (headEEPROM < MAX_EEPROM_BUFFER-1)
  {
    EEPROM.write (headEEPROM, (uint8_t) ch); 
    if (incrementHead)
      headEEPROM++;
  } 
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
  uint8_t value = EEPROM.read (index);
  int step = 0;
  
  if (debugUtils.debugging())
  {
    if (!value) 
      debugUtils.printPSTR ( PSTR ( "Program is empty\n" ) );
    else
      // value at this point represents the next command
      while (value)
      {
        if (!step) // This is the first one
          debugUtils.printPSTR ( PSTR ( "\nScript Steps: \n" ) );
        step++;
        
        Serial.print ( step );
        debugUtils.printPSTR ( PSTR ( ":" ) );
        Serial.print ( index );        
        debugUtils.printPSTR ( PSTR ( ")" ) );
        statements->printString ( value );  
        debugUtils.printPSTR ( PSTR ( " " ) );
        
        // Echo the statement parameters
        index++; // Consume the statement number
        while (value)
        {
          value = EEPROM.read (index++);
          if (value)
            Serial.print ((char)value );
        }
        Serial.println();        
        // value is now zero, index will point to next command or 0 to terminate
        
        value = EEPROM.read (index); // value is now next command or 0 to terminate        
      }
      
  }  
}

void ScriptEEPROM::run( )
{
  debugUtils.printPSTR ( PSTR ( "Start the program" ) );
  testIndex = 0; // Start test over
  testState = 1;
  currentStep = 1;
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
  boolean debugThis = false;
  while (true)
  {
    value = EEPROM.read (*testPointer);
    // Exit if number if not a decimal
    if ((value < '0') || (value > '9'))
      break;
    if (!value)
      break;
    else
    {
      *testPointer += 1;
      if (debugThis)
      {
        debugUtils.printPSTR ( PSTR ( "old value: " ) );
        Serial.println ( total );
      }        
      total *= 10;
      total += value - '0';
      if (debugThis)
      {
        debugUtils.printPSTR ( PSTR ( "new value: " ) );
        Serial.println ( total );
      }        
    }    
  }
    
  return total;
}

void ScriptEEPROM::executeStep(int * step)
{
  static unsigned long waitTimeout = 0;
  static unsigned long startTime = 0;
  static unsigned long startTimer = 0;

  uint8_t value;
  uint8_t op;
  boolean done = true;
  int newStep;
  char varName;
  int * var;
  int index;
  int jumpOffset;
  unsigned long elapsedTime;
  static int totalTime;
  int compareValue;

  if (!testIndex) 
    startTime = millis();
 
  if (startTimer) // We are counting down
  {
    elapsedTime = (millis() - startTimer) / 1000;
    if ( elapsedTime > totalTime )
    {
      T = 0;
      startTimer = 0;
    }
    else
      T = totalTime - elapsedTime;
  }  
  
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
    
    case 2: // Statement = relaynnon or relaynnoff
      index = readDecimal (&testIndex);
      if ((EEPROM.read (testIndex) == 'o') && (EEPROM.read(testIndex+1) == 'n')) 
      {
        debugUtils.printPSTR ( PSTR ( "Relay " ) );
        Serial.print ( index );
        debugUtils.printPSTR ( PSTR ( " turned on\n" ) );
        digitalWrite ( index, 1);
        testIndex = testIndex + 2; // on 
      }  
      else
      {
        debugUtils.printPSTR ( PSTR ( "Relay " ) );
        Serial.print ( index );
        debugUtils.printPSTR ( PSTR ( " turned off\n" ) );
        digitalWrite (index, 0);
        testIndex = testIndex + 3; // off
      }  
      testIndex++; // Skip the command terminator
      
      // TODO energize the relay
      break;
     
    case 3: // Echo output text to serial monitor 
      while (true)
      {
        value = EEPROM.read ( testIndex++);
        if (!value)
          break;
        Serial.print ( (char) value );
      } 
      Serial.println();        
      break;
      
    case 4: // End If
      testIndex++; // Skip the 0 terminator
      break;
     
    case 5: // Evaluate the if statement
      varName = EEPROM.read ( testIndex++);
      var = 0;
      switch (varName)
      {
        case 'A':
          var = &A;
          break;
        case 'T':
          var = &T;
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
      if (EEPROM.read ( testIndex) == '<') // Evaluate a less than
      {
        testIndex++; // Consume the '<'
        compareValue = readDecimal (&testIndex);
        testIndex++;
        debugUtils.printPSTR ( PSTR ( "Check if var <" ) );
        Serial.println ( compareValue );
        if (*var < compareValue) // If Statement evaluates true
        {
        }
        else
        {
          // debugUtils.printPSTR ( PSTR ( "If statement evaluates false\n" ) );
          if (skipTo (4, testIndex ) )
            testIndex = skipTo (4, testIndex );
          else
          {
            debugUtils.printPSTR ( PSTR ( "Error...could not find an endif after " ) );
            Serial.println ( testIndex );  
          }  
        }
      }
      else if (EEPROM.read (testIndex) == '>') // Evaluate a greater than
      {
        testIndex++; // Consume the '>'
        compareValue = readDecimal (&testIndex);
        testIndex++;
        debugUtils.printPSTR ( PSTR ( "Check if var >" ) );
        Serial.println ( compareValue );
        if (*var > compareValue) // If Statement evaluates true
        {
        }
        else
        {
          // debugUtils.printPSTR ( PSTR ( "If statement evaluates false\n" ) );
          if (skipTo (4, testIndex ) )
            testIndex = skipTo (4, testIndex );
          else
          {
            debugUtils.printPSTR ( PSTR ( "Error...could not find an endif after " ) );
            Serial.println ( testIndex );  
          }  
        }
      }
      else if (var)
      {
        testIndex++; // Skip null terminator of if statement 
        if (*var) // If statement evaluates true
        {
          // debugUtils.printPSTR ( PSTR ( "If statement evaluated true\n" ) );
        }  
        else
        {
          // debugUtils.printPSTR ( PSTR ( "If statement evaluates false\n" ) );
          if (skipTo (4, testIndex ) )
            testIndex = skipTo (4, testIndex );
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
      
    case 6: // Jump to step  
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
        *step = readDecimal (&index); 
      }  
      testIndex++; // Consume the terminating 0      
      newStep = *step + jumpOffset;
      done = false; // New step will not be completed until later

      //debugUtils.printPSTR ( PSTR ("Jumping from CurrentStep: " ));
      //Serial.print (*step);
      //debugUtils.printPSTR ( PSTR ( " to Next Step: " ));
      //Serial.print ( newStep); 
      testIndex = findStep ( newStep);
      currentCommand = EEPROM.read ( testIndex );
      //debugUtils.printPSTR ( PSTR ( " " ) );
      //statements->printString (currentCommand);
      //debugUtils.printPSTR ( PSTR ( "\n" ) );
      *step = newStep;
      break;
            
    case 7: // output text to lcd
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
          
    case 8: // wire NN Read wire slave device nn, it will be appended to log file
      debugUtils.printPSTR ( PSTR ( "Read Wire readDecimal\n" ) );
      testIndex++; // Skip the 0 terminator
      break;
      
    case 9: // Set A {+,-,=} Number
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
      
    case 10: // callback (for those wanting to do a c call)
      if (callback)
        callback();
      testIndex++; // Consume the terminating 0
      break;

    case 11: // thermistor, set value in A
      // Get the Analog Input number 
      A = (int) components.thermistorFahrenheit (readDecimal (&testIndex));
      testIndex++; // Consume the terminating 0
      break;      

    case 12: // startTimer
      startTimer = millis();
      totalTime = readDecimal ( &testIndex ); 
      if (debugUtils.debugging())
      {
        debugUtils.printPSTR ( PSTR("Got a totaltime of : " ));
        Serial.println ( totalTime );
      }
      T = totalTime;
      testIndex++; // Consume the terminating 0    
      break;  
      
    /*       
    // Read from slave 2, and write to external eeprom
    case 20: // processWire
      processWire();
      break; 
    */      
      
    default:
      done = false;
    break;
  } 

  if (done)
    *step = *step + 1;  
  
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

void ScriptEEPROM::dump()
{
  int index = 0;
  boolean first = false;
  uint8_t value = EEPROM.read(index);
  
  if (!value) // Program is not-empty
    debugUtils.printPSTR ( PSTR ( "Program is empty" ) );
  else
  {
    debugUtils.printPSTR ( PSTR ( "***Statement***\n" ) );
    while (value || EEPROM.read(index+1))
    {
      Serial.print ( index++ );
      debugUtils.printPSTR ( PSTR(")") );
      Serial.println ( value );
      if (first) 
        debugUtils.printPSTR ( PSTR ( "***Statement***\n" ) );
      first = false;  

      value = EEPROM.read (index);
      if (!value)
        first = true;
    }
    Serial.print ( index );
    debugUtils.printPSTR ( PSTR(")0\n") );
    Serial.print ( index+1 );
    debugUtils.printPSTR ( PSTR(")***END OF PROGRAM***\n") );
    
  }   
}

// This call will do nothing unless the test has been started
// and the test script has been initialized
void ScriptEEPROM::continueTest()
{
  static int lastCommand = -1;
  static int lastStep = 0;
  boolean debugThis = false;
    
  if (currentStep != lastStep)
  { 
    currentCommand = EEPROM.read ( testIndex);
    if (debugThis)
    {
      debugUtils.printPSTR ( PSTR ( "Just read a " ));
      statements->printString (currentCommand);
      debugUtils.printPSTR( PSTR ( " from: " ) );
      Serial.println (testIndex);
    }  
    testIndex++;  
    lastStep = currentStep;
  }  
  
  if ((lastCommand != currentCommand ) && (currentCommand == 0))
  {
    debugUtils.printPSTR ( PSTR ( "Test completed successfully\n" ) );
    init();
  }  
  else
  {
    lastCommand = currentCommand;
    executeStep (&currentStep);
  }  
}
