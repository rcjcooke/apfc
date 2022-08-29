#ifndef __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__
#define __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__

#include <Arduino.h>

#include "AlarmGenerator.hpp"

class IrrigationPressureController: public AlarmGenerator {

public:

  /*******************************
   * Constants
   *******************************/
  // Alarm code triggered when pressure rises over 120PSI (implies that pump is building pressure unexpectedly)
  static const int OVER_PRESSURE_ALARM = 1;

  /*******************************
   * Constructors
   *******************************/
  IrrigationPressureController(uint8_t pressureSensorPin, uint8_t drainValveControlPin, uint8_t irrigationPumpControlPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the valve is open (i.e. draining is in progress)
  bool isDrainValveOpen() const;
  // True when the irrigation pump is on (building pressure)
  bool isIrrigationPumpOn() const;
  // Get the current irrigation system pressure / ?
  double getPressure();

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
  void turnOnIrrigationPump();
  // Turn off the Irrigation Pump
  void turnOffIrrigationPump();

private:
  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the drain valve state
  uint8_t mDrainValveControlPin;
  // The microcontroller pin that controls the irrigation pump state
  uint8_t mIrrigationPumpControlPin;
  // The pin on which the pressure is reported by the pressure sensor (analogue 0-5V ~= 0-150 PSI)
  uint8_t mPressureSensorPin;
  // True when the valve is open (i.e. pressure is being released from the system)
  bool mDrainValveOpen;
  // True when the irrigation pump is running
  bool mIrrigationPumpOn;
  // The current irrigation system pressure / ?
  double mCurrentPressure;

};

#endif // __IRRIGATIONPRESSURECONTROLLER_H_INCLUDED__
