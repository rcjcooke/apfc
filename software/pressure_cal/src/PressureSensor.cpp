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
// Basic constructor
PressureSensor::PressureSensor(uint8_t pressureSensorPin) {
  mPressureSensorPin = pressureSensorPin;
  // Note: No need to set pin mode on pressure sensor pin - analogueRead does it for you
  mLastReadPressure = 0;
}

/*******************************
 * Getters / Setters
 *******************************/
// Get the current pressure / ?
double PressureSensor::getPressure() {
  return mLastReadPressure;
}

// Get the last raw ADC value recorded (useful for calibration purposes)
int PressureSensor::getLastRawADCValue() {
  return mLastRawADCValue;
}


// The value reported by the uC ADC at 0 Bar
void PressureSensor::setADCAt0Bar(int adcValue) {
  mADCAt0Bar = adcValue;
  calculateCoefficients();
}

// Get the uC ADC set point at 0 Bar
int PressureSensor::getADCAt0Bar() {
  return mADCAt0Bar;
}

// The value reported by the uC ADC at 8 Bar
void PressureSensor::setADCAt8Bar(int adcValue) {
  mADCAt8Bar = adcValue;
}

// Get the uC ADC set point at 8 Bar
int PressureSensor::getADCAt8Bar() {
  return mADCAt8Bar;
}


/*******************************
 * Actions
 *******************************/
// Reads the pressure from the sensor
void PressureSensor::readPressure() {
  // NOTE: Arduino Nano has 10-bit resolution ADCs = 0-1024

  // Start at the mid point and then remember the rolling value
  static float filteredValue = 512;
  
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
  mM = 8.0 / (float) (mADCAt8Bar - mADCAt0Bar);
  mC = -(mM * (float) mADCAt0Bar);
}
