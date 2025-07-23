#include "WaterSupplyController.hpp"

using namespace apfc;

/************************
 * Constants
 ************************/
namespace WaterSupplyControllerNS {

  // The distance measured for a completely empty tank - TODO: Measure this
  static const float EMPTY_TANK_DISTANCE_MM = 1000.0f;
  
  // At this depth, there's too much in the tank
  static const float WARNING_TANK_MAX_DEPTH_MM = 900.0f;
  // At this depth, there's too little in the tank
  static const float WARNING_TANK_MIN_DEPTH_MM = 30.0f;

  // The minimum time between control loop executions / ms
  static const unsigned long CONTROLLER_UPDATE_INTERVAL_MILLIS = 1000; // 1 second

  // The minimum TDS delta across the pre-RO filters if they're worked satisfactorily
  // TODO: This needs testing and adjustment
  static const float MIN_PRE_RO_FILTER_DELTA = 100.0f;
  // The maximum acceptable TDS value for the fully filtered water / ppm
  // TODO: This needs testing and adjustment
  static const float MAX_ACCEPTABLE_OUTPUT_TDS = 50.0f;

  // The minimum time between flushing the water filter system / ms
  static const unsigned long MIN_FLUSH_INTERVAL_MILLIS = 604800000l; // 1 week in milliseconds
  // The time the flush valve is open for flushing the water filter system / ms
  static const unsigned long FLUSH_VALVE_OPEN_INTERVAL_MILLIS = 120000l; // 2 minutes in milliseconds

}

/*******************************
 * Constructors
 *******************************/
WaterSupplyController::WaterSupplyController(
    uint8_t waterFilterSystemFlushSolenoidControlPin,
    uint8_t multiUART2CSPin,
    char waterSupplyTankDepthSensorMUARTIndex,
    char wasteWaterTankDepthSensorMUARTIndex,
    uint8_t waterSupplyTankDepthModeSelectPin,
    uint8_t wasteWaterTankDepthModeSelectPin,
    uint8_t waterSupplyTDSDataPin,
    uint8_t preROTDSDataPin,
    uint8_t outputWaterTDSDataPin, 
    DallasTemperature* temperatureSensors,
    const DeviceAddress* waterSupplyTankTemperatureSensorAddress) 
      : StandardController('W', 7), 
        mWaterFilterSystemFlushSolenoidControlPin(waterFilterSystemFlushSolenoidControlPin),
        mTemperatureSensors(temperatureSensors),
        mWaterSupplyTankTemperatureSensorAddress(waterSupplyTankTemperatureSensorAddress) {

  // Sort out the control pins
  pinMode(mWaterFilterSystemFlushSolenoidControlPin, OUTPUT);
  closeFlushValve(); // Ensure the flush valve is closed at startup

  // Sort out the serial interfaces
  MULTIUART* multiuart = new MULTIUART(multiUART2CSPin);
  //SPI Prescaler Options
  //SPI_CLOCK_DIV4 / SPI_CLOCK_DIV16 / SPI_CLOCK_DIV64
  //SPI_CLOCK_DIV128 / SPI_CLOCK_DIV2 / SPI_CLOCK_DIV8 / SPI_CLOCK_DIV32
  multiuart->initialise(SPI_CLOCK_DIV64);
  
  // Set up the depth sensors
  mWaterSupplyTankDepthSensor = new A02YYUW::A02YYUWviaUARTStream(
    new MUARTSingleStream(multiuart, waterSupplyTankDepthSensorMUARTIndex), 
    waterSupplyTankDepthModeSelectPin, true);
  mWasteWaterTankDepthSensor = new A02YYUW::A02YYUWviaUARTStream(
    new MUARTSingleStream(multiuart, wasteWaterTankDepthSensorMUARTIndex), 
    wasteWaterTankDepthModeSelectPin, true);

  // Set up the TDS sensors
  mWaterSupplyTDSSensor = new CQRobotOceanTDS::CQRobotOceanTDSSensor(waterSupplyTDSDataPin, true);
  mPreROTDSensor = new CQRobotOceanTDS::CQRobotOceanTDSSensor(preROTDSDataPin, true);
  mOutputWaterTDSSensor = new CQRobotOceanTDS::CQRobotOceanTDSSensor(outputWaterTDSDataPin, true);

}

/*******************************
 * Getters / Setters
 *******************************/
