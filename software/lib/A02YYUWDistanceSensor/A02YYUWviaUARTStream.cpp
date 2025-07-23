#include "A02YYUWviaUARTStream.hpp"

using namespace A02YYUW;

/*******************************
 * Constructors
 *******************************/
A02YYUWviaUARTStream::A02YYUWviaUARTStream(Stream* mUARTSerial, uint8_t modeSelectPin, bool processed) {
  mModeSelectPin = modeSelectPin;
  pinMode(mModeSelectPin, OUTPUT);

  // Set up the UART interface - The sensor supports a 9600 baud rate
  mSensorUART = mUARTSerial;

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
float A02YYUWviaUARTStream::getDistance() {
  return mLastMeasuredDistance;
}

// Returns true if the sensor is returning processed data, otherwise its returning real-time data
bool A02YYUWviaUARTStream::isProcessed() {
  return mProcessed;
}

// Set processed = true to get the sensor do some pre-processing to reduce noise, otherwise the sensor will return real-time data
void A02YYUWviaUARTStream::setProcessed(bool processed) {
  mProcessed = processed;
  digitalWrite(mModeSelectPin, processed ? PROCESSED : REALTIME);
}

// For Debug purposes: The last time the sensor was asked to update the distance reading / ms since last reset
unsigned long A02YYUWviaUARTStream::getLastReadTime() {
  return mLastReadTime;
}

// For Debug purposes: The time the last sensor reading was successful / mm since reset (i.e. a full data packet was received)
unsigned long A02YYUWviaUARTStream::getLastReadSuccess() {
  return mLastReadSuccess;
}

// For Debug purposes: The result of the last read request: -1 if there's a checksum error, -2 if the frame wasn't read correctly, otherwise a distance in mm
int A02YYUWviaUARTStream::getLastReadResult() const {
  return mLastReadResults.last();
}

/*
 * Get the last 5 results of a readDistance call, excluding thottling: 
 *  -  0 Successfully read and updated distance,
 *  - -1 not enough data available in the read buffer from the sensor yet,
 *  - -2 HEADER_BYTE not found despite there being a full packet's worth of data in the read buffer,
 *  - -3 Not enough data in the buffer after the HEADER_BYTE to get a complete packet
 *  - -4 checksum error
 *  - -5 Invalid data packet (doesn't start with a HEADER_BYTE)
 * 
 * If there have been less than 5 results, then the remainder of the results array returned is 
 * padded with -1. 
 */
int* A02YYUWviaUARTStream::getLast5ReadResults(int* results) {
  mLastReadResults.copyToArray(results);
  if (mLastReadResults.size() < 5) {
    for (size_t i = mLastReadResults.size(); i < 5; i++) {
      // If we haven't got 5 results yet, then pad the end of the array with "not enough data" results
      results[i] = -1;
    }
  }
  return results;
}

// Get the last full data packet read (copied to provided array) - WARNING: No size checking
void A02YYUWviaUARTStream::copyLastDataPacketReadToArray(byte* array) {
  for (size_t i = 0; i < A02YYUW::PACKET_SIZE; i++) {
    array[i] = mLastDataPacketRead[i];
  }
}


// For Debug purposes: Get the underlying data stream for this sensor
Stream* A02YYUWviaUARTStream::getSensorUART() {
  return mSensorUART;
}

/*******************************
 * Actions
 *******************************/
/*
 * Reads the distance from the sensor. Returns: 
 *  -  1 Throttled out: call to readDistance() function made witihin minimum re-read time (this result is the only one not added to the lastResults buffer)
 *  -  0 Successfully read and updated distance,
 *  - -1 not enough data available in the read buffer from the sensor yet,
 *  - -2 HEADER_BYTE not found despite there being a full packet's worth of data in the read buffer,
 *  - -3 Not enough data in the buffer after the HEADER_BYTE to get a complete packet
 *  - -4 checksum error
 *  - -5 Invalid data packet (doesn't start with a HEADER_BYTE)
 */
int A02YYUWviaUARTStream::readDistance() {
  unsigned long now = millis();
  // Note: There's a minimum 100ms between readings at best, so don't read if it's not been 100ms since the last correctly formatted reading
  int lastReadResult = 1;
  if (now - mLastReadTime >= A02YYUW::READ_INTERVAL_MS) {
    int status = readSensorData(mLastDataPacketRead);
    if (status == 0) {
      int result = processData(mLastDataPacketRead);
      // A negative result is an error code
      if (result > 0) {
        mLastMeasuredDistance = result;
        mLastReadSuccess = now;
        lastReadResult = 0;
      } else if (result == -1) {
        // Checksum error
        lastReadResult = -4;
      } else if (result == -2) {
        // Invalid data packet (doesn't start with a HEADER_BYTE)
        lastReadResult = -5;
      }
    } else if (status == -1) {
      // not enough data available in the read buffer from the sensor yet
      lastReadResult = -1;
    } else if (status == -2) {
      // HEADER_BYTE not found despite there being a full packet's worth of data in the read buffer
      lastReadResult = -2;
    } else if (status == -3) {
      // Not enough data in the buffer after the HEADER_BYTE to get a complete packet
      lastReadResult = -3;
    }
    mLastReadTime = now;
    // Don't store throttled out results, otherwise record them
    mLastReadResults.push(lastReadResult);
  }
  return lastReadResult;
}

int A02YYUWviaUARTStream::readSensorData(byte* data) {
  if (mSensorUART->available() < A02YYUW::PACKET_SIZE) return -1;

  /* We want the most recent distance reading, so in case there's been a build
   * up of data in the buffer, get everything and then start from the end and
   * work backwards */

  int bytesToRead = mSensorUART->available();
  if (bytesToRead > A02YYUW::INTERNAL_BUFFER_SIZE) bytesToRead = A02YYUW::INTERNAL_BUFFER_SIZE;
  byte buffer[bytesToRead];
  mSensorUART->readBytes(buffer, bytesToRead);

  /* Read back from the end of the buffer (assuming a full packet, because we've
   tested for that above and we don't want incomplete packets at the end of the
   buffer) until we find the header byte or run out of data */
  for (size_t i = bytesToRead - A02YYUW::PACKET_SIZE; i >= 0; i--) {
    if (buffer[i] == A02YYUW::HEADER_BYTE) {
      for (size_t j = 0; j < A02YYUW::PACKET_SIZE; j++) {
        data[j] = buffer[i+j];
      }
      break;
    }
  }

  // If we didn't find the header byte, return false
  if (data[0] != A02YYUW::HEADER_BYTE) return -2;

  // We've read in a whole packet so tell teh world we're good :)
  return 0;
}

int A02YYUWviaUARTStream::processData(const byte* data) {
  if (data[0] != A02YYUW::HEADER_BYTE) {
    // Invalid data packet
    return -2;
  }

  byte checksum = (data[0] + data[1] + data[2]) & 0xFF;
  if (checksum != data[3]) {
    // Checksum error
    return -1;
  }

  int distance = (data[1] << 8) + data[2];
  if (distance < A02YYUW::LOWER_LIMIT_MM) {
    return A02YYUW::LOWER_LIMIT_MM;
  } else {
    return distance;
  }
}