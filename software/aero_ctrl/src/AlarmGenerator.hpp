#ifndef __ALARMGENERATOR_H_INCLUDED__
#define __ALARMGENERATOR_H_INCLUDED__

class AlarmGenerator {
  
public:
  /*******************************
   * Constructors
   *******************************/
  // Constructor requires the number of unique alarm codes that could be generated
  AlarmGenerator(int numAlarmCodes);

  /*******************************
   * Event handling
   *******************************/
  // Define alarm event handler function type
  typedef void (*IntFuncPtr)(int);
  // Defines a function to call when an alarm occurs
  void onAlarm(volatile IntFuncPtr onAlarmFunction);
  // Defines a function to call when an alarm clears
  void onClear(volatile IntFuncPtr onClearFunction);

protected:
  // Called when triggering an alarm
  void triggerAlarm(int alarmCode);
  // Called when clearing an alarm
  void clearAlarm(int alarmCode);

  /*******************************
   * Member variables
   *******************************/
  // The array of alarm states
  bool* mAlarmStates;

  // The function to call if an alarm occurs
  static volatile IntFuncPtr mOnAlarmFunction;
  // The function to call if an alarm clear
  static volatile IntFuncPtr mOnClearFunction;

};

#endif // __ALARMGENERATOR_H_INCLUDED__
