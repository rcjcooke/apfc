#include <Arduino.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#include <SerialDebugger.hpp>

#include "SolutionTanksController.hpp"
#include "WaterSupplyController.hpp"

// Set to true if the UV Steriliser light is LED based - if LED it will turn the light off when not needed to save power
#define LED_UVC false
// If true, serial output from the Arduino is in human readable form - this means comms with the Raspberry Pi controller won't work
#define DEBUG_SOLO true

using namespace apfc;

/************************
 * Constants
 ************************/
// MultiUART Board 2 Chip Select pin
static const uint8_t MU2CS = 7;
// MultiUART Board 1 Chip Select pin
static const uint8_t MU1CS = 6;

// Irrigation Supply Pump control pin
static const uint8_t ISPCTL = 5;
// Runoff Recycling Pump control pin
static const uint8_t RRPCTL = 4;
// Mixing Pump control pin
static const uint8_t MPCTL = 3;

// UV Sterilisation Control pin
static const uint8_t UVCTL = 2;
// Heater Control pin
static const uint8_t HCTL = 22;
// Water Filter System Flush Solenoid Control pin
static const uint8_t WSFCTL = 23;

/* MultiUART board peripheral indexes */
// MUART1: Irrigation Supply Tank Depth Sensor
static const char IST_MUART_INDEX = 0;
// MUART1: Runoff Recycling Tank Depth Sensor
static const char RRT_MUART_INDEX = 1;
// MUART1: Mixing Tank Depth Sensor
static const char MT_MUART_INDEX = 2;

// MUART2: Water Supply Tank Depth Sensor
static const char WST_MUART_INDEX = 0;
// MUART2: Waste Water Tank Depth Sensor
static const char WWT_MUART_INDEX = 1;

// Mixing Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t MT_DSMS = 10;
// Runoff Recycling Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t RRT_DSMS = 9;
// Irrigation Supply Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t IST_DSMS = 8;
// Water Supply Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t WST_DSMS = 11;
// Waste Water Tank Depth Sensor: Processed Value/Real-time Value Output Selection pin
static const uint8_t WWT_DSMS = 12;

/* TDS Sensor pins */
// Water Supply Tank TDS Sensor data pin
static const uint8_t WS_TDS_A = A0;
// Pre-RO Water TDS Sensor data pin
static const uint8_t PRO_TDS_A = A1;
// Output Water TDS Sensor data pin
static const uint8_t OW_TDS_A = A2;

/* Temperature sensor pins*/
// Water Supply Tank Temperature Sensor data pin
static const uint8_t WST_T = 24;
// Address of the Water Supply Tank Temperature Sensor - should be determined using the address finder utility
static const DeviceAddress WST_T_ADDR = {0x28, 0x80, 0x43, 0x94, 0x97, 0x01, 0x03, 0xC8};

/*
 * WARNING: Not all pins on the Mega and Mega 2560 boards support change
 * interrupts, so only the following can be used for RX: 10, 11, 12, 13, 14, 15,
 * 50, 51, 52, 53, A8 (62), A9 (63), A10 (64), A11 (65), A12 (66), A13 (67), A14
 * (68), A15 (69)
 */

// The total number of alarm generators
static const int NUM_ALARM_GENERATORS = 2;

/************************
 * Variables/ Fields
 ************************/

// A OneWire instance to communicate with any OneWire devices
OneWire* gOneWire;
// Temperature sensors interface - all have to be managed at this level together frustratingly
DallasTemperature* gTemperatureSensors;

 // Controls the movement of solution between tanks
SolutionTanksController* gSolutionTanksController;
// Manages the water supply
WaterSupplyController* gWaterSupplyController;

// The array of all alarm generators
AlarmGenerator** gAGs;
// The serial debug interface
SerialDebugger* mDebugger;

/*********************
 * Utility functions
 *********************/
// Create a human readable string of all the current alarms
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

// Formats a single byte to a Intel format HEX annotation, e.g. 0xF2
String formatByteToIntelString(const uint8_t byte) {
  String result = "0x";
  if (byte < 16) result = result + "0";
  result = result + String((int) byte, HEX);
  return result;
}

// Formats a data packet to a human readable string
String dataPacketToString(const byte* packet) {
	String result = "";
	if (packet) {
		for (size_t i = 0; i < A02YYUW::PACKET_SIZE; i++) {
			result+=(formatByteToIntelString(packet[i]) + " ");
		}
	} else {
		result = "NULL";
	}
	return result;
}

