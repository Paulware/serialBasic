#ifndef SCRIPTEEPROM_H
#define SCRIPTEEPROM_H

#include <Arduino.h>
#include <I2C_eeprom.h>
#include "DebugUtilities.h"
#include "PSTRStrings.h"
#include "Components.h"

#define MAX_EEPROM_BUFFER 1024

typedef void (*VoidCallbackType)();
typedef void (*IntCallbackType)(int value);      


class ScriptEEPROM
{
  public:          
    // Constructor
    ScriptEEPROM(PSTRStrings * _statements);
    void showSteps ();
    int findStep ( int step ); 
    boolean addCh (char ch, boolean incrementHead);
    int numSteps (); 
    void reset(); // Stop test
    void continueTest();
    void executeStep (int * step);
    void clear();  // Clear out the script
    
    const prog_char * testStatus ();
    char * getCh ( char * parameter, char * ch );
    char * readHexFromChar (char * hex, int * total);
    char lcase ( char ch );
    void init();
    void dump();
    int testState; // 0 = Idle, 1 = In Progress, 2 = Successfully completed, 3 = Test Failure Detected 
    void run(); // Start the program
    IntCallbackType callback;
    void del(int stepNumber); // Delete a step
    // void change(int stepNumber, char c); // Change a step
    int readDec (char * ch);
    // void insertCh ( int index, char ch );

  private:
    int headEEPROM; // EEPROM index
    int testIndex;
    int testNumber;
    int currentCommand; // Current Test Command
    DebugUtilities debugUtils;    
    // PROGMEM const char * decodeCommand (uint8_t command );
    int showDecimal (int index);
    int readDecimal (int * testPointer);
    int skipTo (uint8_t cmd, int index );  // Return an EEPROM pointer to the specified command
    int indexToStep (int indexValue);

    int A;
    int E;
    int T;
    int currentStep;
    void processWire ();
    // The eeprom should be able to save 128K of bytes, but only 2 bytes of address? = 64K?
    unsigned long eepromTail;
    unsigned long eepromHead;
    PSTRStrings * statements;
    Components components;
};
#endif


