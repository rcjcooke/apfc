#ifndef __A02YYUWDISTANCESENSOR_H_INCLUDED__
#define __A02YYUWDISTANCESENSOR_H_INCLUDED__

#include <Arduino.h>
#include <SoftwareSerial.h>

class A02YYUWDistanceSensor {

public:

  /*******************************
   * Constructors
   *******************************/
  A02YYUWDistanceSensor(uint8_t txFromuCPin, uint8_t rxTouCPin, bool processed);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last measured distance / mm
  double getDistance();
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
  uint8_t mTXFromuCPin;
  // The microcontroller pin that receives from the Distance sensor's UART interface
  uint8_t mRXTouCPin;
  // The last measured distance / mm
  double mLastMeasuredDistance;
  // If true the sensor is set to return processed data, otherwise its returning real-time data
  bool mProcessed;

  /* Serial interfaces */
  // The interface to the distance sensor
  SoftwareSerial* mSensorUART;
  // The buffer on which we receive new data
  unsigned char mBuffer[4]={};

};

#endif // __A02YYUWDISTANCESENSOR_H_INCLUDED__