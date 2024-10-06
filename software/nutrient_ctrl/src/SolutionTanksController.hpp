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
      uint8_t runoffRecyclingTankDepthModeSelectPin,
      uint8_t irrigationsupplyTankDepthModeSelectPin,
      uint8_t runoffRecyclingTankDepthSensorPin,
      uint8_t irrigationsupplyTankDepthSensorPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the runoff recycling tank pump is on
  bool isRunoffRecyclingPumpOn() const;
  // Get the current irrigationsupply tank depth / mm
  float getIrrigationSupplyTankDepth();
  // Get the current runoff recycling tank depth / mm
  float getRunoffRecyclingTankDepth();

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - moves solution between tanks as required
  void controlLoop();

  // Turn on the Runoff recycling Tank Pump
  void turnOnRunoffRecyclingPump();
  // Turn on the Runoff recycling Tank Pump
  void turnOffRunoffRecyclingPump();

private:
  // Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
  void emergencyStop();

  /*******************************
   * Utilities
   *******************************/
  // Converts a distance sensor reading into a tank depth
  float convertDistanceToDepth(float distance);

  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the runoff recycling pump state
  uint8_t mRunoffRecyclingPumpControlPin;

  // True when the runoff recycling tank pump is on
  bool mRunoffRecyclingPumpOn;
  // The current irrigationsupply tank depth / mm
  float mIrrigationSupplyTankDepth;
  // The current runoff recycling tank depth / mm
  float mRunoffRecyclingTankDepth;

  /** Depth sensors */
  // The irrigationsupply tank depth sensor
  A02YYUWDistanceSensor* mIrrigationSupplyTankDepthSensor;
  // The runoff recycling tank depth sensor
  A02YYUWDistanceSensor* mRunoffRecyclingTankDepthSensor;

};

#endif // __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
