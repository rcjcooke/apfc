#include "SerialDebuggerInput.hpp"

/*******************************
 * Constructors
 *******************************/
SerialDebuggerInput::SerialDebuggerInput(unsigned long baud) : SerialDebugger(baud) {
}

/*******************************
 * Event handling
 *******************************/
void SerialDebuggerInput::onValueChanged(volatile VoidFuncStringStringPtr onHandlerFunction) {
  mOnValueChangedHandlerFunction = onHandlerFunction;
}

/*******************************
 * Actions
 *******************************/
void SerialDebuggerInput::processUserInput(String variable, String newValue) {
  if (mOnValueChangedHandlerFunction) mOnValueChangedHandlerFunction(variable, newValue);
}

void SerialDebuggerInput::printUpdate() {
  // Print all the values in the debugger as usual
  SerialDebugger::printUpdate();

  if (mValueSelection) {
    Serial.print("\nType number of value to change and <enter>: ");
  } else {
    Serial.print("\nType new value and <enter> (blank to cancel): ");
  }
}

/**
 * I don't like this - it's just a duplicate of the SerialDebugger method. Not sure what the
 * better way is though
 */
void SerialDebuggerInput::throttledPrintUpdate() {
  if (millis() > mNextPrintMillis) {
    mNextPrintMillis = millis() + 200;
    printUpdate();
  }
}

/**
 * Processes what raw input is available and updates the referenced value.
 *  
 * Returns true if the termination character (\r) has been received. If there is more 
 * in the buffer after the termination character then this will not be processed and 
 * will be lost.
 */ 
bool SerialDebuggerInput::handleRawSerialInput(String &inputValue) {

  bool terminated = false;
  // Process input - waiting for the terminator (enter key)
  while (!terminated && Serial.available() > 0) {
    int input=0;
    input = Serial.read();
    
    // Correct for terminals that pad out 7-bit ASCII to 8 bits with an extra high bit 
    // (like Putty - pretty sure it's because it's translating but I don't care at this point!) 
    if (input > 127) {
      input = input - 128;
    }

    if (input == '\r') {
      terminated = true;
    } else if (input == '\b') {
      inputValue.remove(inputValue.length()-1, 1);
    } else {
      inputValue.concat((char) input);
    }
  }
  return terminated;
}

// Get any additional user input since the last check and process it. Non-blocking.
void SerialDebuggerInput::getAndProcessUserInputUpdates() {
  static String inputValue="";
  static int valueToChange=0; 
  // Process the new buffer content and update the inputValue with it
  bool terminated = handleRawSerialInput(inputValue);
  
  if (terminated) {
    if (mValueSelection) {
      long valueNumber = inputValue.toInt();
      if (valueNumber > 0 && valueNumber < mStatusValues.size()) {
        valueToChange = (int) valueNumber;
      }
      mValueSelection = false;
    } else {
      // If no new value entered, cancel out
      if (inputValue.length() != 0) {
        String key = mStatusValues.keyAt(valueToChange);
        // Note: New string creation here is deliberate
        processUserInput(key, "" + inputValue);
      }
      mValueSelection = true;
    }
    // Whatever happens, it's terminated so start again
    inputValue = "";
  }

}