#include <EEPROM.h>
#include <Wire.h>
#include "ScriptEEPROM.h"

// Set the maximum number of EEPROM bytes for atmega328
#define MAX_EEPROM 1024

uint8_t ScriptEEPROM::readEEPROM ( int index )
{
  uint8_t value = 0;
  if ((index <= MAX_EEPROM) && (index >= 0))
    value = EEPROM.read (index);
  return value;
}

ScriptEEPROM::ScriptEEPROM(PSTRStrings * _statements, PSTRStrings * _commands):components()
{ 
  statements = _statements;
  commands = _commands;
  callback = 0;
  // Read test state from eeprom 
  testState = EEPROM.read (0);
  paused = false;
}

// Called before starting a test over?
void ScriptEEPROM::reset()
{
  int value;
  
  headEEPROM = 1;
  waitTimeout = 0;
  
  while (value && (headEEPROM < 1024)) // Read all steps
  {
    while ((headEEPROM < 1024) && (value = readEEPROM ( headEEPROM++))) // Read one step
    { 
    }  
    value = readEEPROM (headEEPROM);  
  }  
  
  if (debugUtils.debugging()) 
  {
    if (headEEPROM)
    {
      debugUtils.printPSTR ( PSTR ( "Program byte length = " ) );
      Serial.println ( headEEPROM );
    }  
  }
  
  if (headEEPROM == 1024)
  {
    debugUtils.printPSTR ( PSTR ( "\nEEPROM corruption detected...clearing\n" ) );\
	clear();
  }  
  
  testIndex = 1;
  writeTestState (0);
  currentCommand = 0; 
  // debugUtils.printPSTR ( PSTR( "reset cc" ) );  
  T = 0;
  E = 0;
  A = 0;
  M = 0;
}

void ScriptEEPROM::clear()
{
  debugUtils.printPSTR ( PSTR ( "EEPROM cleared\n" ));
  testIndex = 1;
  EEPROM.write (0,0); // test Status = idle
  // eprogram = null
  EEPROM.write (1,0);
  EEPROM.write (2,0);   
  headEEPROM = 1;
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

// Add a character to the eeprom 
boolean ScriptEEPROM::addCh ( char ch, boolean incrementHead )
{
  if (headEEPROM < MAX_EEPROM_BUFFER-1)
  {
    EEPROM.write (headEEPROM, (uint8_t) ch); 
    if (incrementHead)
      headEEPROM++;
  } 
}

int ScriptEEPROM::indexToStep ( int targetIndex )
{
  uint8_t value;
  int stepCount = 0;
  
  for (int i=1; i<targetIndex; i++)
  {
    value = readEEPROM (i);
    if (!value)
      stepCount++;
  }
  
  return stepCount;
}

int ScriptEEPROM::findStep ( int stepNum ) 
{
  int index = 1;
  uint8_t value = readEEPROM (index++);
  int stepCount = 0;
  
  while (value)
  {
    stepCount++;
    if (stepCount==stepNum)
      break;
      
    while (value)
      value = readEEPROM (index++);
      
    // skip the end of command zero terminator  
    value = readEEPROM (index++);    
  }
  index = index - 1; // Point to last processed data
    
  if (stepCount != stepNum)
  {
    index = 0;
  }      
  return index;
}

void ScriptEEPROM::showSteps ()
{
  int index = 1;
  uint8_t value = readEEPROM (index);
  int step = 0;
  
  if (debugUtils.debugging())
  {
    if (!value) 
      debugUtils.printPSTR ( PSTR ( "Program empty\n" ) );
    else
      // value at this point represents the next command
      while (value)
      {
        if (!step) // This is the first one
        {
          debugUtils.printPSTR ( PSTR ( "HeadEEPROM:" ) );
          Serial.print ( headEEPROM );
          debugUtils.printPSTR ( PSTR ( "\nSteps:\n" ) );
        }  
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
          value = readEEPROM (index++);
          if (value)
            Serial.print ((char)value );
        }
        Serial.println();        
        // value is now zero, index will point to next command or 0 to terminate
        
        value = readEEPROM (index); // value is now next command or 0 to terminate        
      }
      
  }  
}

