#include "A02YYUWviaMultiUART.hpp"

/************************
 * Constants
 ************************/
namespace A02YYUWviaMultiUARTNS {

  // Data packet header byte
  static const byte HEADER_BYTE = 0xFF;
  // Data packet size in bytes
  static const byte PACKET_SIZE = 4;
  // The minimum distance the sensor can detect reliably in millimeters 
  static const int LOWER_LIMIT_MM = 30;
  // The minimum time between data reads
  static const unsigned long READ_INTERVAL_MS = 100;

}

/*******************************
 * Constructors
 *******************************/
A02YYUWviaMultiUART::A02YYUWviaMultiUART(MUARTSingle* mUARTSerial, uint8_t modeSelectPin, bool processed) {
  mModeSelectPin = modeSelectPin;
  pinMode(mModeSelectPin, OUTPUT);

  // Set up the UART interface - The sensor supports a 9600 baud rate
  mSensorUART = mUARTSerial;
  mSensorUART->begin(9600);

  /* 
  The distance sensor has two operating modes, "processed" or "real-time". 
  Essentially "processed" comes pre-filtered to reduce noise but changes less
  frequently (100-300ms), whereas the "real-time" option updates every 100ms.

  HIGH (or floating) = Processed
  LOW = real-time
  */
  // Define what mode to operate the sensor in
  setProcessed(processed);
}

/*******************************
 * Getters / Setters
 *******************************/
// Get the last measured distance / mm (numbers lower than 30 are incorrect - 30mm is the lower bound for readings)
float A02YYUWviaMultiUART::getDistance() {
  return mLastMeasuredDistance;
}

// Returns true if the sensor is returning processed data, otherwise its returning real-time data
bool A02YYUWviaMultiUART::isProcessed() {
  return mProcessed;
}

// Set processed = true to get the sensor do some pre-processing to reduce noise, otherwise the sensor will return real-time data
void A02YYUWviaMultiUART::setProcessed(bool processed) {
  mProcessed = processed;
  digitalWrite(mModeSelectPin, processed ? HIGH : LOW);
}


/*******************************
 * Actions
 *******************************/
// Reads the distance from the sensor (returns -1 if there's a checksum error, -2 if the frame wasn't read correctly). If there wasn't enough data available, or this was called before the next read interval is due, then this returns 0;
int A02YYUWviaMultiUART::readDistance() {
  static unsigned long lastReadTime = 0;
  byte data[A02YYUWviaMultiUARTNS::PACKET_SIZE];
  int result = 0;

  // Note: There's a minimum 100ms between readings at best, so don't read if it's not been 100ms since the last correctly formatted reading
  if (millis() - lastReadTime >= A02YYUWviaMultiUARTNS::READ_INTERVAL_MS) {
    if (readSensorData(data)) {
      result = processData(data);
      // A negative result is an error code
      if (result > 0) {
        mLastMeasuredDistance = result;
      }
    }
    lastReadTime = millis();
  }
  return result;
}

bool A02YYUWviaMultiUART::readSensorData(byte* data) {
  if (mSensorUART->available() < A02YYUWviaMultiUARTNS::PACKET_SIZE) return false;

  // Read until we find the header byte or run out of data
  while (mSensorUART->available()) {
    byte firstByte = mSensorUART->read();
    if (firstByte == A02YYUWviaMultiUARTNS::HEADER_BYTE) {
      data[0] = firstByte;
      break;
    }
  }

  // If we didn't find the header byte, return false
  if (data[0] != A02YYUWviaMultiUARTNS::HEADER_BYTE) return false;

  // Read the rest of the packet
  if (mSensorUART->available() >= A02YYUWviaMultiUARTNS::PACKET_SIZE - 1) {
    // Note: Pointer arithmetic below to ensure we're only filling the byte array _after_ the first byte.
    mSensorUART->readBytes(++data, A02YYUWviaMultiUARTNS::PACKET_SIZE - 1);
  } else {
    return false; // Incomplete packet
  }

  return true;
}

int A02YYUWviaMultiUART::processData(const byte* data) {
  if (data[0] != A02YYUWviaMultiUARTNS::HEADER_BYTE) {
    // Invalid data packet
    return -2;
  }

  byte checksum = (data[0] + data[1] + data[2]) & 0xFF;
  if (checksum != data[3]) {
    // Checksum error
    return -1;
  }

  int distance = (data[1] << 8) + data[2];
  if (distance < A02YYUWviaMultiUARTNS::LOWER_LIMIT_MM) {
    return A02YYUWviaMultiUARTNS::LOWER_LIMIT_MM;
  } else {
    return distance;
  }
}