#ifndef __A02YYUWVIAMULTIUART_H_INCLUDED__
#define __A02YYUWVIAMULTIUART_H_INCLUDED__

#include <Arduino.h>
#include "../MultiUART/MUARTSingle.hpp"

class A02YYUWviaMultiUART {

public:

  /*******************************
   * Constructors
   *******************************/
  A02YYUWviaMultiUART(MUARTSingle* mUARTSerial, uint8_t modeSelectPin, bool processed);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last measured distance / mm
  float getDistance();
  // Returns true if the sensor is returning processed data, otherwise its returning real-time data
  bool isProcessed();
  // Set processed = true to get the sensor do some pre-processing to reduce noise, otherwise the sensor will return real-time data
  void setProcessed(bool processed);

  /*******************************
   * Actions
   *******************************/
  // Reads the distance from the sensor (returns -1 if there's a checksum error, -2 if the frame wasn't read correctly)
  int readDistance();

private:
  /*******************************
   * Member variables
   *******************************/
  // The microcontroller pin that transmits to the Distance sensor's UART interface
  uint8_t mModeSelectPin;
  // The last measured distance / mm
  float mLastMeasuredDistance;
  // If true the sensor is set to return processed data, otherwise its returning real-time data
  bool mProcessed;
  // The next time we should take a reading / ms since last reset
  unsigned long mNextReadTime = 0;

  /* Serial interfaces */
  // The interface to the distance sensor
  MUARTSingle* mSensorUART;

  // Read data from the sensor into the data byte array supplied. If this fails, return false.
  bool readSensorData(byte* data);
  // Process the data in the byte array supplied. Returns distance in mm or a negative number if there's an error.
  int processData(const byte* data);

};

#endif // __A02YYUWVIAMULTIUART_H_INCLUDED__