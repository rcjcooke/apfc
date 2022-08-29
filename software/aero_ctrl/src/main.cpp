#include <Arduino.h>

#include "SprayController.hpp"
#include "FlowSensor.hpp"
#include "IrrigationPressureController.hpp"
#include "SolutionTanksController.hpp"

// Set to true if the UV Steriliser light is LED based - if LED it will turn the light off when not needed to save power
#define LED_UVC false
// If true, serial output from the Arduino is in human readable form - this means comms with the Raspberry Pi controller won't work
#define DEBUG_SOLO true

/************************
 * Constants
 ************************/
// Solenoid valve 1 control pin
static const uint8_t SV1CTL = 6;
// Solenoid valve 2 control pin
static const uint8_t SV2CTL = 7;
// Tank 1 pump control pin
static const uint8_t TP1CTL = 8;
// Tank 2 pump control pin
static const uint8_t TP2CTL = 9;
// Irrigation Pump Control pin
static const uint8_t IPCTL = 12;
// UV Sterilisation Lamp Control pin
static const uint8_t UVLCTL = 11;
// Flowrate Meter Signal - PWM: F=(38*Q), Q=L / min
static const uint8_t FM1S = A0;
// Pressure Sensor Signal - Analogue: 0-5VDC range
static const uint8_t PS1S = A7;
// Tank Depth Sensor 1: Processed Value/Real-time Value Output Selection
static const uint8_t DS1RX = 2;
// Tank Depth Sensor 1: UART TX: Depth data
static const uint8_t DS1TX = 3;
// Tank Depth Sensor 2: Processed Value/Real-time Value Output Selection
static const uint8_t DS2RX = 4;
// Tank Depth Sensor 2: UART TX: Depth data
static const uint8_t DS2TX = 5;
// Tank Depth Sensor 3: Processed Value/Real-time Value Output Selection
static const uint8_t DS3RX = A5;
// Tank Depth Sensor 3: UART TX: Depth data
static const uint8_t DS3TX = A4;

/************************
 * Variables
 ************************/

// Irrigation spray controller
SprayController* gSprayController;
// Irrigation spray flow sensor
FlowSensor* gFlowSensor;
// Irrigation pressure controller
IrrigationPressureController* gIrrigationPressureController;
// Controls the movement of solution between tanks
SolutionTanksController* gSolutionTanksController;

// Last spray volume / millilitres
double gLastSprayVolume = 0;

/*********************
 * Entry point methods
 *********************/
void setup() {

  gSprayController = new SprayController(SV1CTL);
  gFlowSensor = new FlowSensor(FM1S);
  gIrrigationPressureController = new IrrigationPressureController(PS1S, SV2CTL, IPCTL);
  gSolutionTanksController = new SolutionTanksController(TP1CTL, TP2CTL, UVLCTL, LED_UVC, DS1RX, DS2RX, DS3RX, DS1TX, DS2TX, DS3TX);

  gSprayController->onSprayStop([]() {
    gLastSprayVolume = gFlowSensor->getCumulativeVolumeMl();
    gFlowSensor->resetCumulativeVolume();
  });

  // TODO: Sort out Alarm registration and management

  // Sort out serial comms interface
  Serial.begin(115200);
}

/*
	• Spray (X = 3-5 minutes, Y = 1-5 seconds)
		○ Every X seconds, turn on spray
		○ After Y seconds, turn off spray
	• Flow measurement
		○ Every loop, record pulse count and calculate and record flow
		○ If (started spraying) record pulse count
		○ if (spraying) update current spray cumulative volume
		○ If (stopped spraying) record pulse count and reset. Replace last spray cumulative volume with current spray cumulative volume
	• Pressure management
		○ Every loop, calculate and record current pressure
		○ If (pressure < 80 PSI), turn off ap drain valve, turn on pump
		○ If (pressure > 100 PSI), turn off pump
		○ If (pressure > A1 PSI), raise alert and turn on ap drain valve
	• Solution tanks management
		○ Every loop, measure and record nutrient, used solution and mixing tank depths
		○ If (US tank depth > A) turn on US tank pump, if (LED_UVC) turn on UV steriliser
		○ If (US tank depth > A2) raise alert
		○ If (US tank depth < B) turn off US tank pump, if (LED_UVC) turn off UV steriliser
		○ If (M tank depth > A3) raise alert
		○ If (M tank depth + US tank depth < D) turn on Nutrient solution tank pump
		○ If (M tank depth + US tank depth > D + E) turn off Nutrient solution tank pump
		○ if (N tank depth < A4) raise alert
	• Controller comms
		○ Every loop, read USB serial input for:
			§ Control values for X and Y, persist to EEPROM if received
			§ Alert clear signals, clear relevant alerts if received
		○ Every second (?), send back over USB serial connection:
			§ Current flow rate
			§ Last spray volume
			§ current pressure, 
			§ Alerts,
			§ Current tank depths
			§ Pump states
			§ Valve states
      § UV light state
*/
void loop() {
  // Spray control
  gSprayController->controlLoop();
  // Flow sensing
  gFlowSensor->controlLoop();
  // Pressure management
  gIrrigationPressureController->controlLoop();
  // Solution tank management
  gSolutionTanksController->controlLoop();

  // Communicate any updates needed
  // TODO: Define comms protocol
}