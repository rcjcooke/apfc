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
    uint8_t multiUART1CSPin,
    char irrigationSupplyTankDepthSensorMUARTIndex,
    char runoffRecyclingTankDepthSensorMUARTIndex,
    uint8_t runoffRecyclingTankDepthModeSelectPin,
    uint8_t irrigationsupplyTankDepthModeSelectPin,
    uint8_t uvControlPin, bool ledUVC) : AlarmGenerator('S', 7) {

  // Sort out the control pins
  mRunoffRecyclingPumpControlPin = runoffRecyclingPumpControlPin;
  pinMode(mRunoffRecyclingPumpControlPin, OUTPUT);
  mUVControlPin = uvControlPin;
  pinMode(mUVControlPin, OUTPUT);

  mLEDUVC = ledUVC;
  // If it's not an LED based UV light, then it needs to be on the whole time to avoid switching wear and warmup time
  // WARNING: Don't use the standard on/off methods here in case the member variables are incorrectly or not initialised. 
  if (!mLEDUVC) {changeUVControlPinState(HIGH); mUVSteriliserOn = true;}
  else {changeUVControlPinState(LOW); mUVSteriliserOn = false;}

  // Sort out the serial interfaces
  MULTIUART* multiuart = new MULTIUART(multiUART1CSPin);

  //SPI Prescaler Options
  //SPI_CLOCK_DIV4 / SPI_CLOCK_DIV16 / SPI_CLOCK_DIV64
  //SPI_CLOCK_DIV128 / SPI_CLOCK_DIV2 / SPI_CLOCK_DIV8 / SPI_CLOCK_DIV32
  multiuart->initialise(SPI_CLOCK_DIV64);

  // Set up the tank depth (distance) sensors
  mIrrigationSupplyTankDepthSensor = new A02YYUW::A02YYUWviaUARTStream(
    new MUARTSingleStream(multiuart, irrigationSupplyTankDepthSensorMUARTIndex), 
    irrigationsupplyTankDepthModeSelectPin, 
    true);
  mRunoffRecyclingTankDepthSensor = new A02YYUW::A02YYUWviaUARTStream(
    new MUARTSingleStream(multiuart, runoffRecyclingTankDepthSensorMUARTIndex), 
    runoffRecyclingTankDepthModeSelectPin, 
    true);

  // Make sure all the pumps are off (WARNING: Don't use the standard on/off methods here)
  changeRunoffRecyclingPumpControlPinState(LOW);
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the runoff recycling tank pump is on
bool SolutionTanksController::isRunoffRecyclingPumpOn() const {
  return mRunoffRecyclingPumpOn;
}

// True when the UV steriliser lamp is on
bool SolutionTanksController::isUVSteriliserOn() const {
  return mUVSteriliserOn;
}

// Get the current irrigationsupply tank depth / mm
float SolutionTanksController::getIrrigationSupplyTankDepth() const {
  return mIrrigationSupplyTankDepth;
}

// Get the current runoff recycling tank depth / mm
float SolutionTanksController::getRunoffRecyclingTankDepth() const {
  return mRunoffRecyclingTankDepth;
}

// Returns the current operating state of this controller
STCRunState SolutionTanksController::getRunState() const {
  return mRunState;
}

// For Debug purposes: Get the depth sensor instance for the Irrigation Supply Tank
A02YYUW::A02YYUWviaUARTStream* SolutionTanksController::getIrrigationSupplyTankDepthSensor() {
  return mIrrigationSupplyTankDepthSensor;
}

// For Debug purposes: Get the depth sensor instance for the Irrigation Supply Tank
A02YYUW::A02YYUWviaUARTStream* SolutionTanksController::getRunoffRecyclingTankDepthSensor() {
  return mRunoffRecyclingTankDepthSensor;
}

/*******************************
 * Actions
 *******************************/
// Control loop during startup state
void SolutionTanksController::startupStateLoop() {

  // TODO: Introduce timeout check to handle faulty sensors preventing startup

  // Get readings
  mRunoffRecyclingTankDepthSensor->readDistance();
  mIrrigationSupplyTankDepthSensor->readDistance();

  // Keep cycling until we get consistent valid readings from all depth sensors - this deals with buffering and comms protocol sychronisation concerns
  if (checkLast5DepthSensorReadings(mRunoffRecyclingTankDepthSensor, true)) return;
  if (checkLast5DepthSensorReadings(mIrrigationSupplyTankDepthSensor, true)) return;

  // If we've got this far then transition to a running state
  mRunState = STCRunState::RUNNING;

}

// Control loop during running state
void SolutionTanksController::runningStateLoop() {

  // If true then a top up of the irrigationSupply tank is required
  static bool irrigationSupplyTopupRequested = false;

  // Read all the depths
  if (mRunoffRecyclingTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (checkLast5DepthSensorReadings(mRunoffRecyclingTankDepthSensor, false)) {
      triggerEmergency(ALARM_RUNOFF_RECYCLING_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return;
    }
  }
  if (mIrrigationSupplyTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (checkLast5DepthSensorReadings(mRunoffRecyclingTankDepthSensor, false)) {
      triggerEmergency(ALARM_IRRIGATIONSUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return;
    }
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
    if (mLEDUVC) turnOnUV();
    turnOnRunoffRecyclingPump();
  } else {
    turnOffRunoffRecyclingPump();
    if (mLEDUVC) turnOffUV();
  }

  // If the runoff tank has filled up excessively, then something's gone wrong. Raise the alarm
  if (mRunoffRecyclingTankDepth > SolutionTanksControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerAlarm(ALARM_RUNOFF_RECYCLING_TANK_OVER_FULL);
  }

}

// Control loop during emergency state
void SolutionTanksController::emergencyStateLoop() {
  // Read all the depths
  mRunoffRecyclingTankDepthSensor->readDistance();
  mIrrigationSupplyTankDepthSensor->readDistance();

  // TODO
}

// Called every microcontroller main program loop - moves solution between tanks as required
void SolutionTanksController::controlLoop() {
  switch (mRunState) {
    case STCRunState::STARTUP:
      startupStateLoop();
      break;
    case STCRunState::RUNNING:
      runningStateLoop();
      break;
    case STCRunState::EMERGENCY:
      emergencyStateLoop();
      break;
    default:
      // This shouldn't be possible - trigger alarm if here?
      break;
  }
}

// Turn on the Runoff recycling Tank Pump
void SolutionTanksController::turnOnRunoffRecyclingPump() {
  if (!mRunoffRecyclingPumpOn) changeRunoffRecyclingPumpControlPinState(HIGH);
  mRunoffRecyclingPumpOn = true;
}

// Turn on the Runoff recycling Tank Pump
void SolutionTanksController::turnOffRunoffRecyclingPump() {
  if (mRunoffRecyclingPumpOn) changeRunoffRecyclingPumpControlPinState(LOW);
  mRunoffRecyclingPumpOn = false;
}

void SolutionTanksController::changeRunoffRecyclingPumpControlPinState(uint8_t state) {
  digitalWrite(mRunoffRecyclingPumpControlPin, state);
}

// Trigger a transition to the emergency state with a reason
void SolutionTanksController::triggerEmergency(int reason) {
  triggerAlarm(reason);
  emergencyStop();
  mRunState = STCRunState::EMERGENCY;
}

// Called internally in the event of a sensor communication error - if we don't know how deep the tanks are, we shouldn't be moving anything around
void SolutionTanksController::emergencyStop() {
  turnOffRunoffRecyclingPump();
  if (mLEDUVC) turnOffUV();
}

// Turn on the UV Steriliser
void SolutionTanksController::turnOnUV() {
  if (!mUVSteriliserOn) changeUVControlPinState(HIGH);
  mUVSteriliserOn = true;
}

// Turn off the UV Steriliser
void SolutionTanksController::turnOffUV() {
  if (mUVSteriliserOn) changeUVControlPinState(LOW);
  mUVSteriliserOn = false;
}

void SolutionTanksController::changeUVControlPinState(uint8_t state) {
  digitalWrite(mUVControlPin, state);
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

/* Checks the last 5 readings from the sensor. If (allGood) then returns true
 * if all readings are good. If (!allGood) then returns true if every reading
 * is bad. */
bool SolutionTanksController::checkLast5DepthSensorReadings(A02YYUW::A02YYUWviaUARTStream *sensor, bool allGood) {
  int lastResults[5];
  sensor->getLast5ReadResults(lastResults);
  if (allGood) {
    int total = 0;
    for (size_t i = 0; i < 5; i++) {
      total+=lastResults[i];
    }
    return (total == 0);
  } else {
    for (size_t i = 0; i < 5; i++) {
      if (lastResults[i] == 0) return false;
    }
    return true;
  }
}