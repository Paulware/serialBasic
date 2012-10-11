#ifndef SCRIPTEEPROM_H
#define SCRIPTEEPROM_H

#include <Arduino.h>
#include <I2C_eeprom.h>
#include "DebugUtilities.h"

#define MAX_EEPROM_BUFFER 1024

class ScriptEEPROM
{
  public:          
    // Constructor
    ScriptEEPROM();
    void showSteps ();
    int findStep ( int step ); 

    boolean addStep (uint8_t command, char * parameter);
    int numSteps (); 
    void reset(); // Stop test
    void continueTest();
    void executeStep ();
    void clear();  // Clear out the script
    
    const prog_char * testStatus ();
    // Update the char pointer and read a single ch
    char * getCh ( char * parameter, char * ch );
    char * readHexFromChar (char * hex, int * total);
    char lcase ( char ch );
    char * matchPSTR (char * param, const prog_char * command);
    char * findCommand (char * param, int8_t  * cmd);
    void init();
    void dumpEEPROM();
    int testState; // 0 = Idle, 1 = In Progress, 2 = Successfully completed, 3 = Test Failure Detected 
      
  private:
    int headEEPROM; // EEPROM index
    int testIndex;
    int testNumber;
    int currentCommand; // Current Test Command
    DebugUtilities debugUtils;    
    PROGMEM const char * decodeCommand (uint8_t command );
    int showDecimal (int index);
    int readDecimal (int * testPointer);
    int skipTo (uint8_t cmd, int index );  // Return an EEPROM pointer to the specified command
    int indexToStep (int indexValue);

    int A;
    int E;
    int totalTime;
    int currentStep;
    void processWire ();
    // The eeprom should be able to save 128K of bytes, but only 2 bytes of address? = 64K?
    unsigned long eepromTail;
    unsigned long eepromHead;
    unsigned long startTime;



};
#endif


