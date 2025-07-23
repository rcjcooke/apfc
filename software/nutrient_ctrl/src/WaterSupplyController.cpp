#include "WaterSupplyController.hpp"

using namespace apfc;

/************************
 * Constants
 ************************/
namespace WaterSupplyControllerNS {

  // The distance measured for a completely empty tank
  static const float EMPTY_TANK_DISTANCE_MM = 150.0f;
  
  // Depth above which runoff recycling gets transferred back to the mixing tank
  static const float RUNOFF_RECYCLING_TANK_TRANSFER_DEPTH_MM = 50.0f;
  // Minimum depth for the runoff recycling tank
  static const float RUNOFF_RECYCLING_MIN_TANK_DEPTH_MM = 10.0f;
  // The minimum depth available across the mixing and runoff recycling tanks
  static const float IN_SYSTEM_SOLUTION_MIN_DEPTH_MM = 50.0f;
  // The maximum depth that should be available across the mixing and runoff recycling tanks
  static const float IN_SYSTEM_SOLUTION_MAX_DEPTH_MM = 90.0f;

  // The minimum depth the irrigationsupply tank should be allowed to get to
  static const float IRRIGATIONSUPPLY_TANK_MIN_DEPTH_MM = 40.0f;
  // The operating minimum depth of the irrigationsupply tank. If the depth goes below this figure, it should be topped up
  static const float IRRIGATIONSUPPLY_TANK_OPS_MIN_DEPTH_MM = 60.0f;
  // The operating maximum depth of the irrigationsupply tank. This is the depth to which the tank should be topped up when necessary
  static const float IRRIGATIONSUPPLY_TANK_OPS_MAX_DEPTH_MM = 80.0f;
  // At this depth, there's too much in the tank
  static const float WARNING_TANK_MAX_DEPTH_MM = 100.0f;
  // At this depth, there's too little in the tank
  static const float WARNING_TANK_MIN_DEPTH_MM = 100.0f;

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
    uint8_t outputWaterTDSDataPin) 
      : StandardController('W', 4), 
        mWaterFilterSystemFlushSolenoidControlPin(waterFilterSystemFlushSolenoidControlPin) {

  // Sort out the control pins
  pinMode(mWaterFilterSystemFlushSolenoidControlPin, OUTPUT);

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
  if (checkLast5DepthSensorReadings(mWaterSupplyTankDepthSensor, true)) return;
  if (checkLast5DepthSensorReadings(mWasteWaterTankDepthSensor, true)) return;

  // If we've got this far then transition to a running state
  setRunState(ControllerRunState::RUNNING);

}

// Control loop during running state
void WaterSupplyController::runningStateLoop() {

  // Read all the depths
  if (mWaterSupplyTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (checkLast5DepthSensorReadings(mWaterSupplyTankDepthSensor, false)) {
      triggerEmergency(ALARM_WATER_SUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return;
    }
  }
  if (mWasteWaterTankDepthSensor->readDistance() < 0) {
    // If there are 5 bad readings in a row then throw toys out the pram - the occasional bad reading isn't an issue
    if (checkLast5DepthSensorReadings(mWasteWaterTankDepthSensor, false)) {
      triggerEmergency(ALARM_WASTE_WATER_TANK_DEPTH_SENSOR_COMMS_ERROR);
      return;
    }
  }
  
  // Update the tank depths
  mWaterSupplyTankDepth = convertDistanceToDepth(mWaterSupplyTankDepthSensor->getDistance());
  mWasteWaterTankDepth = convertDistanceToDepth(mWasteWaterTankDepthSensor->getDistance());

  // If we're under the minimum depth for the water supply tank, then we shouldn't be filtering.
  if (mWaterSupplyTankDepth < WaterSupplyControllerNS::WARNING_TANK_MIN_DEPTH_MM) {
    triggerEmergency(ALARM_WATER_SUPPLY_TANK_EMPTY);
  } 
  // If we've gone over the maximum depth for the waste water tank, then we shouldn't be filtering.
  if (mWasteWaterTankDepth > WaterSupplyControllerNS::WARNING_TANK_MAX_DEPTH_MM) {
    triggerEmergency(ALARM_WASTE_WATER_TANK_OVER_FULL);
  }

  // Update the water temperature reading (used to calculate TDS values)
  // mWaterTemperature = mSupplyTankThermometer->getTemperature();
  // Update the TDS sensors
  mWaterSupplyTDSSensor->controlLoop();
  mPreROTDSensor->controlLoop();
  mOutputWaterTDSSensor->controlLoop();

  float waterSupplyTDSValue = getWaterSupplyTDSValue();

}

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
