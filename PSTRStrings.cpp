#include "PSTRStrings.h"

// Print a string from program memory
void PSTRStrings::printPSTR ( PGM_P s) 
{
  char c;
  while ((c=pgm_read_byte(s++)) != 0)
    Serial.print (c);
} 

// s = pointer to PSTR string
void PSTRStrings::addString ( const prog_char * s )
{ 
  CommandStringType * str;
  if (numStrings == 0) // This is the first one
  {
    //debugUtils.printPSTR ( PSTR ( "sizeof CommandStringType: " ) );
    //Serial.print ( sizeof (CommandStringType ) );
    //debugUtils.printPSTR ( PSTR ( " initial malloc: " ) );
    strings = (CommandStringType *) malloc ( sizeof (CommandStringType) * numberOfStrings);
    //Serial.print ( (unsigned long ) strings );
    //debugUtils.printPSTR ( PSTR ( "After malloc: " ) );
    //CommandStringType * q = (CommandStringType *) malloc (3);
    //Serial.println ( (unsigned long) q );  
  }
  
  str = &strings [numStrings++];  
  //debugUtils.printPSTR ( PSTR ( "Address of str: " ) );
  //Serial.println ( (unsigned long ) str );
  
  str->ptr = (prog_char *)s;
  // str->len = progLen (str->ptr);
  str->index = 0;
}

PSTRStrings::PSTRStrings(int _numberOfStrings ):debugUtils()
{
  strings = 0;
  numStrings = 0;
  numberOfStrings = _numberOfStrings;
  // strings = dataPtr;
  // CommandStringType strs [numberOfStrings] ;
}

int PSTRStrings::progLen (PGM_P ptr) 
{
  int length = 0;
  
  char ch = pgm_read_byte (ptr);
  while (ch)
  {
    length++;
    ptr++;
    ch = pgm_read_byte (ptr);
  }
  return length;
}

void PSTRStrings::clearCommands () // Called on setup
{
  CommandStringType * str;
  for (int i=0; i<numStrings; i++)  
  {
    str = &strings [i];  
    str->index = 0;
  }
}

void PSTRStrings::show ( int startValue, int stopValue ) 
{
  
  for (int i=startValue; i<stopValue; i++)
  {
    Serial.print ( i );
    debugUtils.printPSTR ( PSTR ( ")" ) );
    printString(i);
    debugUtils.printPSTR ( PSTR ( "\n" ) );
  }
}

void PSTRStrings::printString ( int which )
{
  CommandStringType * str = &strings [which];  
  printPSTR ( str->ptr );
}

int PSTRStrings::matchCharPointer ( char * ch ) 
{
  int matched = -1;
  char c;  
  int len;
  CommandStringType * str;

  for (int i=0; i<numStrings; i++)  
  {
    str = &strings[i]; 
    len = progLen ( str->ptr );
    for (int j=0; j<len; j++)
    {
      c = pgm_read_byte (str->ptr + j);

      if (*(ch+j) != c)
      {
      
        //Serial.print ( c );
        //printPSTR ( PSTR ( " != " ) );
        //Serial.println ( (char) *(ch+j));
        break;
      }  

      if ((j+1) == len)
      {
        matched = i;
        //printPSTR ( PSTR  ( "Got a match on word ") );
        //Serial.println ( i );
        break;
      }
    }  
    if (matched > -1)
      break;
  }
  return matched;    
}

int PSTRStrings::matchCommand ( char ch, boolean doDebug ) 
{
  int matched = -1;
  char c;  
  CommandStringType * str;
  int watching = 4;
  int len;
  
  for (int i=0; i<numStrings; i++)  
  {
    str = &strings [i]; 
    c = pgm_read_byte (str->ptr + str->index);

    if (ch == c)
    {
      if (doDebug && (i==watching))
      {
        Serial.print ( ch );
        printPSTR ( PSTR  ( " == " ));
        Serial.println ( c );
      }  
      str->index++;
    }  
    else
    {
      if (doDebug && (i==watching))
      {
        Serial.print ( ch );
        printPSTR ( PSTR  ( " != ") );
        if ((int)c < 33) 
          Serial.println ( (int) c );
        else          
          Serial.println (c);
      }  
      str->index = 0;
    }  
    len = progLen (str->ptr);
    if (str->index == len)
    {
      matched = i;
      // Clear index for next command
      str->index = 0; 
      if (doDebug)
      {
        printPSTR ( PSTR  ( "Got a match on") );
        Serial.println ( i );
      }
      break;
    }
  }
  return matched;    
}

