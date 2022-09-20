#ifndef __PRESSURESENSOR_H_INCLUDED__
#define __PRESSURESENSOR_H_INCLUDED__

#include <Arduino.h>

/**
 * Writen for an unknown brand industrial brand pressure sensor with a range of 0-150 PSI
 */
class PressureSensor {

public:

  /*******************************
   * Constructors
   *******************************/
  PressureSensor(uint8_t pressureSensorPin, int maxPressurePSI);
  PressureSensor(uint8_t pressureSensorPin, int calADC1, int calP1, int calADC2, int calP2);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last read pressure / PSI
  double getPressurePSI() const;
  // Get the last raw ADC value recorded (useful for calibration purposes)
  int getLastRawADCValue() const;
  // The value reported by the uC ADC at a specificed pressure point / PSI
  void setCalibationPoint1(int adcValue, int pressurePSI);
  // Get the uC ADC set point at a specificed pressure in PSI = returns a two value array: [adc value, pressure / PSI]
  int* getCalibationPoint1();
  // The value reported by the uC ADC at a specificed pressure point / PSI
  void setCalibationPoint2(int adcValue, int pressurePSI);
  // Get the uC ADC set point at a specificed pressure in PSI = returns a two value array: [adc value, pressure / PSI]
  int* getCalibationPoint2();

  /*******************************
   * Actions
   *******************************/
  // Reads the pressure from the sensor
  void readPressure();

private:

  /*******************************
   * Utilities
   *******************************/
  // Given calibration values, work out and record what our co-oefficients should be
  void calculateCoefficients();

  /*******************************
   * Member variables
   *******************************/
  // The pin on which the pressure is reported by the pressure sensor (analogue 0-5V ~= 0-150 PSI)
  uint8_t mPressureSensorPin;
  // The last recorded pressure / Bar
  double mLastReadPressure;
  // The last raw ADC value recorded (useful for calibration purposes)
  int mLastRawADCValue;

  /* Calibration values */
  // Calibation Point 1: The uC ADC set point at a specificed pressure in PSI as a two value array: [adc value, pressure / PSI]
  int mCalibrationPoint1[2] {0, 0};
  // Calibation Point 1: The uC ADC set point at a specificed pressure in PSI as a two value array: [adc value, pressure / PSI]
  int mCalibrationPoint2[2] {0, 0};
  // Pressure calculation coefficients
  float mM, mC;

};

#endif // __PRESSURESENSOR_H_INCLUDED__
