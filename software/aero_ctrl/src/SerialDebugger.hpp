#ifndef __SERIALDEBUGGER_H_INCLUDED__
#define __SERIALDEBUGGER_H_INCLUDED__

#include <Arduino.h>
#include "HashMap.h"
#include "SerialDisplay.hpp"

const unsigned int MAX_DEBUG_VALUES = 10;

class SerialDebugger : public SerialDisplay {
public:
  SerialDebugger();

  void updateValue(String variable, String value);
  void updateValue(String variable, float value);
  void updateValue(String variable, int value);
  
  void printUpdate();

  unsigned long mNextPrintMillis;
  HashMap<String, String, MAX_DEBUG_VALUES> mStatusValues;

};

#endif // __SERIALDEBUGGER_H_INCLUDED__