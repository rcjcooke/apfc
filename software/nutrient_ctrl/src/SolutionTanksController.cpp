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
  static const float EMPTY_TANK_DISTANCE_MM = 150.0f;
  
  // Depth above which runoff recycling gets transferred back to the mixing tank
  static const float RUNOFF_RECYCLING_TANK_TRANSFER_DEPTH_MM = 50.0f;
  // Minimum depth for the runoff recycling tank
  static const float RUNOFF_RECYCLING_MIN_TANK_DEPTH_MM = 10.0f;
  // The minimum depth available across the mixing and runoff recycling tanks
  static const float IN_SYSTEM_SOLUTION_MIN_DEPTH_MM = 50.0f;
  // The maximum depth that should be available across the mixing and runoff recycling tanks
  static const float IN_SYSTEM_SOLUTION_MAX_DEPTH_MM = 90.0f;

  // The minimum depth the irrigationsupply tank should be allowed to get to
  static const float IRRIGATIONSUPPLY_TANK_MIN_DEPTH_MM = 40.0f;
  // The operating minimum depth of the irrigationsupply tank. If the depth goes below this figure, it should be topped up
  static const float IRRIGATIONSUPPLY_TANK_OPS_MIN_DEPTH_MM = 60.0f;
  // The operating maximum depth of the irrigationsupply tank. This is the depth to which the tank should be topped up when necessary
  static const float IRRIGATIONSUPPLY_TANK_OPS_MAX_DEPTH_MM = 80.0f;
  // At this depth, there's too much in the tank
  static const float WARNING_TANK_MAX_DEPTH_MM = 100.0f;

}

/*******************************
 * Constructors
 *******************************/
SolutionTanksController::SolutionTanksController(
    uint8_t runoffRecyclingPumpControlPin,
    uint8_t runoffRecyclingTankDepthModeSelectPin,
    uint8_t irrigationsupplyTankDepthModeSelectPin,
    uint8_t runoffRecyclingTankDepthSensorPin,
    uint8_t irrigationsupplyTankDepthSensorPin)
    : AlarmGenerator('S', 7) {

  // Sort out the control pins
  mRunoffRecyclingPumpControlPin = runoffRecyclingPumpControlPin;
  pinMode(mRunoffRecyclingPumpControlPin, OUTPUT);

  // Set up the tank depth (distance) sensors
  mIrrigationSupplyTankDepthSensor = new A02YYUWDistanceSensor(irrigationsupplyTankDepthModeSelectPin, irrigationsupplyTankDepthSensorPin, true);
  mRunoffRecyclingTankDepthSensor = new A02YYUWDistanceSensor(runoffRecyclingTankDepthModeSelectPin, runoffRecyclingTankDepthSensorPin, true);

  // Make sure all the pumps are off (WARNING: Don't use the standard on/off methods here)
  changeRunoffRecyclingPumpControlPin(LOW);
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the runoff recycling tank pump is on
bool SolutionTanksController::isRunoffRecyclingPumpOn() const {
  return mRunoffRecyclingPumpOn;
}

// Get the current irrigationsupply tank depth / mm
float SolutionTanksController::getIrrigationSupplyTankDepth() {
  return mIrrigationSupplyTankDepth;
}

// Get the current runoff recycling tank depth / mm
float SolutionTanksController::getRunoffRecyclingTankDepth() {
  return mRunoffRecyclingTankDepth;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - moves solution between tanks as required
void SolutionTanksController::controlLoop() {

  // If true then a top up of the irrigationSupply tank is required
  static bool irrigationSupplyTopupRequested = false;

  // Read all the depths
  if (mRunoffRecyclingTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_RUNOFF_RECYCLING_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }
  if (mIrrigationSupplyTankDepthSensor->readDistance() != 0) {
    triggerAlarm(ALARM_IRRIGATIONSUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR);
    emergencyStop();
    return;
  }

  mRunoffRecyclingTankDepth = convertDistanceToDepth(mRunoffRecyclingTankDepthSensor->getDistance());
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
  if (irrigationSupplyTopupRequested && (mRunoffRecyclingTankDepth > SolutionTanksControllerNS::RUNOFF_RECYCLING_MIN_TANK_DEPTH_MM)) {
    turnOnRunoffRecyclingPump();
  } else {
    turnOffRunoffRecyclingPump();
  }

  // If the runoff tank has filled up excessively, then something's gone wrong. Raise the alarm
  if (mRunoffRecyclingTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_RUNOFF_RECYCLING_TANK_OVER_FULL);
  }

}

// Turn on the Runoff recycling Tank Pump
void SolutionTanksController::turnOnRunoffRecyclingPump() {
  if (!mRunoffRecyclingPumpOn) changeRunoffRecyclingPumpControlPin(HIGH);
  mRunoffRecyclingPumpOn = true;
}

// Turn on the Runoff recycling Tank Pump
void SolutionTanksController::turnOffRunoffRecyclingPump() {
  if (mRunoffRecyclingPumpOn) changeRunoffRecyclingPumpControlPin(LOW);
  mRunoffRecyclingPumpOn = false;
}

// Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
void SolutionTanksController::emergencyStop() {
  turnOffRunoffRecyclingPump();
}

void SolutionTanksController::changeRunoffRecyclingPumpControlPin(uint8_t state) {
  digitalWrite(mRunoffRecyclingPumpControlPin, state);
}

/*******************************
 * Utilities
 *******************************/
// Converts a distance sensor reading into a tank depth
float SolutionTanksController::convertDistanceToDepth(float distance) {
  // Bound the distance as the sensor doesn't work under 30 mm - if it gets this close we're in trouble
  if (distance < 30.0f) distance = 30.0f;
  return SolutionTanksControllerNS::EMPTY_TANK_DISTANCE_MM - distance;
}
