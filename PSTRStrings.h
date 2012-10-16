#ifndef PSTRSTRINGS_H
#define PSTRSTRINGS_H

#include <Arduino.h>
#include "DebugUtilities.h"

struct CommandStringType
{
  uint8_t index;
  // uint8_t len;
  prog_char * ptr;
  // char * ptr;
  // PGM_P ptr;
};

class PSTRStrings
{
  public:          
    // Constructor
    PSTRStrings(int _numberOfStrings);
    int matchCommand ( char ch, boolean doDebug); 
    void printString ( int which );
    void clearCommands ();
    void addString (const prog_char * s);
    void show ( int startValue, int stopValue ); 
    int matchCharPointer ( char * ch ); 
    
  private:  
    void printPSTR ( PGM_P s); 
    int progLen (PGM_P ptr);

    int numStrings;
    int numberOfStrings;
    CommandStringType * strings;
    DebugUtilities debugUtils;
};
#endif


