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
  CommandStringType * str = strings + (sizeof (CommandStringType) * numStrings++);
  
  str->ptr = (prog_char *)s;
  str->len = progLen (str->ptr);
  str->index = 0;
}

 PSTRStrings::PSTRStrings(int numberOfStrings )
{
  numStrings = 0;
  strings = (CommandStringType *)calloc (numberOfStrings, sizeof (CommandStringType)); 
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
    str = strings + (sizeof(CommandStringType)*i);
    str->index = 0;
  }
}

void PSTRStrings::printString ( int which )
{
  CommandStringType * str = strings + (sizeof (CommandStringType) * which);
  printPSTR ( str->ptr );
}

int PSTRStrings::matchCommand ( char ch ) 
{
  int matched = -1;
  char c;  
  CommandStringType * str;

  for (int i=0; i<numStrings; i++)  
  {
    str = strings + (sizeof(CommandStringType)*i);  
    c = pgm_read_byte (str->ptr + str->index);

    if (ch == c)
    {
      //Serial.print ( ch );
      //printPSTR ( PSTR  ( " == " ));
      //Serial.println ( c );
      str->index++;
    }  
    else
    {
      //Serial.print ( ch );
      //printPSTR ( PSTR  ( " != ") );
      //Serial.println (c);
      str->index = 0;
    }  

    if (str->index == str->len)
    {
      matched = i;
      // Clear index for next command
      str->index = 0; 
      break;
    }
  }
  return matched;    
}

