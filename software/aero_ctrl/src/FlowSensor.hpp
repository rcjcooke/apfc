#ifndef __FLOWSENSOR_H_INCLUDED__
#define __FLOWSENSOR_H_INCLUDED__

#include <Arduino.h>

class FlowSensor {

public:
  /*******************************
   * Constructors
   *******************************/
  FlowSensor(uint8_t flowSensorPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the current flow rate in millilitres per second
  double getCurrentFlowRate();
  // Get the cumulative volume since the last reset / millilitres
  double getCumulativeVolumeMl();
  // Get the current raw pulse count recorded from the sensor
  unsigned long getFlowSensorPulseCount();

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - updates the flow rate and volumes
  void controlLoop();
  // Used to reset the cumulative volume counter
  void resetCumulativeVolume();

private:
  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that we've got the flow sensor input pin connected to
  uint8_t mFlowSensorPin;
  // The current flow rate in millilitres per second
  double mCurrentFlowRate;

};

#endif // __FLOWSENSOR_H_INCLUDED__
