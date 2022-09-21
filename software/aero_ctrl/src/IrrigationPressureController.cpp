#include "IrrigationPressureController.hpp"

/************************
 * Constants
 ************************/
namespace IrrigationPressureControllerNS {
  /* Sensor Calibration */
  // Maximum pressure the sensor detects / PSI
  const int MAX_PRESSURE = 150; 
}

/*******************************
 * Constructors
 *******************************/
// Basic constructor
IrrigationPressureController::IrrigationPressureController(uint8_t pressureSensorPin, uint8_t drainValveControlPin, uint8_t irrigationPumpControlPin) : AlarmGenerator('I', 1) {
  mPressureSensor = new PressureSensor(pressureSensorPin, IrrigationPressureControllerNS::MAX_PRESSURE);

  mDrainValveControlPin = drainValveControlPin;
  mIrrigationPumpControlPin = irrigationPumpControlPin;

  pinMode(mDrainValveControlPin, OUTPUT);
  pinMode(mIrrigationPumpControlPin, OUTPUT);

  turnOffIrrigationPump();
  closeDrainValve();
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the valve is open (i.e. draining is in progress)
bool IrrigationPressureController::isDrainValveOpen() const {
  return mDrainValveOpen;
};

// True when the irrigation pump is on (building pressure)
bool IrrigationPressureController::isIrrigationPumpOn() const {
  return mIrrigationPumpOn;
}

// Get the current irrigation system pressure / PSI
double IrrigationPressureController::getPressurePSI() const {
  return mPressureSensor->getPressurePSI();
}

// Get the PressureSensor - useful for debug purposes
PressureSensor* IrrigationPressureController::getPressureSensor() const {
  return mPressureSensor;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - controls the pressure in the irrigation system
void IrrigationPressureController::controlLoop() {
  
  // Get updated pressure readings
  mPressureSensor->readPressure();
  float pressurePSI = mPressureSensor->getPressurePSI();

  if (pressurePSI < 120) {
    clearAlarm(OVER_PRESSURE_ALARM); // Just in case it's been triggered
  }
  
  if (pressurePSI <= 80) {
    closeDrainValve(); // Just in case it's open
    turnOnIrrigationPump();
  } else if (pressurePSI >= 100) {
    // Target pressure reached
    turnOffIrrigationPump();
  } else if (pressurePSI >= 120) {
    /*
    Something's gone wrong and the pressure is still building, 
    open the drain valve to releave pressure and raise the alarm
    */
    openDrainValve();
    triggerAlarm(OVER_PRESSURE_ALARM);
  }
 
}

// Open drain valve
void IrrigationPressureController::openDrainValve() {
  digitalWrite(mDrainValveControlPin, HIGH);
  mDrainValveOpen = true;
}

// Close drain valve
void IrrigationPressureController::closeDrainValve() {
  digitalWrite(mDrainValveControlPin, LOW);
  mDrainValveOpen = false;
}

// Turn on the Irrigation Pump (builds up pressure)
void IrrigationPressureController::turnOnIrrigationPump() {
  digitalWrite(mIrrigationPumpControlPin, HIGH);
  mIrrigationPumpOn = true;
}

// Turn off the Irrigation Pump
void IrrigationPressureController::turnOffIrrigationPump() {
  digitalWrite(mIrrigationPumpControlPin, LOW);
  mIrrigationPumpOn = false;
}

