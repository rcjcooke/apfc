#ifndef __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
#define __SOLUTIONTANKSCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SoftwareSerial.h>

#include <AlarmGenerator.hpp>
#include <A02YYUWDistanceSensor.hpp>

class SolutionTanksController : public AlarmGenerator {

public:

  /*******************************
   * Constants
   *******************************/
  // Alarm code triggered when runoff solution tank is too full
  static const int ALARM_RUNOFF_SOLUTION_TANK_OVER_FULL = 0;
  // Alarm code triggered when mixing tank is too full
  static const int ALARM_MIXING_TANK_OVER_FULL = 1;
  // Alarm code triggered when irrigationsupply tank is too low
  static const int ALARM_IRRIGATIONSUPPLY_TANK_TOO_LOW = 2;
  // Alarm code triggered when irrigationsupply tank is too low
  static const int ALARM_IRRIGATIONSUPPLY_TANK_OVER_FULL = 3;

  // Alarm code triggered if there are communication problems with the runoff solution tank depth sensor
  static const int ALARM_RUNOFF_SOLUTION_TANK_DEPTH_SENSOR_COMMS_ERROR = 4;
  // Alarm code triggered if there are communication problems with the mixing tank depth sensor
  static const int ALARM_MIXING_TANK_DEPTH_SENSOR_COMMS_ERROR = 5;
  // Alarm code triggered if there are communication problems with the irrigationsupply tank depth sensor
  static const int ALARM_IRRIGATIONSUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR = 6;

  /*******************************
   * Constructors
   *******************************/
  SolutionTanksController(uint8_t runoffSolutionPumpControlPin, uint8_t irrigationsupplyPumpControlPin, 
                          uint8_t uvControlPin, bool ledUVC,
                          uint8_t runoffSolutionTankDepthModeSelectPin, uint8_t mixingTankDepthModeSelectPin, uint8_t irrigationsupplyTankDepthModeSelectPin,
                          uint8_t runoffSolutionTankDepthSensorPin, uint8_t mixingTankDepthSensorPin, uint8_t irrigationsupplyTankDepthSensorPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the irrigationsupply tank pump is on
  bool isIrrigationSupplyPumpOn() const;
  // True when the runoff solution tank pump is on
  bool isRunoffSolutionPumpOn() const;
  // True when the UV steriliser lamp is on
  bool isUVSteriliserOn() const;
  // Get the current irrigationsupply tank depth / mm
  double getIrrigationSupplyTankDepth();
  // Get the current mixing tank depth / mm
  double getMixingTankDepth();
  // Get the current runoff solution tank depth / mm
  double getRunoffSolutionTankDepth();

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - moves solution between tanks as required
  void controlLoop();

  // Turn on the Runoff Solution Tank Pump
  void turnOnRunoffSolutionPump();
  // Turn on the Runoff Solution Tank Pump
  void turnOffRunoffSolutionPump();
  // Turn on the IrrigationSupply Tank Pump
  void turnOnIrrigationSupplyPump();
  // Turn off the IrrigationSupply Tank Pump
  void turnOffIrrigationSupplyPump();
  // Turn on the UV Steriliser
  void turnOnUVC();
  // Turn off the UV Steriliser
  void turnOffUVC();

private:
  // Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
  void emergencyStop();

  /*******************************
   * Utilities
   *******************************/
  // Converts a distance sensor reading into a tank depth
  int convertDistanceToDepth(int distance);

  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the runoff solution pump state
  uint8_t mRunoffSolutionPumpControlPin;
  // The microcontroller pin that controls the irrigationsupply pump state
  uint8_t mIrrigationSupplyPumpControlPin;
  // The microcontroller pin that controls the UV steriliser light state
  uint8_t mUVControlPin;

  // True if the UV Sterilisation light is LED-based (means regular switching is ok + extremely short switching time)
  bool mLEDUVC;

  // True when the irrigationsupply tank pump is on
  bool mIrrigationSupplyPumpOn;
  // True when the runoff solution tank pump is on
  bool mRunoffSolutionPumpOn;
  // True when the UV steriliser lamp is on
  bool mUVSteriliserOn;
  // The current irrigationsupply tank depth / mm
  double mIrrigationSupplyTankDepth;
  // The current mixing tank depth / mm
  double mMixingTankDepth;
  // The current runoff solution tank depth / mm
  double mRunoffSolutionTankDepth;

  /** Depth sensors */
  // The irrigationsupply tank depth sensor
  A02YYUWDistanceSensor* mIrrigationSupplyTankDepthSensor;
  // The mixing tank depth sensor
  A02YYUWDistanceSensor* mMixingTankDepthSensor;
  // The runoff solution tank depth sensor
  A02YYUWDistanceSensor* mRunoffSolutionTankDepthSensor;

};

#endif // __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
