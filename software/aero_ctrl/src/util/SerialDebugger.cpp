#include "SerialDebugger.hpp"

SerialDebugger::SerialDebugger() : SerialDisplay(SerialDisplayType::ansi_vt100) {
  Serial.begin(115200);
  mNextPrintMillis = millis();
  mStatusValues = HashMap<String, String, MAX_DEBUG_VALUES>();
}

void SerialDebugger::updateValue(String variable, String value) {
  mStatusValues[variable] = value;
}

void SerialDebugger::updateValue(String variable, float value) {
  mStatusValues[variable] = String(value);
}

void SerialDebugger::updateValue(String variable, int value) {
  mStatusValues[variable] = String(value);
}

void SerialDebugger::printUpdate() {
  if (millis() > mNextPrintMillis) {
    mNextPrintMillis = millis() + 200;
    
    clearSerialDisplay();

    Serial.println("------ Now: " + String(millis()) + " ---------");

    for (unsigned int i=0; i<mStatusValues.size(); i++) {
      Serial.println(mStatusValues.keyAt(i) + ": " + mStatusValues.valueAt(i));
    }
  }  
}