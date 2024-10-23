#ifndef __A02YYUWVIAUARTSTREAM_H_INCLUDED__
#define __A02YYUWVIAUARTSTREAM_H_INCLUDED__

#include <Arduino.h>
#include <CircularBuffer.hpp>

namespace A02YYUW {

  /************************
   * Constants
   ************************/
  // Data packet header byte
  static const byte HEADER_BYTE = 0xFF;
  // Data packet size in bytes
  static const byte PACKET_SIZE = 4;
  // The minimum distance the sensor can detect reliably in millimeters 
  static const int LOWER_LIMIT_MM = 30;
  // The minimum time between data reads
  static const unsigned long READ_INTERVAL_MS = 100;
  // Size of the internal buffer used to read data off of the stream
  static const int INTERNAL_BUFFER_SIZE = 255;

  /************************
   * Classes
   ************************/
  class A02YYUWviaUARTStream {

  public:

    /*******************************
     * Constructors
     *******************************/
    A02YYUWviaUARTStream(Stream* mUARTSerial, uint8_t modeSelectPin, bool processed);

    /*******************************
     * Getters / Setters
     *******************************/
    // Get the last measured distance / mm
    float getDistance();
    /* Returns true if the sensor is returning processed data, otherwise its
    * returning real-time data */
    bool isProcessed();
    /* Set processed = true to get the sensor do some pre-processing to reduce
    * noise, otherwise the sensor will return real-time data */
    void setProcessed(bool processed);

    /* For Debug purposes: The last time the sensor was asked to update the
    * distance reading / ms since last reset */
    unsigned long getLastReadTime();
    /* For Debug purposes: The time the last sensor reading was successful / mm
    * since reset (i.e. a full data packet was received) */
    unsigned long getLastReadSuccess();
    /*
    * Get the last result of a readDistance call, excluding thottling: 
    *  -  0 Successfully read and updated distance,
    *  - -1 not enough data available in the read buffer from the sensor yet,
    *  - -2 HEADER_BYTE not found despite there being a full packet's worth of data in the read buffer,
    *  - -3 Not enough data in the buffer after the HEADER_BYTE to get a complete packet
    *  - -4 checksum error
    *  - -5 Invalid data packet (doesn't start with a HEADER_BYTE)
    */
    int getLastReadResult() const;
    /*
    * Get the last 5 results of a readDistance call, excluding thottling: 
    *  -  0 Successfully read and updated distance,
    *  - -1 not enough data available in the read buffer from the sensor yet,
    *  - -2 HEADER_BYTE not found despite there being a full packet's worth of data in the read buffer,
    *  - -3 Not enough data in the buffer after the HEADER_BYTE to get a complete packet
    *  - -4 checksum error
    *  - -5 Invalid data packet (doesn't start with a HEADER_BYTE)
    */
    int* getLast5ReadResults(int* results);
    // Get the last full data packet read (copied to provided array) - WARNING: No size checking
    void copyLastDataPacketReadToArray(byte* array);
    // For Debug purposes: Get the underlying data stream for this sensor
    Stream* getSensorUART();

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
    int readDistance();

  private:
    /*******************************
     * Member variables
     *******************************/
    /* The microcontroller pin that transmits to the Distance sensor's UART
    * interface */
    uint8_t mModeSelectPin;
    // The last measured distance / mm
    float mLastMeasuredDistance;
    /* If true the sensor is set to return processed data, otherwise its returning
    * real-time data */
    bool mProcessed;
    /* The last time the sensor was asked to update the distance reading / ms
    * since last reset */
    unsigned long mLastReadTime = 0;
    /* The time the last sensor reading was successful / mm since reset (i.e. a
    * full data packet was received) */
    unsigned long mLastReadSuccess = 0;
    /* The result of the last read requests (FIFO): (0 if successful, -1 if there's a
    * checksum error, -2 if the frame wasn't read correctly). If there wasn't
    * enough data available, or this was called before the next read interval is
    * due, then this returns 0; */
    CircularBuffer<int, 5> mLastReadResults;
    // The last full data packet read
    byte mLastDataPacketRead[A02YYUW::PACKET_SIZE];
    // The UART interface to the distance sensor
    Stream* mSensorUART;

    /*******************************
     * Actions
     *******************************/
    /* Read data from the sensor into the data byte array supplied. 0 = success,
    * -1 if insufficient bytes available, -2 if the header byte couldn't be found
    * despite there being at least enough bytes for a packet of data, -3 if we
    * couldn't retrieve a complete data packet. */
    int readSensorData(byte *data);
    /* Process the data in the byte array supplied. Returns distance in mm or a
    * negative number if there's an error. */
    int processData(const byte *data);

  };
}

#endif // __A02YYUWVIAUARTSTREAM_H_INCLUDED__