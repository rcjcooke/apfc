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
  PressureSensor(uint8_t pressureSensorPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last read pressure / Bar
  double getPressure();
  // Get the last raw ADC value recorded (useful for calibration purposes)
  int getLastRawADCValue();
  // The value reported by the uC ADC at 0 Bar
  void setADCAt0Bar(int adcValue);
  // Get the uC ADC set point at 0 Bar
  int getADCAt0Bar();
  // The value reported by the uC ADC at 8 Bar
  void setADCAt8Bar(int adcValue);
  // Get the uC ADC set point at 8 Bar
  int getADCAt8Bar();

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
  // The value reported by the uC ADC at 0 Bar - default to 0
  int mADCAt0Bar = 0;
  // The value reported by the uC ADC at 8 Bar - default to 794 (1024 @ 150 PSI)
  int mADCAt8Bar = 794;
  // Pressure calculation coefficients
  float mM, mC;

};

#endif // __PRESSURESENSOR_H_INCLUDED__
