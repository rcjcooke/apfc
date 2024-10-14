#ifndef __SERIALDEBUGGER_H_INCLUDED__
#define __SERIALDEBUGGER_H_INCLUDED__

#include <Arduino.h>
#include "HashMap.h"
#include "SerialDisplay.hpp"

const unsigned int MAX_DEBUG_VALUES = 15;

class SerialDebugger : public SerialDisplay {
public:
  SerialDebugger(unsigned long baud, bool getInput);
  SerialDebugger(unsigned long baud) : SerialDebugger(baud, false) {};

  bool updateValue(String variable, String value);
  bool updateValue(String variable, unsigned long value);
  bool updateValue(String variable, double value);
  bool updateValue(String variable, float value);
  bool updateValue(String variable, int value);
  
  // Print an update but make sure it's not too often
  void throttledPrintUpdate();
  // Prints the update to screen
  void printUpdate();
  // Get any additional user input since the last check. Non-blocking.
  void getAndProcessUserInputUpdates();

  unsigned long mNextPrintMillis = 0;
  HashMap<String, String, MAX_DEBUG_VALUES> mStatusValues;

  /*******************************
   * Event handling
   *******************************/
  // Define event handler function type
  typedef void (*VoidFuncStringStringPtr)(String, String);
  // Defines the function to call on a spray stop event
  void onValueChanged(volatile VoidFuncStringStringPtr onHandlerFunction);

private:

  /*******************************
   * Member variables
   *******************************/
  // The function to call if the user chooses to change one of the values added to this SerialDebugger
  volatile VoidFuncStringStringPtr mOnValueChangedHandlerFunction = nullptr;
  // If true then a value is currently being selected, if false then a new value is being selected
  bool mValueSelection = true;
  // If true, the this debugger will provide the ability for the user to change values
  bool mGetInput = false;

  /*******************************
   * Private functions
   *******************************/
  // Get raw serial input from the user and update inputValue with it. Return true if a terminator has been received.
  bool handleRawSerialInput(String &inputValue);
  // Handle any user input received
  void processUserInput(String variable, String newValue);
  // Updates values in the hashmap
  bool internalUpdateValue(String variable, String value);

};

#endif // __SERIALDEBUGGER_H_INCLUDED__