String resultsArrayToString(const int* results) {
	String result = "{" + String(results[0]);
	for (size_t i = 1; i < 5; i++) {
		result+=("," + String(results[i]));
	}
	return result+="}";
}

// Converts the ControllerRunState enum to a human readable string
String controllerRunStateToString(ControllerRunState state) {
	switch (state) {
		case ControllerRunState::STARTUP: return "STARTUP";
		case ControllerRunState::RUNNING: return "RUNNING";
		case ControllerRunState::EMERGENCY: return "EMERGENCY";
		default: return "UNKNOWN";
	}
}

// Process change requests made on the debug serial interface
void processDebugValueChangesFromUser(String key, String value) {
	// TODO
}

/*********************
 * Entry point methods
 *********************/
void setup() {

	// Set up the OneWire instance to communicate with the temperature sensor 
	// Note: At the moment there is only one temp sensor on this. Once this becomes multiple, this should be renamed to a generic Bus pin name.
	gOneWire = new OneWire(WST_T);
	// Set up temperature sensors interface
	gTemperatureSensors = new DallasTemperature(gOneWire);

  gSolutionTanksController = new SolutionTanksController(
		RRPCTL, MU1CS, IST_MUART_INDEX, RRT_MUART_INDEX, RRT_DSMS, IST_DSMS, 
		UVCTL, LED_UVC);
	gWaterSupplyController = new WaterSupplyController(
		WSFCTL, MU2CS, WST_MUART_INDEX, WWT_MUART_INDEX, WST_DSMS, WWT_DSMS,
		WS_TDS_A, PRO_TDS_A, OW_TDS_A, gTemperatureSensors, WST_T_ADDR);
	// Record which controllers are alarm generators for future access
	gAGs = new AlarmGenerator*[NUM_ALARM_GENERATORS] {gSolutionTanksController, gWaterSupplyController};
	
	if (DEBUG_SOLO) {
		// Note: this also starts the serial interface at a baud rate of 115200 bps
		mDebugger = new SerialDebugger(115200);
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

	static unsigned long controlLoopDurationMillis = 0;
	// static unsigned long lastSerialWrite = 0;
	unsigned long startOfControlLoopMillis = millis();
  // Water Supply management
  gWaterSupplyController->controlLoop();
  // Solution tank management
  gSolutionTanksController->controlLoop();

  // Communicate any updates needed
	if (DEBUG_SOLO) {

		// Retrieve / pre-format some data
		String currentAlarms = createAlarmString(NUM_ALARM_GENERATORS, gAGs);
		// int results[5]; 
		// gSolutionTanksController->getIrrigationSupplyTankDepthSensor()->getLast5ReadResults(results);
		// byte dataPacket[A02YYUW::PACKET_SIZE];
		// gSolutionTanksController->getIrrigationSupplyTankDepthSensor()->copyLastDataPacketReadToArray(dataPacket);
		// mDebugger->updateValue("IST DS Last 5 Read Results", resultsArrayToString(results));
		// mDebugger->updateValue("IST DS UART bytes available to read", gSolutionTanksController->getIrrigationSupplyTankDepthSensor()->getSensorUART()->available());
		// mDebugger->updateValue("IST DS UART last 4 bytes read", dataPacketToString(dataPacket));

		mDebugger->updateValue("Alarms", currentAlarms);
		mDebugger->updateValue("IST DS Last Read Success / ms since reset", gSolutionTanksController->getIrrigationSupplyTankDepthSensor()->getLastReadSuccess());
		mDebugger->updateValue("Irrigation Supply Tank Depth / mm", gSolutionTanksController->getIrrigationSupplyTankDepth());
		mDebugger->updateValue("RST DS Last Read Success / ms since reset", gSolutionTanksController->getIrrigationSupplyTankDepthSensor()->getLastReadSuccess());
		mDebugger->updateValue("Runoff Supply Tank Depth / mm", gSolutionTanksController->getRunoffRecyclingTankDepth());
		mDebugger->updateValue("Solution Tanks Controller Run State", controllerRunStateToString(gSolutionTanksController->getRunState()));
		mDebugger->updateValue("Runoff Recycling Pump On", gSolutionTanksController->isRunoffRecyclingPumpOn());
		mDebugger->updateValue("Control loop duration / ms", (unsigned long) controlLoopDurationMillis);
		mDebugger->throttledPrintUpdate();
	
	} else {
	  // TODO: Define comms protocol
	}

  controlLoopDurationMillis = millis() - startOfControlLoopMillis;

}

