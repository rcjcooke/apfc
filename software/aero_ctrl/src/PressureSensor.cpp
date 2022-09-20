#include "PressureSensor.hpp"

/************************
 * Constants
 ************************/
namespace PressureSensorNS {
  /* Low Pass Filter */
  // Proportion between 0-1 to bias towards 2 point rolling average (i.e. previous filtered value)
  const float ALPHA = 0.95;  
}

/*******************************
 * Constructors
 *******************************/
// Basic constructor - Assume perfect calibration
PressureSensor::PressureSensor(uint8_t pressureSensorPin, int maxPressurePSI) : PressureSensor(pressureSensorPin, 0, 0, 1024, maxPressurePSI) {}

PressureSensor::PressureSensor(uint8_t pressureSensorPin, int calADC1, int calP1, int calADC2, int calP2) {
  mPressureSensorPin = pressureSensorPin;
  mCalibrationPoint1[0] = calADC1;
  mCalibrationPoint1[1] = calP1;
  mCalibrationPoint2[0] = calADC2;
  mCalibrationPoint2[1] = calP2;
  calculateCoefficients();

  // Note: No need to set pin mode on pressure sensor pin - analogueRead does it for you
  mLastReadPressure = 0;
}


/*******************************
 * Getters / Setters
 *******************************/
// Get the current pressure / PSI
double PressureSensor::getPressurePSI() const {
  return mLastReadPressure;
}

// Get the last raw ADC value recorded (useful for calibration purposes)
int PressureSensor::getLastRawADCValue() const {
  return mLastRawADCValue;
}

// The value reported by the uC ADC at a specificed pressure point / PSI
void PressureSensor::setCalibationPoint1(int adcValue, int pressurePSI) {
  mCalibrationPoint1[0] = adcValue;
  mCalibrationPoint1[1] = pressurePSI;
  calculateCoefficients();
}

// Get the uC ADC set point at a specificed pressure in PSI = returns a two value array: [adc value, pressure / PSI]
int* PressureSensor::getCalibationPoint1() {
  return mCalibrationPoint1;
}

// The value reported by the uC ADC at a specificed pressure point / PSI
void PressureSensor::setCalibationPoint2(int adcValue, int pressurePSI) {
  mCalibrationPoint2[0] = adcValue;
  mCalibrationPoint2[1] = pressurePSI;
  calculateCoefficients();
}

// Get the uC ADC set point at a specificed pressure in PSI = returns a two value array: [adc value, pressure / PSI]
int* PressureSensor::getCalibationPoint2() {
  return mCalibrationPoint2;
}

/*******************************
 * Actions
 *******************************/
// Reads the pressure from the sensor
void PressureSensor::readPressure() {
  // NOTE: Arduino Nano has 10-bit resolution ADCs = 0-1024

  // Start at the mid point and then remember the rolling value
  static float filteredValue = 512.0;
  
  // Read the pressure as a raw ADC value
  mLastRawADCValue = analogRead(mPressureSensorPin);
  // Apply low pass filter
  filteredValue = (PressureSensorNS::ALPHA * filteredValue) + ((1-PressureSensorNS::ALPHA) * (float) mLastRawADCValue); // low pass filter to reduce noise
  // Calculate pressure from the filtered result
  mLastReadPressure = mM * filteredValue + mC;

}

/*******************************
 * Utilities
 *******************************/
// Given calibration values, work out and record what our co-oefficients should be
void PressureSensor::calculateCoefficients() {
  // Calculate C and M - basic simultaneous equations for 2 points on the same line
  // Note: calibrationPoint[0] = ADC value, calibrationPoint[1] = pressure in PSI
  mM = (float) (mCalibrationPoint2[1] - mCalibrationPoint1[1]) / (float) (mCalibrationPoint2[0] - mCalibrationPoint1[0]);
  mC = mCalibrationPoint1[1] - (mM * (float) mCalibrationPoint1[0]);
}