// Get the current water supply tank depth / mm
float WaterSupplyController::getWaterSupplyTankDepth() const {
  return mWaterSupplyTankDepth;
};

// Get the current waste water tank depth / mm
float WaterSupplyController::getWasteWaterTankDepth() const {
  return mWasteWaterTankDepth;
};

// Get the current water supply tank temperature / C
// If the sensor is disconnected, this will return DEVICE_DISCONNECTED_C = -127
float WaterSupplyController::getWaterSupplyTemperature() const {
  return mWaterTemperature;
};

// Get the water supply tank TDS value / ppm
float WaterSupplyController::getWaterSupplyTDSValue() const {
  return mWaterSupplyTDSSensor->getTDSValue(mWaterTemperature);
}

// Get the pre-RO water TDS value / ppm
float WaterSupplyController::getPreROTDSValue() const {
  return mPreROTDSensor->getTDSValue(mWaterTemperature);
};

// Get the filtered water TDS value / ppm
float WaterSupplyController::getOutputWaterTDSValue() const {
  return mOutputWaterTDSSensor->getTDSValue(mWaterTemperature);
};

// For Debug purposes: Get the depth sensor instance for the Water Supply Tank
A02YYUW::A02YYUWviaUARTStream* WaterSupplyController::getWaterSupplyTankDepthSensor() {
  return mWaterSupplyTankDepthSensor;
}
// For Debug purposes: Get the depth sensor instance for the Waste Water Tank
A02YYUW::A02YYUWviaUARTStream* WaterSupplyController::getWasteWaterTankDepthSensor() {
  return mWasteWaterTankDepthSensor;
}
// For Debug purposes: Get the TDS sensor for the water supply
CQRobotOceanTDS::CQRobotOceanTDSSensor* WaterSupplyController::getWaterSupplyTDSSensor() {
  return mWaterSupplyTDSSensor;
}
// For Debug purposes: Get the sensor measuring the pre-RO water TDS
CQRobotOceanTDS::CQRobotOceanTDSSensor* WaterSupplyController::getPreROTDSSensor() {
  return mPreROTDSensor;
}
// For Debug purposes: Get the sensor measuring the filtered water TDS
CQRobotOceanTDS::CQRobotOceanTDSSensor* WaterSupplyController::getOutputWaterTDSSensor() {
  return mOutputWaterTDSSensor;
}

/*******************************
 * Actions
 *******************************/
// Control loop during startup state
void WaterSupplyController::startupStateLoop() {

  // TODO: Introduce timeout check to handle faulty sensors preventing startup

  // Get readings
  mWaterSupplyTankDepthSensor->readDistance();
  mWasteWaterTankDepthSensor->readDistance();

  // Keep cycling until we get consistent valid readings from all depth sensors - this deals with buffering and comms protocol sychronisation concerns
  if (A02YYUW::checkLast5DepthSensorReadings(mWaterSupplyTankDepthSensor, true)) return;
  if (A02YYUW::checkLast5DepthSensorReadings(mWasteWaterTankDepthSensor, true)) return;
  // Make sure we've got a valid temperature reading
  if (!updateWaterTemperature()) return;

  // Nothing to set up on the TDS sensors

  // If we've got this far then transition to a running state
  setRunState(ControllerRunState::RUNNING);

}

// Control loop during running state
void WaterSupplyController::runningStateLoop() {

  if (millis() - mLastControlLoopMillis < WaterSupplyControllerNS::CONTROLLER_UPDATE_INTERVAL_MILLIS) {
    // If we haven't waited long enough since the last control loop, then return
    return;
  }
  mLastControlLoopMillis = millis();

  bool success = manageTanks();
  if (!success) {
    // An emergency situation has arisen, immediately exit the control loop
    return;
  };
  filterManagement();
  membraneFlushManagement();
}


