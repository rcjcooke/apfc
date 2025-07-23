#include "StandardController.hpp"

using namespace apfc;

/*******************************
 * Constructors
 *******************************/
StandardController::StandardController(char uniqueAlarmGeneratorCode, int numUniqueAlarmCodes)
    : AlarmGenerator(uniqueAlarmGeneratorCode, numUniqueAlarmCodes) {}

/*******************************
 * Getters / Setters
 *******************************/
// Returns the current operating state of this controller
ControllerRunState StandardController::getRunState() const {
  return mRunState;
}

// Sets the current operating state of this controller
void StandardController::setRunState(ControllerRunState runState) {
  mRunState = runState;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - moves solution between tanks as required
void StandardController::controlLoop() {
  switch (getRunState()) {
    case ControllerRunState::STARTUP:
      startupStateLoop();
      break;
    case ControllerRunState::RUNNING:
      runningStateLoop();
      break;
    case ControllerRunState::EMERGENCY:
      emergencyStateLoop();
      break;
    default:
      // This shouldn't be possible - trigger alarm if here?
      break;
  }
}

// Trigger a transition to the emergency state with a reason
void StandardController::triggerEmergency(int reason) {
  triggerAlarm(reason);
  emergencyStop();
  setRunState(ControllerRunState::EMERGENCY);
}