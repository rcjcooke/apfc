#include "AlarmGenerator.hpp"

/*******************************
 * Constructors
 *******************************/
// Constructor requires the number of unique alarm codes that could be generated
AlarmGenerator::AlarmGenerator(char uniqueAlarmGeneratorCode, int numAlarmCodes) {
  mUniqueAlarmGeneratorCode = uniqueAlarmGeneratorCode;
  mNumAlarmCodes = numAlarmCodes;
  mAlarmStates = new bool[numAlarmCodes];
  for (int i = 0; i < numAlarmCodes; i++) {
    mAlarmStates[i] = false;
  }  
}

/*******************************
 * Getters / Setters
 *******************************/
// Get the total number of possible alarm codes generated by this instance
int AlarmGenerator::getNumAlarmCodes() const {
  return mNumAlarmCodes;
}

// Get the array of alarm states
bool* AlarmGenerator::getAlarmStates() const {
  return mAlarmStates;
};

// Get the unique character representing this alarm generator instance
char AlarmGenerator::getUniqueAlarmGeneratorCode() const {
  return mUniqueAlarmGeneratorCode;
}


/*******************************
 * Event handling
 *******************************/
// Defines a function to call when an alarm occurs
void AlarmGenerator::onAlarm(volatile AlarmGeneratorPtrCharIntFuncPtr onAlarmFunction) {
  mOnAlarmFunction = onAlarmFunction;
}

// Defines a function to call when an alarm clears
void AlarmGenerator::onClear(volatile AlarmGeneratorPtrCharIntFuncPtr onClearFunction) {
  mOnClearFunction = onClearFunction;
}

// Called when triggering an alarm
void AlarmGenerator::triggerAlarm(int alarmCode) {
  // Make sure it hasn't been triggered already
  if (!mAlarmStates[alarmCode]) {
    mAlarmStates[alarmCode] = true;
    if (mOnAlarmFunction) mOnAlarmFunction(this, mUniqueAlarmGeneratorCode, alarmCode);
  }
}

// Called when clearing an alarm
void AlarmGenerator::clearAlarm(int alarmCode) {
  // Make sure it has been triggered
  if (mAlarmStates[alarmCode]) {
    mAlarmStates[alarmCode] = false;
    if (mOnClearFunction) mOnClearFunction(this, mUniqueAlarmGeneratorCode, alarmCode);
  }
}