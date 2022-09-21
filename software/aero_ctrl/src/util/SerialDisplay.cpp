#include <Arduino.h>

#include "SerialDisplay.hpp"

SerialDisplay::SerialDisplay(SerialDisplayType serialDisplayType) {
  mSerialDisplayType = serialDisplayType;
}

void SerialDisplay::clearSerialDisplay() {
  switch(mSerialDisplayType) {
    case SerialDisplayType::ansi_vt100:
      Serial.print("\e[2J");
      Serial.print("\e[H");
      break;
    default:
      break;
  }
}
