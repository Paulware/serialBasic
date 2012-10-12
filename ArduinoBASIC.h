#ifndef ARDUINOBASIC_H
#define ARDUINOBASIC_H

#include <Arduino.h>
#include "PSTRStrings.h"
#include "ScriptEEPROM.h"
#include "DebugUtilities.h"

class ArduinoBASIC
{
public:          
  //typedef void (*VoidType)();  
  ArduinoBASIC();
  PSTRStrings commands;   // BASIC commands
  PSTRStrings statements; // BASIC Statements
  ScriptEEPROM eepromProgram; 
  void init ();   
  void handleChar ( char ch );
  void setCallback (VoidType _callback);
  
private:
  void continueStatement ( char ch );
  DebugUtilities debugUtils;
  int lastStatement;

};
#endif


