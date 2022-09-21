#include "SolutionTanksController.hpp"

/************************
 * Constants
 ************************/
namespace SolutionTanksControllerNS {

  // Mode select for processed values from depth sensor
  static const uint8_t PROCESSED = HIGH;
  // Mode select for real-time values from depth sensor
  static const uint8_t REALTIME = LOW;

  // The distance measured for a completely empty tank
  static const int EMPTY_TANK_DISTANCE_MM = 150;
  
  // Depth above which used solution gets transferred back to the mixing tank
  static const int USED_SOLUTION_TANK_TRANSFER_DEPTH_MM = 50;
  // Minimum depth for the used solution tank
  static const int USED_SOLUTION_MIN_TANK_DEPTH_MM = 10;
  // The minimum depth available across the mixing and used solution tanks
  static const int IN_SYSTEM_SOLUTION_MIN_DEPTH_MM = 50;
  // The maximum depth that should be available across the mixing and used solution tanks
  static const int IN_SYSTEM_SOLUTION_MAX_DEPTH_MM = 90;

  // The minimum depth the nutrient tank should be allowed to get to
  static const int NUTRIENT_TANK_MIN_DEPTH_MM = 40;
  // At this depth, there's too much in the tank
  static const int WARNING_TANK_MAX_DEPTH_MM = 100;

}

/*******************************
 * Constructors
 *******************************/
SolutionTanksController::SolutionTanksController(uint8_t usedSolutionPumpControlPin, uint8_t nutrientPumpControlPin, 
                        uint8_t uvControlPin, bool ledUVC,
                        uint8_t usedSolutionTankDepthModeSelectPin, uint8_t mixingTankDepthModeSelectPin, uint8_t nutrientTankDepthModeSelectPin,
                        uint8_t usedSolutionTankDepthSensorPin, uint8_t mixingTankDepthSensorPin, uint8_t nutrientTankDepthSensorPin) : AlarmGenerator('S', 6) {
  mUsedSolutionPumpControlPin = usedSolutionPumpControlPin;
  mNutrientPumpControlPin = nutrientPumpControlPin;
  mUVControlPin = uvControlPin;

  mNutrientTankDepthSensor = new A02YYUWDistanceSensor(nutrientTankDepthModeSelectPin, nutrientTankDepthSensorPin);
  mMixingTankDepthSensor = new A02YYUWDistanceSensor(mixingTankDepthModeSelectPin, mixingTankDepthSensorPin);
  mUsedSolutionTankDepthSensor = new A02YYUWDistanceSensor(usedSolutionTankDepthModeSelectPin, usedSolutionTankDepthSensorPin);

  mLEDUVC = ledUVC;

  // If it's not an LED based UV light, then it needs to be on the whole time to avoid switching wear and warmup time
  if (!mLEDUVC) turnOnUVC();
  else turnOffUVC(); // Otherwise start with it off

  // Make sure all the pumps are off
  turnOffNutrientPump();
  turnOffUsedSolutionPump();
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the nutrient tank pump is on
bool SolutionTanksController::isNutrientPumpOn() const {
  return mNutrientPumpOn;
}

// True when the used solution tank pump is on
bool SolutionTanksController::isUsedSolutionPumpOn() const {
  return mUsedSolutionPumpOn;
}

// True when the UV steriliser lamp is on
bool SolutionTanksController::isUVSteriliserOn() const {
  return mUVSteriliserOn;
}

// Get the current nutrient tank depth / mm
double SolutionTanksController::getNutrientTankDepth() {
  return mNutrientTankDepth;
}

// Get the current mixing tank depth / mm
double SolutionTanksController::getMixingTankDepth() {
  return mMixingTankDepth;
}

// Get the current used solution tank depth / mm
double SolutionTanksController::getUsedSolutionTankDepth() {
  return mUsedSolutionTankDepth;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - moves solution between tanks as required
void SolutionTanksController::controlLoop() {
  // Read all the depths
  if (mUsedSolutionTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_USED_SOLUTION_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }
  if (mMixingTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_MIXING_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }
  if (mNutrientTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_NUTRIENT_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }

  mUsedSolutionTankDepth = convertDistanceToDepth(mUsedSolutionTankDepthSensor->getDistance());
  mMixingTankDepth = convertDistanceToDepth(mMixingTankDepthSensor->getDistance());
  mNutrientTankDepth = convertDistanceToDepth(mNutrientTankDepthSensor->getDistance());

  if (mUsedSolutionTankDepth > SolutionTanksControllerNS::USED_SOLUTION_TANK_TRANSFER_DEPTH_MM) {
    if (mLEDUVC) turnOnUVC();
    turnOnUsedSolutionPump();
  } else if (mUsedSolutionTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_USED_SOLUTION_TANK_OVER_FULL);
  } else if (mUsedSolutionTankDepth < SolutionTanksControllerNS::USED_SOLUTION_MIN_TANK_DEPTH_MM) {
    turnOffUsedSolutionPump();
    if (mLEDUVC) turnOffUVC();
  }

  int systemSolutionDepth = mMixingTankDepth + mUsedSolutionTankDepth;
  if (mMixingTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_MIXING_TANK_OVER_FULL);
  }

  if (systemSolutionDepth < SolutionTanksControllerNS::IN_SYSTEM_SOLUTION_MIN_DEPTH_MM) {
    turnOnNutrientPump();
  } else if (systemSolutionDepth < SolutionTanksControllerNS::IN_SYSTEM_SOLUTION_MAX_DEPTH_MM) {
    turnOffNutrientPump();
  }

  if (mNutrientTankDepth < SolutionTanksControllerNS::NUTRIENT_TANK_MIN_DEPTH_MM) {
    triggerAlarm(ALARM_NUTRIENT_TANK_TOO_LOW);
  }

}

// Turn on the Used Solution Tank Pump
void SolutionTanksController::turnOnUsedSolutionPump() {
  digitalWrite(mUsedSolutionPumpControlPin, HIGH);
  mUsedSolutionPumpOn = true;
}

// Turn on the Used Solution Tank Pump
void SolutionTanksController::turnOffUsedSolutionPump() {
  digitalWrite(mUsedSolutionPumpControlPin, LOW);
  mUsedSolutionPumpOn = false;
}

// Turn on the Nutrient Tank Pump
void SolutionTanksController::turnOnNutrientPump() {
  digitalWrite(mNutrientPumpControlPin, HIGH);
  mNutrientPumpOn = true;
}

// Turn off the Nutrient Tank Pump
void SolutionTanksController::turnOffNutrientPump() {
  digitalWrite(mNutrientPumpControlPin, LOW);
  mNutrientPumpOn = false;
}

// Turn on the UV Steriliser
void SolutionTanksController::turnOnUVC() {
  digitalWrite(mUVControlPin, HIGH);
  mUVSteriliserOn = true;
}

// Turn off the UV Steriliser
void SolutionTanksController::turnOffUVC() {
  digitalWrite(mUVControlPin, LOW);
  mUVSteriliserOn = false;
}

// Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
void SolutionTanksController::emergencyStop() {
  turnOffUsedSolutionPump();
  turnOffNutrientPump();
  if (mLEDUVC) turnOffUVC();
}

/*******************************
 * Utilities
 *******************************/
// Converts a distance sensor reading into a tank depth
int convertDistanceToDepth(int distance) {
  // Bound the distance as the sensor doesn't work under 30 mm - if it gets this close we're in trouble
  if (distance < 30) distance = 30;
  return SolutionTanksControllerNS::EMPTY_TANK_DISTANCE_MM - distance;
}
