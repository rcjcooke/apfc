#include "SprayController.hpp"

/*******************************
 * Constructors
 *******************************/
SprayController::SprayController(uint8_t valveControlPin) {
  mValveControlPin = valveControlPin;
  pinMode(mValveControlPin, OUTPUT);
  mLastSprayStartMillis = 0;
  // Set defaults
  stopSpraying();
  mSprayIntervalMillis = 4*60*1000;
  mSprayDurationMillis = 3*1000;
}

/*******************************
 * Getters / Setters
 *******************************/
// True when the valve is open (i.e. spraying is in progress)
bool SprayController::isValveOpen() const {
  return mValveOpen;
}

// Get the interval between the start of one spray and the start of the next / millis
unsigned long SprayController::getSprayIntervalMillis() {
  return mSprayIntervalMillis;
}

// Set the interval between the start of one spray and the start of the next / millis
void SprayController::setSprayIntervalMillis(unsigned long interval) {
  mSprayIntervalMillis = interval;
}

// Get the duration of each spray / millis
unsigned long SprayController::getSprayDurationMillis() {
  return mSprayDurationMillis;
}

// Set the duration of each spray / millis
void SprayController::setSprayDurationMillis(unsigned long duration) {
  mSprayDurationMillis = duration;
}

/*******************************
 * Event handling
 *******************************/
// Defines the function to call on a spray stop event
void SprayController::onSprayStop(VoidFuncPtr onHandlerFunction) {
  mOnHandlerFunction = onHandlerFunction;
}

/*******************************
 * Actions
 *******************************/
// Called every microcontroller main program loop - controls the valve state
void SprayController::controlLoop() {
  unsigned long currentMillis = millis();
  unsigned long timeSinceLastSprayStart = currentMillis - mLastSprayStartMillis;
  if (mValveOpen && (timeSinceLastSprayStart > mSprayDurationMillis)) {
    stopSpraying();
  }
  if (timeSinceLastSprayStart > mSprayIntervalMillis) {
    mLastSprayStartMillis = currentMillis;
    startSpraying();
  }
}

  // Start spraying
void SprayController::startSpraying() {
  digitalWrite(mValveControlPin, HIGH);
  mValveOpen = true;
}

// Stop spraying
void SprayController::stopSpraying() {
  digitalWrite(mValveControlPin, LOW);
  mValveOpen = false;
  if (mOnHandlerFunction) mOnHandlerFunction();
}