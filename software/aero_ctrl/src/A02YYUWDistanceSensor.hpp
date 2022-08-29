#ifndef __A02YYUWDISTANCESENSOR_H_INCLUDED__
#define __A02YYUWDISTANCESENSOR_H_INCLUDED__

#include <Arduino.h>
#include <SoftwareSerial.h>

#include "AlarmGenerator.hpp"

class A02YYUWDistanceSensor {

public:

  /*******************************
   * Constructors
   *******************************/
  A02YYUWDistanceSensor(uint8_t txFromuCPin, uint8_t rxTouCPin);

  /*******************************
   * Getters / Setters
   *******************************/
  // Get the last measured distance / mm
  double getDistance();

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

  /* Serial interfaces */
  // The interface to the distance sensor
  SoftwareSerial* mSensorUART;
  // The buffer on which we receive new data
  unsigned char mBuffer[4]={};

};

#endif // __A02YYUWDISTANCESENSOR_H_INCLUDED__