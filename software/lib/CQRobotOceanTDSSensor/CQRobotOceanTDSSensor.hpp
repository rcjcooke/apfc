#ifndef __CQROBOTCOEANTDS_H_INCLUDED__
#define __CQROBOTCOEANTDS_H_INCLUDED__

#include <Arduino.h>

namespace CQRobotOceanTDS {

  /************************
   * Constants
   ************************/
  // The analog reference voltage of the ADC
  static const float VREF = 5.0;
  // The number of sample points to average
  static const int SCOUNT = 30;
  // The minimum time between data reads
  static const unsigned long READ_INTERVAL_MS = 40;

  /************************
   * Classes
   ************************/
  class CQRobotOceanTDSSensor {

  public:

    /*******************************
     * Constructors
     *******************************/
    CQRobotOceanTDSSensor(uint8_t dataPin, bool processed = true);

    /*******************************
     * Getters / Setters
     *******************************/
    // Get the last measured total dissolved solids count / ppm
    // NOTE: This needs to know the temperature of the solution to convert the analog reading to a TDS value
    float getTDSValue(float temperature);
    /* Returns true if the sensor is returning processed data, otherwise its
    * returning real-time data */
    bool isProcessed();
    /* Set processed = true to get the sensor do some pre-processing to reduce
    * noise, otherwise the sensor will return real-time data */
    void setProcessed(bool processed);

    /* For Debug purposes: The last time the sensor was asked to update the
    * distance reading / ms since last reset */
    unsigned long getLastReadTime();

    /*******************************
     * Actions
     *******************************/
    // Should be called in the main loop to keep sensor data up to date
    void controlLoop();
     // Reads the sensor data and updates the internal buffer
    void readSensor();

  private:
    /*******************************
     * Member variables
     *******************************/
    // The microcontroller pin that reads the TDS sensor's analog output
    uint8_t mDataPin;
    /* If true the sensor is set to return processed data, otherwise its returning
    * real-time data */
    bool mProcessed;
    // The buffer of read results
    int mAnalogBuffer[SCOUNT];
    // The index of the next buffer position to write to
    int mAnalogBufferIndex = 0;
    /* The last time the sensor was queried for the TDS readings
    * since last reset /ms */
    unsigned long mLastReadTime = 0;

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

    /*******************************
     * Utilities
     *******************************/
    // Utility function to get the median of an array of integers
    int getMedianNum(int bArray[], int iFilterLen);

  };
}

#endif // __CQROBOTCOEANTDS_H_INCLUDED__