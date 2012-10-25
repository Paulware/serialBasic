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
}

void ScriptEEPROM::reset()
{
  int value;
  
  testNumber = 0;
  headEEPROM = 0;
  
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
      debugUtils.printPSTR ( PSTR ( "headEEPROM = " ) );
      Serial.println ( headEEPROM );
    }  
  }
  
  if (headEEPROM == 1024)
  {
    debugUtils.printPSTR ( PSTR ( "\nEEPROM corruption detected...clearing\n" ) );
    EEPROM.write (0,0);
  }  
  
  testIndex = 0;
  testState = 0;
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
  testIndex = 0;
  EEPROM.write (0,0);
  headEEPROM = 0;
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
  
  for (int i=0; i<targetIndex; i++)
  {
    value = readEEPROM (i);
    if (!value)
      stepCount++;
  }
  
  return stepCount;
}

int ScriptEEPROM::findStep ( int stepNum ) 
{
  int index = 0;
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
  int index = 0;
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
  testState = 1;
  
  currentCommand = readEEPROM(testIndex++);
}

// Return an EEPROM pointer, that points to the next command=cmd
void ScriptEEPROM::skipToNext (uint8_t cmd)
{
  int index = testIndex; // Looking for next 
  uint8_t value = readEEPROM (index); // Read the first command 
  
  while (true)  
  {
    // Read until end of statement  
    while (value)
      value = readEEPROM (index++);  
    
    value = readEEPROM (index++); // Read the next command   
    if (value == cmd) 
      break; 
    else if (!value)
      break;
  }  
  
  if (value == cmd)
  {
    testIndex = index;
    currentCommand = cmd;
  }   
  else
  {
    debugUtils.printPSTR ( PSTR ( "Could not find next: " ));
    Serial.println ( cmd );
  }  
}

// Count the number of 0s in the eeprom, stop on double zero
// Not sure if this is necessary
int ScriptEEPROM::numSteps ( )
{
  int index = 0;
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

/* 
  addMatch commands must be the first in the program
*/
boolean ScriptEEPROM::eepromMatch ( char ch, int which) 
{
  boolean found = false;
  int index = 0;
  uint8_t value;
  int count = 0;
  static uint8_t indexes [] = {0,0,0,0,0,0,0,0};
  static uint8_t results = 0;
  uint8_t bits[] = {1,2,4,8,16,32,64,128};
  char c;
  int i=0;
  int checkThis = -1; // Do not debug
  
  if (which > -1)
  {
    if (results & bits[which])
	  found = true;  
	results &= ~bits[which]; // Consume the information	  
	results = 0; // TODO correct
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
		      //debugUtils.printPSTR ( PSTR ( "Got a string match on " ));
			  //Serial.print ( count );
			  results |= bits[count];
	        }		
		  }
	    }
	    count++;
	  }	
	}  
  }
  return found;
}

