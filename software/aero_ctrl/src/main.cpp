#include <Arduino.h>

#include <SerialDebuggerInput.hpp>

#include "SprayController.hpp"
#include "FlowSensor.hpp"
#include "IrrigationPressureController.hpp"

// If true, serial output from the Arduino is in human readable form - this means comms with the Raspberry Pi controller won't work
#define DEBUG_SOLO true

/************************
 * Constants
 ************************/
// Irrigation solenoid valve control pin
static const uint8_t ISVCTL = 6;
// Irrigation pressure release solenoid valve control pin
static const uint8_t IPRSVCTL = 7;
// Irrigation Pump Control pin
static const uint8_t IPCTL = 12;
// Flowrate Meter Signal - PWM: F=(38*Q), Q=L / min
static const uint8_t FM1S = A0;
// Pressure Sensor Signal - Analogue: 0-5VDC range
static const uint8_t PS1S = A7;

// The total number of alarm generators
static const int NUM_ALARM_GENERATORS = 1;

// Debug keys
static const String DEBUG_COMPRESSOR_ON_KEY = "Compressor on";

/************************
 * Variables
 ************************/

// Irrigation spray controller
SprayController* gSprayController;
// Irrigation spray flow sensor
FlowSensor* gFlowSensor;
// Irrigation pressure controller
IrrigationPressureController* gIrrigationPressureController;

// The array of all alarm generators
AlarmGenerator** gAGs;
// The serial debug interface (show values and allow control)
SerialDebuggerInput* mDebugger;

// Last spray volume / millilitres
double gLastSprayVolumeMl = 0;

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

void processDebugValueChangesFromUser(String key, String value) {
	if (key.equals(DEBUG_COMPRESSOR_ON_KEY)) {
		if (value.equals("1")) {
			gIrrigationPressureController->turnOnIrrigationCompressor();
		} else if (value.equals("0")) {
			gIrrigationPressureController->turnOffIrrigationCompressor();
		}
	}
}

/*********************
 * Entry point methods
 *********************/
void setup() {

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

  gIrrigationPressureController = new IrrigationPressureController(PS1S, IPRSVCTL, IPCTL);
	gSprayController = new SprayController(ISVCTL);
  gFlowSensor = new FlowSensor(FM1S);
  
  gSprayController->onSprayStop([]() {
    gLastSprayVolumeMl = gFlowSensor->getCumulativeVolumeMl();
    gFlowSensor->resetCumulativeVolume();
  });

	// Record which controllers are alarm generators for future access
	gAGs = new AlarmGenerator*[NUM_ALARM_GENERATORS] {gIrrigationPressureController};

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

  // Spray control
  gSprayController->controlLoop();
  // Flow sensing
  gFlowSensor->controlLoop();
  // Pressure management
  gIrrigationPressureController->controlLoop();

  // Communicate any updates needed
	if (DEBUG_SOLO) {

		String currentAlarms = createAlarmString(NUM_ALARM_GENERATORS, gAGs);
		unsigned long nextSpraySecs = gSprayController->getNextSprayCountdownMillis()/1000;

		// mDebugger->updateValue("Calibration point 1 ADC value / bits", ps->getCalibationPoint1()[0]);
		// mDebugger->updateValue("Calibration point 1 pressure / PSI", ps->getCalibationPoint1()[1]);
		// mDebugger->updateValue("Calibration point 2 ADC value / bits", ps->getCalibationPoint2()[0]);
		// mDebugger->updateValue("Calibration point 2 pressure / PSI", ps->getCalibationPoint2()[1]);
		// mDebugger->updateValue("Raw pressure sensor ADC value / bits", ps->getLastRawADCValue());
		mDebugger->updateValue("Alarms", currentAlarms);
		mDebugger->updateValue("Current calculated irrigation pressure / PSI", (float) gIrrigationPressureController->getPressurePSI());
		mDebugger->updateValue("Spray Valve open", gSprayController->isValveOpen());
		mDebugger->updateValue("Drain valve open", gIrrigationPressureController->isDrainValveOpen());
		mDebugger->updateValue(DEBUG_COMPRESSOR_ON_KEY, gIrrigationPressureController->isIrrigationCompressorOn());
		mDebugger->updateValue("Last spray volume / ml", gLastSprayVolumeMl);
		mDebugger->updateValue("Next spray in / s", nextSpraySecs);
		mDebugger->updateValue("Control loop duration / ms", (int) controlLoopDurationMillis);
		mDebugger->throttledPrintUpdate();
		mDebugger->getAndProcessUserInputUpdates();
	
	} else {
	  // TODO: Define comms protocol
	}

  controlLoopDurationMillis = millis() - startOfControlLoopMillis;

}