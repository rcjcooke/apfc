// Add in weekly 2-3 minute flush (need to add solenoid in parallel to manual flush valve) - This is to ensure that the RO membranes are flushed to remove impurities and bacteria left on the surface of the membrane.
// Add TDS sensors to detect when the filters and membrane need to be replaced

#ifndef __WATERSUPPLYCONTROLLER_H_INCLUDED__
#define __WATERSUPPLYCONTROLLER_H_INCLUDED__

#include <Arduino.h>
#include <SPI.h>

#include <MULTIUART.hpp>
#include <MUARTSingleStream.hpp>
#include <A02YYUWviaUARTStream.hpp>
#include <CQRobotOceanTDSSensor.hpp>

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
      uint8_t outputWaterTDSDataPin);

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

    /*******************************
     * Actions
     *******************************/
    // Called every microcontroller main program loop - does everything at runtime
    void controlLoop();

    // // Turn on the Runoff recycling Tank Pump
    // void turnOnRunoffRecyclingPump();
    // // Turn on the Runoff recycling Tank Pump
    // void turnOffRunoffRecyclingPump();
    // // Turn on the UV Steriliser
    // void turnOnUV();
    // // Turn off the UV Steriliser
    // void turnOffUV();

  private:

    /*******************************
     * Member variables
     *******************************/
    // The microcontroller pin that controls the water filter system flush solenoid
    uint8_t mWaterFilterSystemFlushSolenoidControlPin;
    
    // The current water supply tank temperature / C
    float mWaterTemperature = 0.0f;
    // The current water supply tank depth / mm
    float mWaterSupplyTankDepth = 0.0f;
    // The current waste water tank depth / mm
    float mWasteWaterTankDepth = 0.0f;

    /** Depth sensors */
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
    
    /*******************************
     * Actions
     *******************************/
    // Control loop during startup state
    void startupStateLoop() override;
    // Control loop during running state
    void runningStateLoop() override;
    // Control loop during emergency state
    void emergencyStateLoop() override;

    /* Called internally in the event of a sensor communication error - if we
     * don't know how deep the tanks are, we shouldn't be moving anything around
     */
    void emergencyStop();

    /*******************************
     * Utilities
     *******************************/
    // Converts a distance sensor reading into a tank depth
    float convertDistanceToDepth(float distance);
    /* Checks the last 5 readings from the sensor. If (allGood) then returns true
    * if all readings are good. If (!allGood) then returns true if every reading
    * is bad. */
    bool checkLast5DepthSensorReadings(A02YYUW::A02YYUWviaUARTStream *sensor, bool allGood);

  };

}

#endif // __WATERSUPPLYCONTROLLER_H_INCLUDED__