int ScriptEEPROM::findLabel ( int index )
{
  char ch;
  int startIndex;
  int i = 0;
  uint8_t statement;
  char c;
  boolean found = false;
  int labelIndex = 0;
  boolean debugThis = false;
  
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
		
		if (debugThis)
		  Serial.println ( );
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
  static unsigned long waitTimeout = 0;
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
      Serial.print ( elapsedSeconds );
      debugUtils.printPSTR ( PSTR ( " of " ) );
      Serial.print ( totalTime );
      debugUtils.printPSTR ( PSTR ( " seconds\n" ) ); 
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
      debugUtils.printPSTR ( PSTR ("Test ended\n" ));
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
        debugUtils.printPSTR ( PSTR ( "Check var =" ) );
        Serial.println ( compareValue );
        if (*var == compareValue) // If Statement evaluates true
          ifTrue = true;
        else
        {
          skipToNext(4);
          done = false;
        }
      }
      else if (readEEPROM ( testIndex) == '<') // Evaluate a less than
      {
        testIndex++; // Consume the '<'
        compareValue = readDecimal (testIndex);
        debugUtils.printPSTR ( PSTR ( "Check var <" ) );
        Serial.println ( compareValue );
        if (*var < compareValue) // If Statement evaluates true
          ifTrue = true;
        else
        {
          testIndex++; // Consume the zero terminator
          skipToNext(4);
          done = false;
        }
      }
      else if (readEEPROM (testIndex) == '>') // Evaluate a greater than
      {
        testIndex++; // Consume the '>'
        compareValue = readDecimal (testIndex);
        debugUtils.printPSTR ( PSTR ( "Check var >" ) );
        Serial.println ( compareValue );
        if (*var > compareValue) // If Statement evaluates true
          ifTrue = true;
        else
        {
          skipToNext (4);
          done = false;
        }
      }
      else if (var)
      {
	    //Serial.print ( "A:" );
		//Serial.println ( A );
        if (*var) // If statement evaluates true
          ifTrue = true;
        else
        {
          testIndex++;
          skipToNext (4);
          done = false;
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
      // Handled Below; 
	  index = findLabel (testIndex);
	  if (index > -1)
	  {
	    testIndex = index;
        done = false; // New step will not be completed until later
        currentCommand = readEEPROM ( testIndex++ );
      }		
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
			Serial.print ( varName );
			debugUtils.printPSTR ( PSTR ( " set to: " ) );
			Serial.println ( *var );
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
      
    case 13: // Wait X milliseconds
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
	   
      //if (matches.numStrings < NUMBER_OF_MATCHES )
      //{
	  //  debugUtils.printPSTR ( PSTR ( "matches.addString\n" ) );
      //  matches.addString (0,testIndex);
        // Increment testIndex until a zero is encountered.
        while (readEEPROM(testIndex))
          testIndex++;
      //}  
      //else
      //  debugUtils.printPSTR ( PSTR ( "Matches full\n" ) );      	
    break;  
    
    case 15: // showMatches, This is more of a debug command to check eeprom strings.
      //matches.showMatches();
      //matches.clearMatches();
      break;
      
    case 16: // isMatched
	  
      index = readDecimal ( testIndex );
      M = 0;
	  if (eepromMatch (' ',index)) 
        M = 1;
      
	  break;
	  
	case 17: // :Label
        // Increment testIndex until a zero is encountered.
        while (readEEPROM(testIndex))
          testIndex++;
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

/*
void ScriptEEPROM::insertCh ( int index, char ch ) 
{
  char temp = ch;
  int i = index;
  
  while (temp || readEEPROM(index))
  {
    temp = readEEPROM (index);
    EEPROM.write (index++, ch);
    ch = temp;
  } 
  EEPROM.write (index,0); // Terminate the program
  
}

void ScriptEEPROM::change(int stepNumber, char ch)
{
  int index;
  int offset;
  int i;
  int value = 0;
  int startIndex;
  
  if (stepNumber)
  { // Delete the step
    i = findStep (stepNumber);
    startIndex = i;
    offset = findStep ( stepNumber + 1) - i;
    Serial.print ( "Offset: " );
    Serial.println ( offset );
    if (offset > 0) // TODO: delete the last step
    {
      // Point to the subsequent step
      index = findStep ( stepNumber ) + offset;
      while (value || readEEPROM(index))
      {
        value = readEEPROM (index);
        Serial.print ( "value: " );
        Serial.print ( value );
        Serial.print ( " index: " );
        Serial.println ( index++ );
        EEPROM.write (i++, value);
      }        
      EEPROM.write ( i, 0); // Write terminating 0
    }  
    
    while (ch != 13)
    {
      insertCh ( startIndex++, ch );
      ch = Serial.read();
    }
    insertCh ( startIndex, 0); // Terminate the step
    Serial.println ( "All done inserting" );
  }  
}
*/
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

void ScriptEEPROM::dump()
{
  int index = 0;
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
    
  if (done && !currentCommand)
  {
    debugUtils.printPSTR ( PSTR ( "Test completed successfully\n" ) );
    testState = 0;
  }  
  else
  {
    executeStep (done);
    if (done)
      currentCommand = readEEPROM (testIndex++);
  }  
}
