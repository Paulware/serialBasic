#ifndef ARDUINOBASIC_H
#define ARDUINOBASIC_H

#include <Arduino.h>
#include "PSTRStrings.h"
#include "ScriptEEPROM.h"
#include "DebugUtilities.h"

class ArduinoBASIC
{
public:          
  ArduinoBASIC();
  PSTRStrings commands;   // BASIC commands
  PSTRStrings statements; // BASIC Statements
  ScriptEEPROM eepromProgram;  
  
  void handleChar ( char ch );
  
private:
  DebugUtilities debugUtils;
  
};
#endif


