#include <Arduino.h>

#include <SerialDebuggerInput.hpp>

#include "SolutionTanksController.hpp"

// Set to true if the UV Steriliser light is LED based - if LED it will turn the light off when not needed to save power
#define LED_UVC false
// If true, serial output from the Arduino is in human readable form - this means comms with the Raspberry Pi controller won't work
#define DEBUG_SOLO true

/************************
 * Constants
 ************************/
// Runoff Recycling Pump control pin
static const uint8_t RRPCTL = 4;
// Irrigation Supply Pump control pin
static const uint8_t ISPCTL = 9; // TODO
// UV Sterilisation Lamp Control pin
static const uint8_t UVLCTL = 11; // TODO
/*
 * WARNING: Not all pins on the Mega and Mega 2560 boards support change
 * interrupts, so only the following can be used for RX: 10, 11, 12, 13, 14, 15,
 * 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14
 * (68), A15 (69)
 */
// Runoff Recycling Tank Depth Sensor: Processed Value/Real-time Value Output
// Selection pin
static const uint8_t RRT_DSMS = 49;
// Runoff Recycling Tank Depth Sensor: Depth data pin
static const uint8_t RRT_DSO = 53;
// Irrigation Supply Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t IST_DSMS = 47;
// Irrigation Supply Tank Depth Sensor: Depth data pin
static const uint8_t IST_DSO = 51;
// Mixing Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t MT_DSMS = 2; // TODO
// Mixing Tank Depth Sensor: Depth data pin
static const uint8_t MT_DSO = 3; // TODO

// The total number of alarm generators
static const int NUM_ALARM_GENERATORS = 1;

/************************
 * Variables
 ************************/

// Controls the movement of solution between tanks
SolutionTanksController* gSolutionTanksController;

// The array of all alarm generators
AlarmGenerator** gAGs;
// The serial debug interface
SerialDebuggerInput* mDebugger;

/*********************
 * Utility functions
 *********************/
String createAlarmString(int numAGs, AlarmGenerator** ags) {
	String alarmString = "";
	for (int agi = 0; agi < numAGs; agi++) {
		AlarmGenerator* ag = ags[agi];
		bool* aStates = ag->getAlarmStates();
		int numAlarmCodes = ag->getNumAlarmCodes();
		char agCode = ag->getUniqueAlarmGeneratorCode();
		for (int i = 0; i < numAlarmCodes; i++) {
			if (aStates[i]) {
				if (alarmString.length() != 0) alarmString += ',';
				alarmString += agCode;
				alarmString += i;
			}
		}
	}
	return alarmString;
}

// Process change requests made on the debug serial interface
void processDebugValueChangesFromUser(String key, String value) {
	// TODO
}

/*********************
 * Entry point methods
 *********************/
void setup() {

  gSolutionTanksController = new SolutionTanksController(RRPCTL, ISPCTL, UVLCTL, LED_UVC, RRT_DSMS, IST_DSMS, MT_DSMS, RRT_DSO, IST_DSO, MT_DSO);

	// Record which controllers are alarm generators for future access
	gAGs = new AlarmGenerator*[NUM_ALARM_GENERATORS] {gSolutionTanksController};

	if (DEBUG_SOLO) {
		// Note: this also starts the serial interface at a baud rate of 115200 bps
		mDebugger = new SerialDebuggerInput(115200);
		mDebugger->onValueChanged(processDebugValueChangesFromUser);
	} else {
		// TODO: Sort out serial comms interface
		Serial.begin(115200);
		// Wait for initialisation of the serial interface
		while(!Serial);
	}

}

/*
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

  unsigned long startOfControlLoopMillis = millis();
	static unsigned long controlLoopDurationMillis = 0;

  // Solution tank management
  gSolutionTanksController->controlLoop();

  // Communicate any updates needed
	if (DEBUG_SOLO) {

		String currentAlarms = createAlarmString(NUM_ALARM_GENERATORS, gAGs);

		mDebugger->updateValue("Alarms", currentAlarms);
		mDebugger->updateValue("Irrigation Supply Tank Depth / mm", gSolutionTanksController->getIrrigationSupplyTankDepth());
		mDebugger->updateValue("Runoff Recycling Tank Depth / mm", gSolutionTanksController->getRunoffRecyclingTankDepth());
		mDebugger->updateValue("Runoff Recycling Pump On", gSolutionTanksController->isRunoffRecyclingPumpOn());
		mDebugger->updateValue("Control loop duration / ms", (int) controlLoopDurationMillis);
		mDebugger->throttledPrintUpdate();
		// mDebugger->getAndProcessUserInputUpdates(); - This is broken - don't know why yet but I don't need it to work yet anyway
	
	} else {
	  // TODO: Define comms protocol
	}

  controlLoopDurationMillis = millis() - startOfControlLoopMillis;

}