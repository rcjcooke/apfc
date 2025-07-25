#ifndef __WATERSUPPLYCONTROLLER_H_INCLUDED__
#define __WATERSUPPLYCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SPI.h>

#include <MULTIUART.hpp>
#include <MUARTSingleStream.hpp>
#include <A02YYUWviaUARTStream.hpp>
#include <CQRobotOceanTDSSensor.hpp>
#include <OneWire.h>
#include <DallasTemperature.h>

#include "StandardController.hpp"

namespace apfc {

  class WaterSupplyController : public StandardController {

  public:

    /*******************************
     * Constants
     *******************************/
    // Alarm code triggered if there are communication problems with the water supply tank depth sensor
    static const int ALARM_WATER_SUPPLY_TANK_DEPTH_SENSOR_COMMS_ERROR = 1;
    // Alarm code triggered if there are communication problems with the waste water tank depth sensor
    static const int ALARM_WASTE_WATER_TANK_DEPTH_SENSOR_COMMS_ERROR = 2;

    // Alarm code triggered when water supply tank is empty
    static const int ALARM_WATER_SUPPLY_TANK_EMPTY = 3;
    // Alarm code triggered when waste water tank is too full
    static const int ALARM_WASTE_WATER_TANK_OVER_FULL = 4;
    // Alarm code triggered if there are communication problems with the water supply tank temperature sensor
    static const int ALARM_WATER_SUPPLY_TANK_TEMPERATURE_SENSOR_COMMS_ERROR = 5;

    // Alarm code triggered if the pre-RO filters need changing
    static const int ALARM_PRE_RO_FILTERS_NEED_CHANGING = 6;
    // Alarm code triggered if the RO filters need changing
    static const int ALARM_RO_FILTERS_NEED_CHANGING = 7;

    /*******************************
     * Constructors
     *******************************/
    WaterSupplyController(
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
      const DeviceAddress* waterSupplyTankTemperatureSensorAddress);

    /*******************************
     * Getters / Setters
     *******************************/
    // Get the current water supply tank depth / mm
    float getWaterSupplyTankDepth() const;
    // Get the current waste water tank depth / mm
    float getWasteWaterTankDepth() const;

    // Get the current water supply tank temperature / C
    float getWaterSupplyTemperature() const;
    
    // Get the water supply tank TDS value / ppm
    float getWaterSupplyTDSValue() const;
    // Get the pre-RO water TDS value / ppm
    float getPreROTDSValue() const;
    // Get the filtered water TDS value / ppm
    float getOutputWaterTDSValue() const;

    // For Debug purposes: Get the depth sensor instance for the Water Supply Tank
    A02YYUW::A02YYUWviaUARTStream* getWaterSupplyTankDepthSensor();
    // For Debug purposes: Get the depth sensor instance for the Waste Water Tank
    A02YYUW::A02YYUWviaUARTStream* getWasteWaterTankDepthSensor();
    // For Debug purposes: Get the TDS sensor for the water supply
    CQRobotOceanTDS::CQRobotOceanTDSSensor* getWaterSupplyTDSSensor();
    // For Debug purposes: Get the sensor measuring the pre-RO water TDS
    CQRobotOceanTDS::CQRobotOceanTDSSensor* getPreROTDSSensor();
    // For Debug purposes: Get the sensor measuring the filtered water TDS
    CQRobotOceanTDS::CQRobotOceanTDSSensor* getOutputWaterTDSSensor();

    // Open the water filter system flush valve
    void openFlushValve();
    // Close the water filter system flush valve
    void closeFlushValve();

  private:

    /*******************************
     * Member variables
     *******************************/
    // The last time the control loop was run / ms since last reset
    unsigned long mLastControlLoopMillis = 0;
    // The last time the water filter system was flushed / ms since last reset
    unsigned long mLastFlushMillis = 0;

    // The microcontroller pin that controls the water filter system flush solenoid
    uint8_t mWaterFilterSystemFlushSolenoidControlPin;
    // Whether the water filter system is currently flushing
    bool mFlushing = false;
    
    // The current water supply tank temperature / C
    float mWaterTemperature = 0.0f;
    // The current water supply tank depth / mm
    float mWaterSupplyTankDepth = 0.0f;
    // The current waste water tank depth / mm
    float mWasteWaterTankDepth = 0.0f;

    // The water supply tank depth sensor
    A02YYUW::A02YYUWviaUARTStream* mWaterSupplyTankDepthSensor{ nullptr };
    // The waste water tank depth sensor
    A02YYUW::A02YYUWviaUARTStream* mWasteWaterTankDepthSensor{ nullptr };
    // // The water supply tank thermometer
    // auto mSupplyTankThermometer*{ nullptr };
    // The TDS sensor for the water supply
    CQRobotOceanTDS::CQRobotOceanTDSSensor* mWaterSupplyTDSSensor{ nullptr };
    // The TDS sensor measuring the pre-RO water
    CQRobotOceanTDS::CQRobotOceanTDSSensor* mPreROTDSensor{ nullptr };
    // The TDS sensor measuring the filtered water
    CQRobotOceanTDS::CQRobotOceanTDSSensor* mOutputWaterTDSSensor{ nullptr };
    // The interface to all temperature sensors  
    DallasTemperature* mTemperatureSensors;
    // The address of the water supply tank temperature sensor
    const DeviceAddress* mWaterSupplyTankTemperatureSensorAddress;
    
    /*******************************
     * Actions
     *******************************/
    // Control loop during startup state
    void startupStateLoop() override;
    // Control loop during running state
    void runningStateLoop() override;
    // Control loop during emergency state
    void emergencyStateLoop() override;

    // Manage the tanks
    bool manageTanks();
    // Manage the filters
    void filterManagement();
    // Manage the flush schedule
    void membraneFlushManagement();
    // Trigger an update of the water temperature reading. Returns false if the temperature sensor is disconnected.
    bool updateWaterTemperature();

    /* Called internally in the event of a sensor communication error - if we
     * don't know how deep the tanks are, we shouldn't be moving anything around
     */
    void emergencyStop();

    /*******************************
     * Utilities
     *******************************/
    // Converts a distance sensor reading into a tank depth
    float convertDistanceToDepth(float distance);
  };

}

#endif // __WATERSUPPLYCONTROLLER_H_INCLUDED__
