#ifndef __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
#define __SOLUTIONTANKSCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "AlarmGenerator.hpp"
#include "A02YYUWDistanceSensor.hpp"

class SolutionTanksController : public AlarmGenerator {

public:

  /*******************************
   * Constants
   *******************************/
  // Alarm code triggered when used solution tank is too full
  static const int USED_SOLUTION_TANK_OVER_FULL = 1;
  // Alarm code triggered when mixing tank is too full
  static const int MIXING_TANK_OVER_FULL = 2;
  // Alarm code triggered when nutrient tank is too low
  static const int NUTRIENT_TANK_TOO_LOW = 3;

  // Alarm code triggered if there are communication problems with the used solution tank depth sensor
  static const int USED_SOLUTION_TANK_DEPTH_SENSOR_COMMS_ERROR = 4;
  // Alarm code triggered if there are communication problems with the mixing tank depth sensor
  static const int MIXING_TANK_DEPTH_SENSOR_COMMS_ERROR = 5;
  // Alarm code triggered if there are communication problems with the nutrient tank depth sensor
  static const int NUTRIENT_TANK_DEPTH_SENSOR_COMMS_ERROR = 6;

  /*******************************
   * Constructors
   *******************************/
  SolutionTanksController(uint8_t usedSolutionPumpControlPin, uint8_t nutrientPumpControlPin, 
                          uint8_t uvControlPin, bool ledUVC,
                          uint8_t usedSolutionTankDepthModeSelectPin, uint8_t mixingTankDepthModeSelectPin, uint8_t nutrientTankDepthModeSelectPin,
                          uint8_t usedSolutionTankDepthSensorPin, uint8_t mixingTankDepthSensorPin, uint8_t nutrientTankDepthSensorPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the nutrient tank pump is on
  bool isNutrientPumpOn() const;
  // True when the used solution tank pump is on
  bool isUsedSolutionPumpOn() const;
  // True when the UV steriliser lamp is on
  bool isUVSteriliserOn() const;
  // Get the current nutrient tank depth / mm
  double getNutrientTankDepth();
  // Get the current mixing tank depth / mm
  double getMixingTankDepth();
  // Get the current used solution tank depth / mm
  double getUsedSolutionTankDepth();

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - moves solution between tanks as required
  void controlLoop();

  // Turn on the Used Solution Tank Pump
  void turnOnUsedSolutionPump();
  // Turn on the Used Solution Tank Pump
  void turnOffUsedSolutionPump();
  // Turn on the Nutrient Tank Pump
  void turnOnNutrientPump();
  // Turn off the Nutrient Tank Pump
  void turnOffNutrientPump();
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
  // The microcontroller pin that controls the used solution pump state
  uint8_t mUsedSolutionPumpControlPin;
  // The microcontroller pin that controls the nutrient pump state
  uint8_t mNutrientPumpControlPin;
  // The microcontroller pin that controls the UV steriliser light state
  uint8_t mUVControlPin;

  // True if the UV Sterilisation light is LED-based (means regular switching is ok + extremely short switching time)
  bool mLEDUVC;

  // True when the nutrient tank pump is on
  bool mNutrientPumpOn;
  // True when the used solution tank pump is on
  bool mUsedSolutionPumpOn;
  // True when the UV steriliser lamp is on
  bool mUVSteriliserOn;
  // The current nutrient tank depth / mm
  double mNutrientTankDepth;
  // The current mixing tank depth / mm
  double mMixingTankDepth;
  // The current used solution tank depth / mm
  double mUsedSolutionTankDepth;

  /** Depth sensors */
  // The nutrient tank depth sensor
  A02YYUWDistanceSensor* mNutrientTankDepthSensor;
  // The mixing tank depth sensor
  A02YYUWDistanceSensor* mMixingTankDepthSensor;
  // The used solution tank depth sensor
  A02YYUWDistanceSensor* mUsedSolutionTankDepthSensor;

};

#endif // __SOLUTIONTANKSCONTROLLER_H_INCLUDED__
