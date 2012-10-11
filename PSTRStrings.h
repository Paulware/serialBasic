#ifndef PSTRSTRINGS_H
#define PSTRSTRINGS_H

#include <Arduino.h>
struct CommandStringType
{
  int index;
  int len;
  prog_char * ptr;
};
class PSTRStrings
{
  public:          
    // Constructor
    PSTRStrings(int numberOfStrings);
    int matchCommand ( char ch ); 
    void printString ( int which );
    void clearCommands ();
    void addString (const prog_char * s);
    
  private:  
    void printPSTR ( PGM_P s); 
    int progLen (PGM_P ptr);

    int numStrings;
    CommandStringType * strings;
};
#endif


