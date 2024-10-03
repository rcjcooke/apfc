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
  
  // Depth above which runoff solution gets transferred back to the mixing tank
  static const int RUNOFF_SOLUTION_TANK_TRANSFER_DEPTH_MM = 50;
  // Minimum depth for the runoff solution tank
  static const int RUNOFF_SOLUTION_MIN_TANK_DEPTH_MM = 10;
  // The minimum depth available across the mixing and runoff solution tanks
  static const int IN_SYSTEM_SOLUTION_MIN_DEPTH_MM = 50;
  // The maximum depth that should be available across the mixing and runoff solution tanks
  static const int IN_SYSTEM_SOLUTION_MAX_DEPTH_MM = 90;

  // The minimum depth the irrigationsupply tank should be allowed to get to
  static const int IRRIGATIONSUPPLY_TANK_MIN_DEPTH_MM = 40;
  // The operating minimum depth of the irrigationsupply tank. If the depth goes below this figure, it should be topped up
  static const int IRRIGATIONSUPPLY_TANK_OPS_MIN_DEPTH_MM = 60;
  // The operating maximum depth of the irrigationsupply tank. This is the depth to which the tank should be topped up when necessary
  static const int IRRIGATIONSUPPLY_TANK_OPS_MAX_DEPTH_MM = 80;
  // At this depth, there's too much in the tank
  static const int WARNING_TANK_MAX_DEPTH_MM = 100;

}

/*******************************
 * Constructors
 *******************************/
SolutionTanksController::SolutionTanksController(
    uint8_t runoffSolutionPumpControlPin,
    uint8_t irrigationsupplyPumpControlPin, uint8_t uvControlPin, bool ledUVC,
    uint8_t runoffSolutionTankDepthModeSelectPin,
    uint8_t mixingTankDepthModeSelectPin,
    uint8_t irrigationsupplyTankDepthModeSelectPin,
    uint8_t runoffSolutionTankDepthSensorPin, uint8_t mixingTankDepthSensorPin,
    uint8_t irrigationsupplyTankDepthSensorPin)
    : AlarmGenerator('S', 7) {

  // Sort out the control pins
  mRunoffSolutionPumpControlPin = runoffSolutionPumpControlPin;
  mIrrigationSupplyPumpControlPin = irrigationsupplyPumpControlPin;
  mUVControlPin = uvControlPin;
  pinMode(mRunoffSolutionPumpControlPin, OUTPUT);
  pinMode(mIrrigationSupplyPumpControlPin, OUTPUT);
  pinMode(mUVControlPin, OUTPUT);

  // Set up the tank depth (distance) sensors
  mIrrigationSupplyTankDepthSensor = new A02YYUWDistanceSensor(irrigationsupplyTankDepthModeSelectPin, irrigationsupplyTankDepthSensorPin);
  mMixingTankDepthSensor = new A02YYUWDistanceSensor(mixingTankDepthModeSelectPin, mixingTankDepthSensorPin);
  mRunoffSolutionTankDepthSensor = new A02YYUWDistanceSensor(runoffSolutionTankDepthModeSelectPin, runoffSolutionTankDepthSensorPin);

  mLEDUVC = ledUVC;

  // If it's not an LED based UV light, then it needs to be on the whole time to avoid switching wear and warmup time
  if (!mLEDUVC) turnOnUVC();
  else turnOffUVC(); // Otherwise start with it off

  // Make sure all the pumps are off
  turnOffIrrigationSupplyPump();
  turnOffRunoffSolutionPump();
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the irrigationsupply tank pump is on
bool SolutionTanksController::isIrrigationSupplyPumpOn() const {
  return mIrrigationSupplyPumpOn;
}

// True when the runoff solution tank pump is on
bool SolutionTanksController::isRunoffSolutionPumpOn() const {
  return mRunoffSolutionPumpOn;
}

// True when the UV steriliser lamp is on
bool SolutionTanksController::isUVSteriliserOn() const {
  return mUVSteriliserOn;
}

// Get the current irrigationsupply tank depth / mm
double SolutionTanksController::getIrrigationSupplyTankDepth() {
  return mIrrigationSupplyTankDepth;
}

// Get the current mixing tank depth / mm
double SolutionTanksController::getMixingTankDepth() {
  return mMixingTankDepth;
}

// Get the current runoff solution tank depth / mm
double SolutionTanksController::getRunoffSolutionTankDepth() {
  return mRunoffSolutionTankDepth;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - moves solution between tanks as required
void SolutionTanksController::controlLoop() {

  // If true then a top up of the irrigationSupply tank is required
  static bool irrigationSupplyTopupRequested = false;

  // Read all the depths
  if (mRunoffSolutionTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_RUNOFF_SOLUTION_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }
  if (mMixingTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_MIXING_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }
  if (mIrrigationSupplyTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_IRRIGATIONSUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }

  mRunoffSolutionTankDepth = convertDistanceToDepth(mRunoffSolutionTankDepthSensor->getDistance());
  mMixingTankDepth = convertDistanceToDepth(mMixingTankDepthSensor->getDistance());
  mIrrigationSupplyTankDepth = convertDistanceToDepth(mIrrigationSupplyTankDepthSensor->getDistance());

  /* Pull from the irrigation supply tank. This drives everything else. */
  // If we're too low on supply then something's gone wrong so trigger the alarm
  if (mIrrigationSupplyTankDepth < SolutionTanksControllerNS::IRRIGATIONSUPPLY_TANK_MIN_DEPTH_MM) {
    triggerAlarm(ALARM_IRRIGATIONSUPPLY_TANK_TOO_LOW);
  } 
  if (mIrrigationSupplyTankDepth < SolutionTanksControllerNS::IRRIGATIONSUPPLY_TANK_OPS_MIN_DEPTH_MM) {
    // The irrigation supply is running low, it needs to be topped up with new mixed solution
    irrigationSupplyTopupRequested = true;
  } else if (mIrrigationSupplyTankDepth > SolutionTanksControllerNS::IRRIGATIONSUPPLY_TANK_OPS_MAX_DEPTH_MM) {
    // We've got enough solution in the tank for now, stop requesting it
    irrigationSupplyTopupRequested = false;
  }
  // If we've gone over the maximum depth for the tank, then something's gone wrong. Raise the alarm.
  if (mIrrigationSupplyTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_IRRIGATIONSUPPLY_TANK_OVER_FULL);
  } 

  /* V2.1 - no mixing tank, runoff goes direct to supply tank, all water still, so no sterilisation etc. */
  // If we need more solution and it's available, then top it up
  if (irrigationSupplyTopupRequested && mRunoffSolutionTankDepth > SolutionTanksControllerNS::RUNOFF_SOLUTION_MIN_TANK_DEPTH_MM) {
    turnOnRunoffSolutionPump();
  } else {
    turnOffRunoffSolutionPump();
  }

  // if the runoff tank has filled up excessively, then something's gone wrong. Raise the alarm
  if (mRunoffSolutionTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_RUNOFF_SOLUTION_TANK_OVER_FULL);
  }

  // if (mRunoffSolutionTankDepth > SolutionTanksControllerNS::RUNOFF_SOLUTION_TANK_TRANSFER_DEPTH_MM) {
  //   if (mLEDUVC) turnOnUVC();
  //   turnOnRunoffSolutionPump();
  // } else if (mRunoffSolutionTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
  //   triggerAlarm(ALARM_RUNOFF_SOLUTION_TANK_OVER_FULL);
  // } else if (mRunoffSolutionTankDepth < SolutionTanksControllerNS::RUNOFF_SOLUTION_MIN_TANK_DEPTH_MM) {
  //   turnOffRunoffSolutionPump();
  //   if (mLEDUVC) turnOffUVC();
  // }

  // int systemSolutionDepth = mMixingTankDepth + mRunoffSolutionTankDepth;
  // if (mMixingTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
  //   triggerAlarm(ALARM_MIXING_TANK_OVER_FULL);
  // }

  // if (systemSolutionDepth < SolutionTanksControllerNS::IN_SYSTEM_SOLUTION_MIN_DEPTH_MM) {
  //   turnOnIrrigationSupplyPump();
  // } else if (systemSolutionDepth < SolutionTanksControllerNS::IN_SYSTEM_SOLUTION_MAX_DEPTH_MM) {
  //   turnOffIrrigationSupplyPump();
  // }


}

