#ifndef __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__
#define __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__

#include <Arduino.h>

#include <AlarmGenerator.hpp>

#include "PressureSensor.hpp"

class IrrigationPressureController: public AlarmGenerator {

public:

  /*******************************
   * Constants
   *******************************/
  // Alarm code triggered when pressure rises over 120PSI (implies that pump is building pressure unexpectedly)
  static const int OVER_PRESSURE_ALARM = 0;

  /*******************************
   * Constructors
   *******************************/
  // TODO: Constructor doc
  // The pin on which the pressure is reported by the pressure sensor (analogue 0-5V ~= 0-150 PSI)
  IrrigationPressureController(uint8_t pressureSensorPin, uint8_t drainValveControlPin, uint8_t irrigationPumpControlPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the valve is open (i.e. draining is in progress)
  bool isDrainValveOpen() const;
  // True when the irrigation pump is on (building pressure)
  bool isIrrigationCompressorOn() const;
  // Get the current irrigation system pressure / PSI
  double getPressurePSI() const;
  // Get the PressureSensor - useful for debug purposes
  PressureSensor* getPressureSensor() const;

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - controls the pressure in the irrigation system
  void controlLoop();
  // Open drain valve
  void openDrainValve();
  // Close drain valve
  void closeDrainValve();
  // Turn on the Irrigation Pump (builds up pressure)
  void turnOnIrrigationCompressor();
  // Turn off the Irrigation Pump
  void turnOffIrrigationCompressor();

private:
  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the drain valve state
  uint8_t mDrainValveControlPin;
  // The microcontroller pin that controls the irrigation pump state
  uint8_t mIrrigationCompressorControlPin;
  // The pressure sensor
  PressureSensor* mPressureSensor;
  // True when the valve is open (i.e. pressure is being released from the system)
  bool mDrainValveOpen;
  // True when the irrigation pump is running
  bool mIrrigationCompressorOn;

};

#endif // __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__
