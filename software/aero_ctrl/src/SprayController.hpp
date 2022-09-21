#ifndef __SPRAYCONTROLLER_H_INCLUDED__
#define __SPRAYCONTROLLER_H_INCLUDED__

#include <Arduino.h>

class SprayController {

public:
  /*******************************
   * Constructors
   *******************************/
  SprayController(uint8_t valveControlPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // True when the valve is open (i.e. spraying is in progress)
  bool isValveOpen() const;
  // Get the interval between the start of one spray and the start of the next / millis
  unsigned long getSprayIntervalMillis();
  // Set the interval between the start of one spray and the start of the next / millis
  void setSprayIntervalMillis(unsigned long interval);
  // Get the duration of each spray / millis
  unsigned long getSprayDurationMillis();
  // Set the duration of each spray / millis
  void setSprayDurationMillis(unsigned long duration);

  /*******************************
   * Event handling
   *******************************/
  // Define event handler function type
  typedef void (*VoidFuncPtr)(void);
  // Defines the function to call on a spray stop event
  void onSprayStop(volatile VoidFuncPtr onHandlerFunction);

  /*******************************
   * Actions
   *******************************/
  // Called every microcontroller main program loop - controls the valve state
  void controlLoop();
  // Start spraying
  void startSpraying();
  // Stop spraying
  void stopSpraying();

protected:

private:
  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that controls the valve state
  uint8_t mValveControlPin;
  // True when the valve is open (i.e. spraying is in progress)
  bool mValveOpen;
  // The last time we sprayed / millis since board poweron
  unsigned long mLastSprayStartMillis;
  // Time between sprays / millis
  unsigned long mSprayIntervalMillis;
  // Duration of a single spray / millis
  unsigned long mSprayDurationMillis;

  // The function to call on a spray stop event
  volatile VoidFuncPtr mOnSprayStopHandlerFunction;
};

#endif // __SPRAYCONTROLLER_H_INCLUDED__
