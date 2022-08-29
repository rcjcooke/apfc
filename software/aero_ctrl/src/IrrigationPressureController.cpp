#include "IrrigationPressureController.hpp"

/************************
 * Constants
 ************************/
namespace IrrigationPressureControllerNS {
  /* Sensor Calibration */
  // Voltage produced at 0 PSI
  const float VOLTAGE_AT_0_PSI = 0.0;
  // Voltage produced at 100 PSI
  const float VOLTAGE_AT_100_PSI = 5.0;
  // Analogue Reference Voltage
  const float A_REF = 5.0;

  const float C = 0.0;

  const float M = 0.0;

  /* Low Pass Filter */
  // Proportion between 0-1 to bias towards 2 point rolling average (i.e. previous filtered value)
  const float ALPHA = 0.95;  
}

/*******************************
 * Constructors
 *******************************/
// Basic constructor
IrrigationPressureController::IrrigationPressureController(uint8_t pressureSensorPin, uint8_t drainValveControlPin, uint8_t irrigationPumpControlPin) : AlarmGenerator(1) {
  mPressureSensorPin = pressureSensorPin;
  mDrainValveControlPin = drainValveControlPin;
  mIrrigationPumpControlPin = irrigationPumpControlPin;
  // Note: No need to set pin mode on pressure sensor pin - analogueRead does it for you
  pinMode(mDrainValveControlPin, OUTPUT);
  pinMode(mIrrigationPumpControlPin, OUTPUT);

  turnOffIrrigationPump();
  closeDrainValve();
  mCurrentPressure = 0;
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

// Get the current irrigation system pressure / ?
double IrrigationPressureController::getPressure() {
  return mCurrentPressure;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - controls the pressure in the irrigation system
void IrrigationPressureController::controlLoop() {
  // NOTE: Arduino Nano has 10-bit resolution ADCs = 0-1024

  // Assume we're starting up within the "at pressure" region so no action is taken by default
  static float filteredValue = 512; // TODO: Work out what this should be for 90 PSI
  
  float rawValue = (float) analogRead(mPressureSensorPin);
  filteredValue = (IrrigationPressureControllerNS::ALPHA * filteredValue) + ((1-IrrigationPressureControllerNS::ALPHA) * rawValue); // low pass filter to reduce noise
  float voltage = (filteredValue * 1024.0) * IrrigationPressureControllerNS::A_REF;
  float pressurePSI = (voltage - IrrigationPressureControllerNS::C) / IrrigationPressureControllerNS::M; // TODO: Work out this equation once I've calibrated

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

