#include <Arduino.h>

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

/*********************
 * Interrupt routines
 *********************/
void flowMetrePulsedISR() {
  unsigned long currentMillis = millis();
  cli();
  // TODO: Deal with stuff here

  sei();
} 

/*********************
 * Entry Point methods
 *********************/
void setup() {
  // Set up control pins
  pinMode(SV1CTL, OUTPUT);
  pinMode(SV2CTL, OUTPUT);
  pinMode(TP1CTL, OUTPUT);
  pinMode(TP2CTL, OUTPUT);
  pinMode(IPCTL, OUTPUT);
  pinMode(UVLCTL, OUTPUT);
  
  pinMode(PS1S, INPUT);

  // Set up Interrupts
  attachInterrupt(digitalPinToInterrupt(FM1S), flowMetrePulsedISR, RISING);

  // TODO: Set up the Depth Sensor UART inputs

}

void loop() {
  int pressureSensorRawValue = analogRead(PS1S);

}