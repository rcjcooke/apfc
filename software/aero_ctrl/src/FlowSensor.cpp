#include "FlowSensor.hpp"

// Number of millilitres per pulse on the flow sensor
const float ML_PER_PULSE = 0.438596;

/**
 * Singleton namespace defined to aggregate Flow Sensor global variables / methods for interrupts
 */
namespace FlowSensorNS {
  
  // The number of pulses received from the flow metre 
  unsigned long gFlowSensorPulseCount = 0;
  // The number of pulses received from the flow metre since the last reset
  unsigned long gFlowSensorPulseCountSinceReset = 0;

  // Flow Sensor Pulsed Interrupt Service Routine - called on every interrupt
  void flowSensorPulsedISR() {
    gFlowSensorPulseCount++;
    gFlowSensorPulseCountSinceReset++;
  } 

}

/*******************************
 * Constructors
 *******************************/
FlowSensor::FlowSensor(uint8_t flowSensorPin) {
  // Flowrate Meter Signal - PWM: F=(38*Q), Q=L / min
  mFlowSensorPin = flowSensorPin;
    // Set up Interrupts
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), FlowSensorNS::flowSensorPulsedISR, RISING);

}

/*******************************
 * Getters / Setters
 *******************************/
// Get the current flow rate in millilitres per second
double FlowSensor::getCurrentFlowRate() {
  return mCurrentFlowRate;
}

// Get the cumulative volume since the last reset / millilitres
double FlowSensor::getCumulativeVolumeMl() {
  return FlowSensorNS::gFlowSensorPulseCountSinceReset * ML_PER_PULSE;
}

// Get the current raw pulse count recorded from the sensor
unsigned long FlowSensor::getFlowSensorPulseCount() {
  return FlowSensorNS::gFlowSensorPulseCount;
}


/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - updates the flow rate and volumes
void FlowSensor::controlLoop() {
  
  static unsigned long lastCountPulses = 0, lastCalcMillis = 0; 
  
  unsigned long currentCountPulses = FlowSensorNS::gFlowSensorPulseCount;
  unsigned long currentMillis = millis();
  unsigned long deltaPulses = 0;
  unsigned long deltaTimeMillis = abs(currentMillis - lastCalcMillis);

  // Only calculated once every second
  if (deltaTimeMillis < 1000) return;

  // handle counter overflow
  if (currentCountPulses < lastCountPulses) {
    deltaPulses = lastCountPulses - currentCountPulses;
  } else {
    deltaPulses = currentCountPulses - lastCountPulses;
  }

  // Update the flow rate
  double volumeMl = ML_PER_PULSE * deltaPulses;
  mCurrentFlowRate = volumeMl / deltaTimeMillis * 1000;

  // Store the details for next time around the loop
  lastCalcMillis = currentMillis;
  lastCountPulses = currentCountPulses;

}

// Used to reset the cumulative volume counter
void FlowSensor::resetCumulativeVolume() {
  FlowSensorNS::gFlowSensorPulseCountSinceReset = 0;
}