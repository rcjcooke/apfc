#include "AlarmGenerator.hpp"

/*******************************
 * Constructors
 *******************************/
// Constructor requires the number of unique alarm codes that could be generated
AlarmGenerator::AlarmGenerator(int numAlarmCodes) {
  mAlarmStates = new bool[numAlarmCodes];
  for (int i = 0; i < numAlarmCodes; i++) {
    mAlarmStates[i] = false;
  }  
}

/*******************************
 * Event handling
 *******************************/
// Defines a function to call when an alarm occurs
void AlarmGenerator::onAlarm(volatile IntFuncPtr onAlarmFunction) {
  mOnAlarmFunction = onAlarmFunction;
}

// Defines a function to call when an alarm clears
void AlarmGenerator::onClear(volatile IntFuncPtr onClearFunction) {
  mOnClearFunction = onClearFunction;
}

// Called when triggering an alarm
void AlarmGenerator::triggerAlarm(int alarmCode) {
  // Make sure it hasn't been triggered already
  if (!mAlarmStates[alarmCode]) {
    mAlarmStates[alarmCode] = true;
    if (mOnAlarmFunction) mOnAlarmFunction(alarmCode);
  }
}

// Called when clearing an alarm
void AlarmGenerator::clearAlarm(int alarmCode) {
  // Make sure it has been triggered
  if (mAlarmStates[alarmCode]) {
    mAlarmStates[alarmCode] = false;
    if (mOnClearFunction) mOnClearFunction(alarmCode);
  }
}
