#ifndef PSTRSTRINGS_H
#define PSTRSTRINGS_H

#include <Arduino.h>
#include "DebugUtilities.h"

struct CommandStringType
{
  int index;
  int len;
  prog_char * ptr;
  // Note: Removing the next line causes word and bizarre behavior
  uint8_t pad; // To stay on word boundary?
};

class PSTRStrings
{
  public:          
    // Constructor
    PSTRStrings(int numberOfStrings);
    int matchCommand ( char ch, boolean doDebug); 
    void printString ( int which );
    void clearCommands ();
    void addString (const prog_char * s);
    void show ( int startValue, int stopValue ); 
    
  private:  
    void printPSTR ( PGM_P s); 
    int progLen (PGM_P ptr);

    int numStrings;
    CommandStringType * strings;
    DebugUtilities debugUtils;
};
#endif


