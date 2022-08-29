#include "A02YYUWDistanceSensor.hpp"

/*******************************
 * Constructors
 *******************************/
A02YYUWDistanceSensor::A02YYUWDistanceSensor(uint8_t txFromuCPin, uint8_t rxTouCPin) {
  mTXFromuCPin = txFromuCPin;
  mRXTouCPin = rxTouCPin;

  /* 
  The distance sensor has two operating modes, "processed" or "real-time". 
  Essentially "processed" comes pre-filtered to reduce noise but changes less
  frequently (100-300ms), whereas the "real-time" option updates every 100ms.

  HIGH (or floating) = Processed
  LOW = real-time
  */
  pinMode(mTXFromuCPin, OUTPUT);
  pinMode(mRXTouCPin, INPUT);

  // Set up the UART interface - The sensor supports a 9600 baud rate
  mSensorUART = new SoftwareSerial(mRXTouCPin, mTXFromuCPin); // RX, TX
  mSensorUART->begin(9600);
}

/*******************************
 * Getters / Setters
 *******************************/
// Get the last measured distance / mm (numbers lower than 30 are incorrect - 30mm is the lower bound for readings)
double A02YYUWDistanceSensor::getDistance() {
  return mLastMeasuredDistance;
}

/*******************************
 * Actions
 *******************************/
// Reads the distance from the sensor (returns -1 if there's a checksum error, -2 if the frame wasn't read correctly)
int A02YYUWDistanceSensor::readDistance() {
  do {
    for (int i = 0; i < 4; i++) {
      mBuffer[i] = mSensorUART->read();
    }
  } while (mSensorUART->read() == 0xff);

  mSensorUART->flush();

  // Note: 0xff = header
  if (mBuffer[0] == 0xff) {
    int sum = (mBuffer[0] + mBuffer[1] + mBuffer[2]) & 0x00FF;
    // checksum check
    if (sum == mBuffer[3]) {
      mLastMeasuredDistance = (mBuffer[1] << 8) + mBuffer[2];
      return 0;
    } else return -1;
  } else return -2;
}