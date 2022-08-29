#include <Arduino.h>

#include "PressureSensor.hpp"
#include "SerialDebugger.hpp"

// The pressure sensor connected to pin A7
PressureSensor* gPSensor = new PressureSensor(A7);
// The serial output interface
SerialDebugger* mDebugger;

/*******************************
 * Processing functions
 *******************************/
// Processes some raw input and updates the value
bool getRawSerialInput(int &inputValue) {

  if (Serial.available() > 0) {
    int input=0;
    input = Serial.read();
    
    // Correct for terminals that pad out 7-bit ASCII to 8 bits with an extra high bit 
    // (like Putty - pretty sure it's because it's encoding as UTF-8) 
    if (input > 127) {
      input = input - 128;
    }

    // Cheat convert ASCII to decimal
    input = input - 48;
    inputValue = inputValue*10 + input;
    return true;
  } else {
    return false;
  }
}

/*******************************
 * Setup functions
 *******************************/
void setup() {
  // Note: this also starts the serial interface at a baud rate of 115200 bps
  mDebugger = new SerialDebugger();
}

/*******************************
 * Main loop
 *******************************/
void loop() {

  // Get the latest values
  gPSensor->readPressure();
  int rawValue = gPSensor->getLastRawADCValue();
  int pressure = gPSensor->getPressure();

  mDebugger->updateValue("Raw Sensor Value / bits", rawValue);
  mDebugger->updateValue("ADC At 0 Bar (press 0 to calibrate) / bits", gPSensor->getADCAt0Bar());
  mDebugger->updateValue("ADC At 8 Bar (press 8 to calibrate) / bits", gPSensor->getADCAt8Bar());
  mDebugger->updateValue("Pressure / Bar", pressure);
  mDebugger->printUpdate();

  // Get user input
  int inputValue=0;
  bool gotRequest = getRawSerialInput(inputValue);

  if (gotRequest) {
    switch (inputValue) {
    case 0:
      gPSensor->setADCAt0Bar(rawValue);
      break;
    case 8:
      gPSensor->setADCAt8Bar(rawValue);
      break;
    default:
      break;
    }
  }
}