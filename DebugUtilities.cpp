// Comment out next line to remove debugging
// Note: Debugging can significantly increase the project response time
#define DEBUGIT
#include "DebugUtilities.h"

DebugUtilities::DebugUtilities ()
{
}

// Print a string from program memory
void DebugUtilities::printPSTR ( PGM_P s) 
{
  #ifdef DEBUGIT
  char c;
  while ((c=pgm_read_byte(s++)) != 0)
    Serial.print (c);
  #endif
} 

boolean DebugUtilities::debugging () 
{
  #ifdef DEBUGIT
    return true;
  #else
    return false;
  #endif
}