// Turn on the Runoff Solution Tank Pump
void SolutionTanksController::turnOnRunoffSolutionPump() {
  if (!mRunoffSolutionPumpOn) digitalWrite(mRunoffSolutionPumpControlPin, HIGH);
  mRunoffSolutionPumpOn = true;
}

// Turn on the Runoff Solution Tank Pump
void SolutionTanksController::turnOffRunoffSolutionPump() {
  if (mRunoffSolutionPumpOn) digitalWrite(mRunoffSolutionPumpControlPin, LOW);
  mRunoffSolutionPumpOn = false;
}

// Turn on the IrrigationSupply Tank Pump
void SolutionTanksController::turnOnIrrigationSupplyPump() {
  if (!mIrrigationSupplyPumpOn) digitalWrite(mIrrigationSupplyPumpControlPin, HIGH);
  mIrrigationSupplyPumpOn = true;
}

// Turn off the IrrigationSupply Tank Pump
void SolutionTanksController::turnOffIrrigationSupplyPump() {
  if (mIrrigationSupplyPumpOn) digitalWrite(mIrrigationSupplyPumpControlPin, LOW);
  mIrrigationSupplyPumpOn = false;
}

// Turn on the UV Steriliser
void SolutionTanksController::turnOnUVC() {
  if (!mUVSteriliserOn) digitalWrite(mUVControlPin, HIGH);
  mUVSteriliserOn = true;
}

// Turn off the UV Steriliser
void SolutionTanksController::turnOffUVC() {
  if (mUVSteriliserOn) digitalWrite(mUVControlPin, LOW);
  mUVSteriliserOn = false;
}

// Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
void SolutionTanksController::emergencyStop() {
  turnOffRunoffSolutionPump();
  turnOffIrrigationSupplyPump();
  if (mLEDUVC) turnOffUVC();
}

/*******************************
 * Utilities
 *******************************/
// Converts a distance sensor reading into a tank depth
int SolutionTanksController::convertDistanceToDepth(int distance) {
  // Bound the distance as the sensor doesn't work under 30 mm - if it gets this close we're in trouble
  if (distance < 30) distance = 30;
  return SolutionTanksControllerNS::EMPTY_TANK_DISTANCE_MM - distance;
}
