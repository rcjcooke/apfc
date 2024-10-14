#include "SerialDebugger.hpp"

SerialDebugger::SerialDebugger(unsigned long baud, bool getInput) : SerialDisplay(SerialDisplayType::ansi_vt100) {
  Serial.begin(baud);
  // Wait for initialisation of the serial interface
  while(!Serial);
  mGetInput = getInput;
  mValueSelection = true;
  mStatusValues = HashMap<String, String, MAX_DEBUG_VALUES>();
}

/*******************************
 * Event handling
 *******************************/
void SerialDebugger::onValueChanged(volatile VoidFuncStringStringPtr onHandlerFunction) {
  mOnValueChangedHandlerFunction = onHandlerFunction;
}

/*******************************
 * Actions
 *******************************/
bool SerialDebugger::updateValue(String variable, String value) {
  return internalUpdateValue(variable, value);
}

bool SerialDebugger::updateValue(String variable, unsigned long value) {
  return internalUpdateValue(variable, String(value));
}

bool SerialDebugger::updateValue(String variable, double value) {
  return internalUpdateValue(variable, String(value));
}

bool SerialDebugger::updateValue(String variable, float value) {
  return internalUpdateValue(variable, String(value));
}

bool SerialDebugger::updateValue(String variable, int value) {
  return internalUpdateValue(variable, String(value));
}

void SerialDebugger::processUserInput(String variable, String newValue) {
  if (mOnValueChangedHandlerFunction) mOnValueChangedHandlerFunction(variable, newValue);
}

bool SerialDebugger::internalUpdateValue(String variable, String value) {
  // Check that we aren't going to add too many entries to the map
  if (!mStatusValues.contains(variable) && mStatusValues.willOverflow()) return false;
  mStatusValues[variable] = value;
  return true;
}

void SerialDebugger::printUpdate() {
  clearSerialDisplay();

  Serial.println("------ Now: " + String(millis()) + " ---------");

  for (unsigned int i=0; i<mStatusValues.size(); i++) {
    Serial.println(String(i) + ". " + mStatusValues.keyAt(i) + ": " + mStatusValues.valueAt(i));
  }

  if (mGetInput) {
    if (mValueSelection) {
      Serial.print("\nType number of value to change and <enter>: ");
    } else {
      Serial.print("\nType new value and <enter> (blank to cancel): ");
    }
  }
}

void SerialDebugger::throttledPrintUpdate() {
  unsigned long now = millis();
  if (mNextPrintMillis == 0) mNextPrintMillis = now;
  if (now > mNextPrintMillis) {
    mNextPrintMillis = now + 200;
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
bool SerialDebugger::handleRawSerialInput(String &inputValue) {
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
void SerialDebugger::getAndProcessUserInputUpdates() {
  if (mGetInput) {
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
}