#ifndef __SERIALDEBUGGER_H_INCLUDED__
#define __SERIALDEBUGGER_H_INCLUDED__

#include <Arduino.h>
#include "HashMap.h"
#include "SerialDisplay.hpp"

const unsigned int MAX_DEBUG_VALUES = 15;

class SerialDebugger : public SerialDisplay {
public:
  SerialDebugger(unsigned long baud);

  bool updateValue(String variable, String value);
  bool updateValue(String variable, unsigned long value);
  bool updateValue(String variable, double value);
  bool updateValue(String variable, float value);
  bool updateValue(String variable, int value);
  
  void printUpdate();

  unsigned long mNextPrintMillis;
  HashMap<String, String, MAX_DEBUG_VALUES> mStatusValues;

private:

  bool internalUpdateValue(String variable, String value);

};

#endif // __SERIALDEBUGGER_H_INCLUDED__