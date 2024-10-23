#ifndef __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
#define __SOLUTIONTANKSCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SPI.h>

#include <AlarmGenerator.hpp>
#include <MULTIUART.hpp>
#include <MUARTSingleStream.hpp>
#include <A02YYUWviaUARTStream.hpp>

// SolutionTanksController Run State
enum class STCRunState {
  STARTUP,
  RUNNING,
  EMERGENCY
};

class SolutionTanksController : public AlarmGenerator {

public:

  /*******************************
   * Constants
   *******************************/
  // Alarm code triggered when runoff recycling tank is too full
  static const int ALARM_RUNOFF_RECYCLING_TANK_OVER_FULL = 0;
  // Alarm code triggered when irrigationsupply tank is too low
  static const int ALARM_IRRIGATIONSUPPLY_TANK_TOO_LOW = 2;
  // Alarm code triggered when irrigationsupply tank is too low
  static const int ALARM_IRRIGATIONSUPPLY_TANK_OVER_FULL = 3;

  // Alarm code triggered if there are communication problems with the runoff recycling tank depth sensor
  static const int ALARM_RUNOFF_RECYCLING_TANK_DEPTH_SENSOR_COMMS_ERROR = 4;
  // Alarm code triggered if there are communication problems with the irrigationsupply tank depth sensor
  static const int ALARM_IRRIGATIONSUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR = 6;

  /*******************************
   * Constructors
   *******************************/
  SolutionTanksController(
      uint8_t runoffRecyclingPumpControlPin,
      uint8_t multiUART1CSPin,
      char irrigationSupplyTankDepthSensorMUARTIndex,
      char runoffRecyclingTankDepthSensorMUARTIndex,
      uint8_t runoffRecyclingTankDepthModeSelectPin,
      uint8_t irrigationsupplyTankDepthModeSelectPin,
      uint8_t uvControlPin, bool ledUVC);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the runoff recycling tank pump is on
  bool isRunoffRecyclingPumpOn() const;
  // True when the UV steriliser lamp is on
  bool isUVSteriliserOn() const;
  // Get the current irrigationsupply tank depth / mm
  float getIrrigationSupplyTankDepth() const;
  // Get the current runoff recycling tank depth / mm
  float getRunoffRecyclingTankDepth() const;
  // Returns the current operating state of this controller
  STCRunState getRunState() const;
  // For Debug purposes: Get the depth sensor instance for the Irrigation Supply Tank
  A02YYUW::A02YYUWviaUARTStream* getIrrigationSupplyTankDepthSensor();
  // For Debug purposes: Get the depth sensor instance for the Irrigation Supply Tank
  A02YYUW::A02YYUWviaUARTStream* getRunoffRecyclingTankDepthSensor();

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - moves solution between tanks as required
  void controlLoop();

  // Turn on the Runoff recycling Tank Pump
  void turnOnRunoffRecyclingPump();
  // Turn on the Runoff recycling Tank Pump
  void turnOffRunoffRecyclingPump();
  // Turn on the UV Steriliser
  void turnOnUV();
  // Turn off the UV Steriliser
  void turnOffUV();

private:

  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the runoff recycling pump state
  uint8_t mRunoffRecyclingPumpControlPin;
  // The microcontroller pin that controls the UV steriliser light state
  uint8_t mUVControlPin;

  // True if the UV Sterilisation light is LED-based (means regular switching is ok + extremely short switching time)
  bool mLEDUVC = false;
  // True when the UV steriliser lamp is on
  bool mUVSteriliserOn = false;

  // True when the runoff recycling tank pump is on
  bool mRunoffRecyclingPumpOn = false;
  // The current irrigationsupply tank depth / mm
  float mIrrigationSupplyTankDepth = 0.0f;
  // The current runoff recycling tank depth / mm
  float mRunoffRecyclingTankDepth = 0.0f;

  /** Depth sensors */
  // The irrigationsupply tank depth sensor
  A02YYUW::A02YYUWviaUARTStream* mIrrigationSupplyTankDepthSensor;
  // The runoff recycling tank depth sensor
  A02YYUW::A02YYUWviaUARTStream* mRunoffRecyclingTankDepthSensor;
  
  // The current run state of this controller
  STCRunState mRunState = STCRunState::STARTUP;

  /*******************************
   * Actions
   *******************************/
  // Control loop during startup state
  void startupStateLoop();
  // Control loop during running state
  void runningStateLoop();
  // Control loop during emergency state
  void emergencyStateLoop();

  // Trigger a transition to the emergency state with a reason
  void triggerEmergency(int reason);
  /* Called internally in the event of a sensor communication error - if we
   * don't know how deep the tanks are, we shouldn't be moving anything around
   */
  void emergencyStop();
  // Change state of the Runoff recycling Tank Pump
  void changeRunoffRecyclingPumpControlPinState(uint8_t state);
  // Change state of the UV Steriliser
  void changeUVControlPinState(uint8_t state);

  /*******************************
   * Utilities
   *******************************/
  // Converts a distance sensor reading into a tank depth
  float convertDistanceToDepth(float distance);
  /* Checks the last 5 readings from the sensor. If (allGood) then returns true
   * if all readings are good. If (!allGood) then returns true if every reading
   * is bad. */
  bool checkLast5DepthSensorReadings(A02YYUW::A02YYUWviaUARTStream *sensor, bool allGood);

};

#endif // __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
