#include <Arduino.h>

#include "PressureSensor.hpp"
#include "SerialDebugger.hpp"

// Valve control pin
static const uint8_t VALVE_PIN = 5;
// Valve closed (0 = closed, 1 = open)
bool gValveState = 0;

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

  // Set up valve
  gValveState = 0;
  pinMode(VALVE_PIN, OUTPUT);
  digitalWrite(VALVE_PIN, LOW);  
}

/*******************************
 * Main loop
 *******************************/
void loop() {

  // Get the latest values
  gPSensor->readPressure();
  int rawValue = gPSensor->getLastRawADCValue();

  mDebugger->updateValue("Raw Sensor Value / bits", rawValue);
  mDebugger->updateValue("ADC At 0 Bar (press 0 to calibrate) / bits", gPSensor->getADCAt0Bar());
  mDebugger->updateValue("ADC At 8 Bar (press 8 to calibrate) / bits", gPSensor->getADCAt8Bar());
  mDebugger->updateValue("Calculated Pressure / Bar", (float) gPSensor->getPressure());
  mDebugger->updateValue("Valve Open (press 1 to open, 2 to close)", gValveState);
  mDebugger->printUpdate();

  // Get user input
  int inputValue=0;
  bool gotRequest = getRawSerialInput(inputValue);

  if (gotRequest) {
    switch (inputValue) {
    case 0:
      gPSensor->setADCAt0Bar(rawValue);
      break;
    case 1:
      digitalWrite(VALVE_PIN, HIGH);
      gValveState = 1;
      break;
    case 2:
      digitalWrite(VALVE_PIN, LOW);
      gValveState = 0;
      break;
    case 8:
      gPSensor->setADCAt8Bar(rawValue);
      break;
    default:
      break;
    }
  }
}