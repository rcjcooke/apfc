#ifndef __STANDARDCONTROLLER_H_INCLUDED__
#define __STANDARDCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SPI.h>

#include <AlarmGenerator.hpp>

namespace apfc {

  // Controller Run State
  enum class ControllerRunState {
    STARTUP,
    RUNNING,
    EMERGENCY
  };

  class StandardController : public AlarmGenerator {

  public:

    StandardController(char uniqueAlarmGeneratorCode, int numUniqueAlarmCodes);

    // Returns the current operating state of this controller
    ControllerRunState getRunState() const;
    // Main control loop for this controller
    void controlLoop();

  protected:

    // Sets the current operating state of this controller
    void setRunState(ControllerRunState runState);

    // Control loop during startup state
    virtual void startupStateLoop() = 0;
    // Control loop during running state
    virtual void runningStateLoop() = 0;
    // Control loop during emergency state
    virtual void emergencyStateLoop() = 0;

    // Trigger a transition to the emergency state with a reason
    void triggerEmergency(int reason);
    // Called when an emergency is triggered
    virtual void emergencyStop() = 0;

  private:

    // The current run state of this controller
    ControllerRunState mRunState = ControllerRunState::STARTUP;

  };

}

#endif // __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