// Manage the tanks
bool WaterSupplyController::manageTanks() {
  // Read all the depths
  if (mWaterSupplyTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (A02YYUW::checkLast5DepthSensorReadings(mWaterSupplyTankDepthSensor, false)) {
      triggerEmergency(ALARM_WATER_SUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return false;
    }
  }
  if (mWasteWaterTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (A02YYUW::checkLast5DepthSensorReadings(mWasteWaterTankDepthSensor, false)) {
      triggerEmergency(ALARM_WASTE_WATER_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return false;
    }
  }
  // Update the tank depths
  mWaterSupplyTankDepth = convertDistanceToDepth(mWaterSupplyTankDepthSensor->getDistance());
  mWasteWaterTankDepth = convertDistanceToDepth(mWasteWaterTankDepthSensor->getDistance());

  // If we're under the minimum depth for the water supply tank, then we shouldn't be filtering.
  if (mWaterSupplyTankDepth < WaterSupplyControllerNS::WARNING_TANK_MIN_DEPTH_MM) {
    triggerEmergency(ALARM_WATER_SUPPLY_TANK_EMPTY);
    return false;
  }
  // If we've gone over the maximum depth for the waste water tank, then we shouldn't be filtering.
  if (mWasteWaterTankDepth > WaterSupplyControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerEmergency(ALARM_WASTE_WATER_TANK_OVER_FULL);
    return false;
  }
  
  // Tanks are all good :)
  return true;
}

// Manage the filters
void WaterSupplyController::filterManagement() {
  // Update the water temperature reading (used to calculate TDS values)
  bool success = updateWaterTemperature();
  if (!success) {
    triggerAlarm(ALARM_WATER_SUPPLY_TANK_TEMPERATURE_SENSOR_COMMS_ERROR);
    // This isn't the end of the world, we'll just use the last temperature reading
  }
  // Update the TDS sensors
  mWaterSupplyTDSSensor->controlLoop();
  mPreROTDSensor->controlLoop();
  mOutputWaterTDSSensor->controlLoop();

  float waterSupplyTDSValue = getWaterSupplyTDSValue();
  float preROTDSValue = getPreROTDSValue();
  float outputWaterTDSValue = getOutputWaterTDSValue();

  if (waterSupplyTDSValue - preROTDSValue < WaterSupplyControllerNS::MIN_PRE_RO_FILTER_DELTA) {
    triggerAlarm(ALARM_PRE_RO_FILTERS_NEED_CHANGING);
  } else if (outputWaterTDSValue > WaterSupplyControllerNS::MAX_ACCEPTABLE_OUTPUT_TDS) {
    triggerAlarm(ALARM_RO_FILTERS_NEED_CHANGING);
  }
}

// Manage the flush schedule
void WaterSupplyController::membraneFlushManagement() {
  // If it's been a week since the last flush (and we're not in the middle of flushing already), then flush the system
  static unsigned long flushValveOpenTimeMillis = 0;
  if (!mFlushing && millis() - mLastFlushMillis > WaterSupplyControllerNS::MIN_FLUSH_INTERVAL_MILLIS) {
    mLastFlushMillis = millis();
    openFlushValve();
    flushValveOpenTimeMillis = mLastFlushMillis;
    mFlushing = true;
  }
  if (mFlushing && millis() - flushValveOpenTimeMillis > WaterSupplyControllerNS::FLUSH_VALVE_OPEN_INTERVAL_MILLIS) {
    closeFlushValve();
    mFlushing = false;
  }

}

// Open the water filter system flush valve
void WaterSupplyController::openFlushValve() {
  digitalWrite(mWaterFilterSystemFlushSolenoidControlPin, HIGH);
}

// Close the water filter system flush valve
void WaterSupplyController::closeFlushValve() {
  digitalWrite(mWaterFilterSystemFlushSolenoidControlPin, LOW);
}

// Control loop during emergency state
void WaterSupplyController::emergencyStateLoop() {
  // TODO: Get out of the emergency state
}

// Trigger an update of the water temperature reading. Returns false if the temperature sensor is disconnected.
bool WaterSupplyController::updateWaterTemperature() {
  float waterTemperature = mTemperatureSensors->getTempC(*mWaterSupplyTankTemperatureSensorAddress);
  if (waterTemperature == DEVICE_DISCONNECTED_C) {
    // If the temperature sensor is disconnected, report the failure
    return false;
  } else {
    mWaterTemperature = waterTemperature;
    return true;
  }
}

/* Called internally in the event of a sensor communication error - if we
* don't know how deep the tanks are, we shouldn't be moving anything around
*/
void WaterSupplyController::emergencyStop() {
  closeFlushValve();
}

/*******************************
 * Utilities
 *******************************/
// Converts a distance sensor reading into a tank depth
float WaterSupplyController::convertDistanceToDepth(float distance) {
  // Bound the distance as the sensor doesn't work under 30 mm - if it gets this close we're in trouble
  if (distance < 30.0f) distance = 30.0f;
  return WaterSupplyControllerNS::EMPTY_TANK_DISTANCE_MM - distance;
}