void ScriptEEPROM::run( )
{
  debugUtils.printPSTR ( PSTR ( "Start program\n" ) );
  reset();
  writeTestState (1);  
  paused = false;
  currentCommand = readEEPROM(testIndex++);
}

// Count the number of 0s in the eeprom, stop on double zero
// Not sure if this is necessary
int ScriptEEPROM::numSteps ( )
{
  int index = 1;
  uint8_t value = readEEPROM (index++);
  int steps = 0;
  while (value)
  {
    // skip to end of command
    while (value)
      value = readEEPROM (index++);
    // skip the end of command zero terminator
    value = readEEPROM (index++);
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
int ScriptEEPROM::readDec (char * ch)
{
  int value = 0;
  int total = 0;
  boolean debugThis = false;
  while (true)
  {
    *ch = Serial.read();
    value = *ch;
    // Exit if number if not a decimal
    if ((value < '0') || (value > '9'))
      break;
    if (!value)
      break;
    else
    {
      total *= 10;
      total += value - '0';
    }    
  }
    
  return total;
}

// Do not consume the terminating 0
int ScriptEEPROM::readDecimal (int &testPointer)
{
  int value = 0;
  int total = 0;
  while (true)
  {
    value = readEEPROM (testPointer);
    // Exit if number if not a decimal
    if ((value < '0') || (value > '9'))
      break;
    if (!value)
      break;
    else
    {
      testPointer++;
      total *= 10;
      total += value - '0';
    }    
  }
    
  return total;
}

void ScriptEEPROM::showStatus()
{
  static int lastIndex = 0;
  static int lastCommand = 0;
  int cc = currentCommand;
  // If (Index change and currentCommand != Wait) OR currentCommand Changed
  if (
       ((lastIndex != testIndex) && (cc != 1)) || (cc != lastCommand)
      )
  {
    Serial.print ( testIndex-1 );
    debugUtils.printPSTR ( PSTR ( ")" ) );
    statements->printString ( cc );
    debugUtils.printPSTR ( PSTR ( "\n" ));
  } 
  lastIndex = testIndex;  
  lastCommand = cc;
}

void ScriptEEPROM::skipTo ( int value, int &index, boolean consume )
{
  while (EEPROM.read (index) != value)
    index++;
	
  if (consume) // Consume the target
    index++;
	
}

// Print the specified addMatch string from EEPROM
void ScriptEEPROM::printMatchString ( int which )
{
  int index = 1;
  uint8_t value;
  int count = 0;
  
  while (true)
  {
    value = EEPROM.read ( index++); // Get the statement 
	if (!value) // Done
	  break;
	else if (value != 14) // Not an addMatch statement
	  skipTo (0,index, true);
    else if (count++ != which) 
	  skipTo (0,index, true);
	else  // This is the addMatch we care about
 	{
      while (value)
	  {
	    value = EEPROM.read ( index++);
	    if (value)
	      Serial.print ((char) value);
      }
	}  
  }
}

/* 
  addMatch commands must be the first in the program
*/
boolean ScriptEEPROM::eepromMatch ( char ch, int which, boolean clearResults) 
{
  boolean found = false;
  int index = 1;
  uint8_t value;
  int count = 0;
  static uint8_t indexes [] = {0,0,0,0,0,0,0,0};
  static uint8_t results = 0;
  uint8_t bits[] = {1,2,4,8,16,32,64,128};
  char c;
  int i=0;
  int checkThis = -1; // Do not debug
  boolean debugThis = false;
  
  if (clearResults)
  {
    results = 0;
	for (int i=0; i<8; i++)
	  indexes[i] = 0;
	debugUtils.printPSTR ( PSTR ( "EEPROM matches cleared\n" ) );  
  }
  else if ((which > -1) && (which < 8)) // Return results for specified string
  {
    if (results & bits[which])
	{
	  found = true;
	  results &= ~bits[which]; // Consume the item
	}
  }
  else
  {  
    while (true)
    {
      value = EEPROM.read ( index++); // Get the statement 
      if (value != 14) // Done with program
        break;
  	  else if (value == 14) // addMatch
  	  {
	    //Serial.print ( count );
	    //debugUtils.printPSTR ( PSTR ( ")" ) );
	    i = 0;
	    while (value)
	    {
	      value = EEPROM.read ( index++);
	      if (value)
		  {
            if (i++ == indexes[count]) // This is the character we care about
		    {
		      c = (char) value;
              if (c == ch)
			  {
			    if (count == checkThis)
			    {
			      Serial.print ( c );
			      Serial.print ( " == " );
			      Serial.println ( ch );
			    }	
                indexes[count] = indexes[count] + 1;
			  }  
              else
			  {
			    if (count == checkThis)
			    {
			      Serial.print ( c );
			      Serial.print ( " != " );
			      Serial.println ( ch );
		        }		
                indexes[count] = 0;				  
			  }  
			  // We are done with this word
              while (value = EEPROM.read (index++));						
		    }  
  	        // Serial.print ( (char) value );
		  }  
		  else // We are at the end
		  {
		    if (indexes[count] == i) // We matched the string
		    {
			  if (debugThis)
			  {
		        debugUtils.printPSTR ( PSTR ( "Matched on " ));
			    printMatchString ( count );
			    Serial.println();
		      }		
			  results |= bits[count];
			  found = true;
	        }		
		  }
	    }
	    count++;
	  }	
	}  
  }
  return found;
}

int ScriptEEPROM::findLabel ( int index , boolean debugThis)
{
  char ch;
  int startIndex;
  int i = 0;
  uint8_t statement;
  char c;
  boolean found = false;
  int labelIndex = 0;
  
  while (true)
  {
    statement = EEPROM.read (i++);
	if (!statement) 
	{
	  if (debugThis)
	  {
	    debugUtils.printPSTR ( PSTR ( "search over : " ) );
	    Serial.println ( i-1 );
      }		
	  break; // done 
	}  
	else
	{
	  if (statement == 17) // label
	  {
	    labelIndex = i-1;
		if (debugThis)
          debugUtils.printPSTR (PSTR( "Find label: " ));
        startIndex = index;
		while (true)
		{
		  c = EEPROM.read (i++);
          ch = EEPROM.read ( startIndex++);
		  if (debugThis)
  		    Serial.print ( (char) ch );
		  if (!c)
		  {
		    found = true;
		    break;
	      }		
	      if (c != ch)
		  {
		    if (debugThis)
			{
		      debugUtils.printPSTR ( PSTR ( "i:" ) );
			  Serial.print (i);
		      debugUtils.printPSTR ( PSTR ( " c != ch: " ) );
			  Serial.print ( (char ) c );
			  Serial.print ( "!=" );
			  Serial.println ( (char) ch );
			}  
			while (EEPROM.read (i++));
            break;		  
		  }	
		}  
		
		if (!c)
		  break;
	  }  
	  else // read to zero
	    while (EEPROM.read (i++));
	}  
  }
  if (found)
  {
    i = labelIndex;
	if (debugThis)
	{
	  debugUtils.printPSTR ( PSTR ( "Label found at: " ) );
	  Serial.println ( labelIndex );
	}  
  }	
  else
  {
    if (debugThis)
      debugUtils.printPSTR ( PSTR ( "Could not find label\n" ) );
    i = -1;
  }	
  return i;
}

void ScriptEEPROM::executeStep(boolean &stepDone)
{
  static unsigned long startTimer = 0;
  static int elapsedSeconds;
  static unsigned long oneSecond;
  static int waitSeconds;
  static int totalTime;

  uint8_t value;
  uint8_t op;
  boolean done = true;
  int newStep;
  char varName;
  int * var;
  int index;
  unsigned long elapsedTime;
  
  int compareValue;
  boolean debugThis = false;
  boolean ifTrue = false;

  if (debugThis)
    showStatus();
 
  if (startTimer) // We are counting down
  {
    if (millis() > oneSecond)
    {
      elapsedSeconds++;
      oneSecond = millis() + 1000;
	  debugUtils.printPSTR ( PSTR ( "\r\n" ) );
      Serial.print ( elapsedSeconds );
      debugUtils.printPSTR ( PSTR ( " of " ) );
      Serial.print ( totalTime );
      debugUtils.printPSTR ( PSTR ( " seconds\r\n" ) ); 
      oneSecond = millis() + 1000;
    }    
    
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
      // debugUtils.printPSTR ( PSTR ("Test ended\n" ));
      break;
      
    case 1: // Wait X seconds (1 byte)
      if (waitTimeout)
      {
        if (millis() > waitTimeout) 
          waitTimeout = 0;
        else
        {
          done = false;  
          if (millis() > oneSecond) 
          {
            elapsedSeconds++;
            oneSecond = millis() + 1000;
            Serial.print ( elapsedSeconds );
            debugUtils.printPSTR ( PSTR ( " of " ) );
            Serial.print ( waitSeconds );
            debugUtils.printPSTR ( PSTR ( " seconds\n" ) ); 
          }  
        }  
      }
      else // read variable number of seconds
      {
        elapsedSeconds = 0;
        oneSecond = millis() + 1000;
        waitSeconds = readDecimal ( testIndex );
        if (debugUtils.debugging())
        {
          debugUtils.printPSTR ( PSTR ( "Wait for " ) );
          Serial.print ( waitSeconds );
          debugUtils.printPSTR ( PSTR ( " second(s)\n" ) );
        }
        waitTimeout = waitSeconds;
        waitTimeout *= 1000;
        waitTimeout += millis();
        done = false;
      }      
    break;
    
    case 2: // Statement = relaynnon or relaynnoff
      index = readDecimal (testIndex);
      if ((readEEPROM (testIndex) == 'o') && (readEEPROM(testIndex+1) == 'n')) 
      {
        debugUtils.printPSTR ( PSTR ( "Relay " ) );
        Serial.print ( index );
        debugUtils.printPSTR ( PSTR ( " on\n" ) );
        digitalWrite ( index, 1);
        testIndex = testIndex + 2; // on 
      }  
      else if ((readEEPROM (testIndex) == 'o') && (readEEPROM(testIndex+1) == 'f')&& (readEEPROM(testIndex+1) == 'f')) 
      {
        debugUtils.printPSTR ( PSTR ( "Relay " ) );
        Serial.print ( index );
        debugUtils.printPSTR ( PSTR ( " off\n" ) );
        digitalWrite (index, 0);
        testIndex = testIndex + 3; // off
      }  
      else
      {
        debugUtils.printPSTR ( PSTR ( "Unknown relay command starting at testIndex: " ) );
        Serial.println ( testIndex );
      }
      break;
     
    case 3: // Echo output text to serial monitor 
      while (true)
      {
        value = readEEPROM ( testIndex);
        if (!value)
          break;
        testIndex++; // next character  
        Serial.print ( (char) value );
      } 
      Serial.println();        
      break;
      
    case 4: // End If
      break;
     
    case 5: // Evaluate the if statement
      varName = readEEPROM ( testIndex++);
      var = 0;
      switch (varName)
      {
        case 'A':
          var = &A;
          break;
        case 'M':
          var = &M;
          break;          
        case 'T':
          var = &T;
          break;
        default:
          debugUtils.printPSTR ( PSTR ( "Unknown variable: " ));
          Serial.println ( (int)varName );
          break;
      }
      if (readEEPROM ( testIndex) == '=') // Evaluate an equal sign
      {
        testIndex++; // Consume the '='
        compareValue = readDecimal (testIndex);
        if (*var == compareValue) // If Statement evaluates true        
  		  ifTrue = true;
        else
          skipTo(4,testIndex, true); // Skipto endif
		  
		if (debugThis)
        {		
          debugUtils.printPSTR ( PSTR ( "If " ));
		  Serial.print ( varName );
		  Serial.print ( "=" );
          Serial.print ( compareValue );
		  if (ifTrue)
		    debugUtils.printPSTR ( PSTR ( " true\n" ));
		  else
		    debugUtils.printPSTR ( PSTR ( " false\n" ));
	    }		
      }
      else if (readEEPROM ( testIndex) == '<') // Evaluate a less than
      {
        testIndex++; // Consume the '<'
        compareValue = readDecimal (testIndex);
		if (debugThis)
		{
          debugUtils.printPSTR ( PSTR ( "if var <" ) );
          Serial.println ( compareValue );
		}  
        if (*var < compareValue) // If Statement evaluates true
          ifTrue = true;
        else
          skipTo(4,testIndex, true); // Skipto endif
      }
      else if (readEEPROM (testIndex) == '>') // Evaluate a greater than
      {
        testIndex++; // Consume the '>'
        compareValue = readDecimal (testIndex);
		if (debugThis)
		{
          debugUtils.printPSTR ( PSTR ( "Check var >" ) );
          Serial.println ( compareValue );
		}  
        if (*var > compareValue) // If Statement evaluates true
          ifTrue = true;
        else
          skipTo(4,testIndex, true); // Skipto endif
      }
      else if (var)
      {
        if (*var) // If statement evaluates true
          ifTrue = true;
        else
          skipTo(4,testIndex, true); // Skipto endif
		 
        if (debugThis)
        {		
          debugUtils.printPSTR ( PSTR ( "If " ));
		  Serial.print ( varName );
		  if (ifTrue)
		    debugUtils.printPSTR ( PSTR ( " true\n" ));
		  else
		    debugUtils.printPSTR ( PSTR ( " false\n" ));	
		}
      }  
      else
        debugUtils.printPSTR ( PSTR ( "Missing varname in if statement" ) );
        
      if (debugThis)
      {      
        if (ifTrue)
          debugUtils.printPSTR ( PSTR ( "If evaulated true\n" ) );
        else
          debugUtils.printPSTR ( PSTR ( "If evaluated false\n" ) );
      }  
      break;  
      
    case 6: // Jump to label  
	  testIndex = findLabel (testIndex, debugThis);
	  skipTo (0, testIndex, false);
      break;
            
    case 7: // output text to lcd
      if (debugUtils.debugging())
      {
        debugUtils.printPSTR ( PSTR ( "Output: " ) );
        while (true)
        {
          value = readEEPROM ( testIndex);
          if (!value)
            break;
          testIndex++; // Read the next character
          Serial.print ( (char) value );
        }
        debugUtils.printPSTR ( PSTR ( " to LCD\n" ) );
      }  
      break;
          
    // To Do ReadDecimal which wire slave
    case 8: // wire NN Read wire slave device nn, it will be appended to log file
      debugUtils.printPSTR ( PSTR ( "Read Wire\n" ) );
      break;
      
    case 9: // Set A {+,-,=} Number
      varName = readEEPROM ( testIndex++);
      op = readEEPROM ( testIndex++);
      var = 0;
      switch (varName)
      {
        case 'A':
          var = &A;
          break;
        default:
          debugUtils.printPSTR ( PSTR ( "Unknown var:" ));
          Serial.println ( varName );
          break;
      }
      if (var)
      {
        switch (op)
        {
          case '=': 
            *var = readDecimal ( testIndex );
			//Serial.print ( varName );
			//debugUtils.printPSTR ( PSTR ( " set to: " ) );
			//Serial.println ( *var );
            break;
          case '-':
            *var = *var - readDecimal ( testIndex );
            break;
          case '+':
            *var = *var + readDecimal ( testIndex );
            break;
          default:
            debugUtils.printPSTR ( PSTR ( "Unknown operator:" ) );
            Serial.println ( (char ) op );
            break;
        }
      }
      break;
      
    case 10: // callback (for those wanting to do a c call)      
      value = readDecimal (testIndex);
      if (callback)
        callback(value);
        
      break;

    case 11: // thermistor, set value in A
      // Get the Analog Input number 
      A = (int) components.thermistorFahrenheit (readDecimal (testIndex));
      break;      

    case 12: // startTimer
      elapsedSeconds = 0;
      totalTime = readDecimal ( testIndex );
      startTimer = millis();
      Serial.print ( "waitSeconds: " );
      Serial.println ( waitSeconds );
      T = totalTime;
      oneSecond = millis() + 1000;
      break;  
      
    case 13: // delay X milliseconds
      if (waitTimeout)
      {
        if (millis() > waitTimeout) 
          waitTimeout = 0;          
        else
          done = false;  
      }
      else // read variable number of milliseconds
      {
        waitTimeout = readDecimal ( testIndex );
        if (debugThis)
        {
          debugUtils.printPSTR ( PSTR ( "Delay " ) );
          Serial.print ( waitTimeout );
          debugUtils.printPSTR ( PSTR ( " msec\n" ) );
        }  
        else
          Serial.print ( "." ); // Give some feedback to the user
        waitTimeout += millis(); // Offset from current time       
        done = false;
      }      
    break;
    
    case 14: // addMatch
	  skipTo ( 0, testIndex, false ); // Skip the text for now
    break;  
    
    case 15: // showMatches, This is more of a debug command to check eeprom strings.
      //matches.showMatches();
      //matches.clearMatches();
      break;
      
    case 16: // checkMatch
	  
      index = readDecimal ( testIndex );
      M = 0;
	  if (eepromMatch (' ',index,false)) // Read results of string match
	  {
  	    printMatchString ( index ); 
	    debugUtils.printPSTR ( PSTR ( " was found!\n" ) );
        M = 1;
      }		
      
	  break;
	  
	case 17: // :Label
      // Increment testIndex until a zero is encountered.
      while (readEEPROM(testIndex))
        testIndex++;
      break;

    case 18: // clearMatches
      (void) eepromMatch (' ',-1,true );
      break;	

	case 19: // testState
      writeTestState (readDecimal ( testIndex ));
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
    testIndex++;
    
  stepDone = done; 
  //debugUtils.printPSTR ( PSTR ( "cc:" ));
  //Serial.println ( currentCommand );  
}

void ScriptEEPROM::removeLine(int stepNumber)
{
  int index;
  int offset;
  int i;
  int value = 0;

  if (stepNumber)
  {
    i = findStep (stepNumber);
    offset = findStep ( stepNumber + 1);
    if (!offset) // No second step
      EEPROM.write (i,0);
    else
    {
      offset = offset - i;
      // Point to the subsequent step
      index = findStep ( stepNumber ) + offset;
      while (value || readEEPROM(index))
        EEPROM.write (i++, readEEPROM (index++));        
        
      EEPROM.write ( i, 0); // Write terminating 0
    }  
    headEEPROM = i; 
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
      progmem = PSTR ( "Test Passed" );
      break;
    case 3:
      progmem = PSTR ( "Test Failed!");
      break;
    default:
      progmem = PSTR ( "Unknown state" );
      break;
  }
  return progmem;
}

int ScriptEEPROM::dumpStatement(int &index)
{
  uint8_t value = readEEPROM (index);
  int dumped = 0;
  if (value)
  {
    debugUtils.printPSTR ( PSTR ( "***Statement***\n" ) );
    Serial.print (index);
    debugUtils.printPSTR ( PSTR ( ")" ) );
    statements->printString ( value );
    index++;
    debugUtils.printPSTR ( PSTR ( "\n" ));
    while (value = readEEPROM(index))
    {
      Serial.print ( index++ );
      debugUtils.printPSTR ( PSTR(")") );
      Serial.println ( value );
    }
    index++; // Consume the 0, point to the next statement 
    dumped = 1;
  }
  return dumped;    
}

void ScriptEEPROM::writeTestState(int newState)
{
  testState = newState;
  EEPROM.write ( 0,(uint8_t) testState );
}

void ScriptEEPROM::dump()
{
  int index = 1;
  uint8_t value = readEEPROM(index);
  boolean first = false;
  
  if (!value) // Program is not-empty
    debugUtils.printPSTR ( PSTR ( "Program empty" ) );
  else
  {
    // Show the program
    while (dumpStatement ( index ) );
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
  static boolean done = true;
  switch (testState)
  {
     case 2: // Test complete
	 case 3: // Test Failed
	 case 0: // Idle 
	 break;
	 	 
	 case 1: // Test running
       executeStep (done);
       if (done)
	   {
         currentCommand = readEEPROM (testIndex++);
	     if (!currentCommand)
	     {
           debugUtils.printPSTR ( PSTR ( "Test completed\n" ) );    
	  	   writeTestState (2); // Test Completed 
	     }
	   } 
	 break;
  }  
}
