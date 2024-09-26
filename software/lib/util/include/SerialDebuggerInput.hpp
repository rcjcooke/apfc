#ifndef __SERIALDEBUGGERINPUT_H_INCLUDED__
#define __SERIALDEBUGGERINPUT_H_INCLUDED__

#include <Arduino.h>
#include "HashMap.h"
#include "SerialDebugger.hpp"

/**
 * Allows the user to request changes to any of the values updated on this SerialDebugger.
 * 
 * Usage:
 * 
 * void setup() {
 *    SerialDebuggerInput* sdi->onValueChanged(myHandlerFunction);
 * }
 * 
 * void myHandlerFunction(String variable) {
 *    if(variable="Thing") {
 *      thingOwner.setThing(thing);   
 *    }
 * }
 */ 
class SerialDebuggerInput : public SerialDebugger {
public:
  SerialDebuggerInput(unsigned long baud);

  // Get any additional user input since the last check. Non-blocking.
  void getAndProcessUserInputUpdates();
  // Overridden to add user input request string
  void printUpdate();
  // Overriden to use this class' printUpdate
  void throttledPrintUpdate();

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
  volatile VoidFuncStringStringPtr mOnValueChangedHandlerFunction;
  // If true then a value is currently being selected, if false then a new value is being selected
  bool mValueSelection = true;

  /*******************************
   * Private functions
   *******************************/
  // Get raw serial input from the user and update inputValue with it. Return true if a terminator has been received.
  bool handleRawSerialInput(String &inputValue);
  // Handle any user input received
  void processUserInput(String variable, String newValue);

};

#endif // __SERIALDEBUGGERINPUT_H_INCLUDED__