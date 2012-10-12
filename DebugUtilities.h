#ifndef DEBUGUTILITIES_H
#define DEBUGUTILITIES_H
#include <Arduino.h>
class DebugUtilities
{
  public:          
    // Constructor
    DebugUtilities();  
    boolean debugging (); 
    void printPSTR ( PGM_P s );
    void showNumber ( PGM_P s, int number );

  // private:
};
#endif


