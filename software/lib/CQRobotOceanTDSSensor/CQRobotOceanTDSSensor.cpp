#include "CQRobotOceanTDSSensor.hpp"

using namespace CQRobotOceanTDS;

/*******************************
 * Constructors
 *******************************/
CQRobotOceanTDSSensor::CQRobotOceanTDSSensor(uint8_t dataPin, bool processed) {
  mDataPin = dataPin;
  pinMode(dataPin, INPUT);
  mProcessed = processed;
  mLastReadTime = 0;
}

void CQRobotOceanTDSSensor::controlLoop() {
  readSensor();
}

void CQRobotOceanTDSSensor::readSensor() {
  // Only read as frequently as the READ_INTERVAL_MS
  if (millis() - mLastReadTime > READ_INTERVAL_MS) {
    mLastReadTime = millis();

    if (isProcessed()) {
      mAnalogBuffer[mAnalogBufferIndex] = analogRead(mDataPin);    //read the analog value and store into the buffer
      mAnalogBufferIndex++;
      if (mAnalogBufferIndex == SCOUNT) mAnalogBufferIndex = 0;
    } else {
      // Read the analog value directly
      mAnalogBuffer[0] = analogRead(mDataPin);
      mAnalogBufferIndex = 1; // Reset index to 1 since we only have one reading
    }
  }
}

bool CQRobotOceanTDSSensor::isProcessed() {
  return mProcessed;
}

void CQRobotOceanTDSSensor::setProcessed(bool processed) {
  mProcessed = processed;
}

float CQRobotOceanTDSSensor::getTDSValue(float temperature) {
  int analogValue = 0;
  if (isProcessed()) {
    // Get the median of the readings array
    int analogBufferTemp[SCOUNT];
    for (int copyIndex = 0; copyIndex < SCOUNT; copyIndex++) {
      analogBufferTemp[copyIndex] = mAnalogBuffer[copyIndex];
    }
    analogValue = getMedianNum(analogBufferTemp, SCOUNT);
  } else {
    // Just get the last reading
    analogValue = mAnalogBuffer[0];
  }

  // convert the ADC value to a voltage
  float voltage = analogValue * VREF / 1024.0;
  // Adjust the voltage based on the temperature
  // Temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
  float compensationCoefficient = 1.0 + 0.02 * (temperature - 25.0);
  float compensatedVoltage = voltage / compensationCoefficient;
  // Convert voltage value to TDS value in ppm
  float tdsValue = (133.42 * compensatedVoltage * compensatedVoltage * compensatedVoltage - 255.86 * compensatedVoltage * compensatedVoltage + 857.39 * compensatedVoltage) * 0.5;
  return tdsValue;
}

int CQRobotOceanTDSSensor::getMedianNum(int bArray[], int iFilterLen) {
  int bTab[iFilterLen];
  for (byte i = 0; i < iFilterLen; i++) {
    bTab[i] = bArray[i];
  }
  int i, j, bTemp;
  for (j = 0; j < iFilterLen - 1; j++) {
    for (i = 0; i < iFilterLen - j - 1; i++) {
      if (bTab[i] > bTab[i + 1]) {
        bTemp = bTab[i];
        bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
      }
    }
  }
  if ((iFilterLen & 1) > 0) {
    bTemp = bTab[(iFilterLen - 1) / 2];
  } else {
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
  }
  return bTemp;
}