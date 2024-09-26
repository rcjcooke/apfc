#include "SerialDebugger.hpp"

SerialDebugger::SerialDebugger(unsigned long baud) : SerialDisplay(SerialDisplayType::ansi_vt100) {
  Serial.begin(baud);
  // Wait for initialisation of the serial interface
  while(!Serial);
  mNextPrintMillis = millis();
  mStatusValues = HashMap<String, String, MAX_DEBUG_VALUES>();
}

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

bool SerialDebugger::internalUpdateValue(String variable, String value) {
  // Check that we aren't going to add too many entries to the map
  if (!mStatusValues.contains(variable) && mStatusValues.willOverflow()) return false;
  mStatusValues[variable] = value;
  return true;
}

void SerialDebugger::printUpdate() {
  if (millis() > mNextPrintMillis) {
    mNextPrintMillis = millis() + 200;
    
    clearSerialDisplay();

    Serial.println("------ Now: " + String(millis()) + " ---------");

    for (unsigned int i=0; i<mStatusValues.size(); i++) {
      Serial.println(i + ". " + mStatusValues.keyAt(i) + ": " + mStatusValues.valueAt(i));
    }
  }  
}