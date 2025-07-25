// See https://randomnerdtutorials.com/guide-for-ds18b20-temperature-sensor-with-arduino/ for original source
#include <Arduino.h>

#include <OneWire.h>
#include <DallasTemperature.h>

// The pin used for the OneWire bus
#define ONE_WIRE_BUS 4
// Setup a oneWire instance to communicate with any OneWire devices
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature. 
DallasTemperature sensors(&oneWire);

int numberOfDevices; // Number of temperature devices found
DeviceAddress tempDeviceAddress; // We'll use this variable to store a found device address

// function to print a device address
void printAddress(DeviceAddress deviceAddress) {
  for (uint8_t i = 0; i < 8; i++) {
    if (deviceAddress[i] < 16) Serial.print("0");
      Serial.print(deviceAddress[i], HEX);
  }
}

// function to print a device address in C++ pasteable format
void printAddressLongForm(DeviceAddress deviceAddress) {
  Serial.print("{");
  for (uint8_t i = 0; i < 8; i++) {
    if (i>0) Serial.print(", ");
    Serial.print("0x");  
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.print("}");
}

void setup() {
  // start serial port
  Serial.begin(115200);
  // Wait for initialisation of the serial interface
  while(!Serial);

  // Start up the library
  sensors.begin();
  
  // locate devices on the bus
  Serial.print("Locating devices...");
  // Grab a count of devices on the wire
  numberOfDevices = sensors.getDeviceCount();
  
  Serial.print("Found ");
  Serial.print(numberOfDevices, DEC);
  Serial.println(" devices.");

  // Loop through each device, print out address
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)) {
      Serial.print("Found device ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddressLongForm(tempDeviceAddress);
      Serial.println();
		} else {
		  Serial.print("Found ghost device at ");
		  Serial.print(i, DEC);
		  Serial.print(" but could not detect address. Check power and cabling");
		}
  }
}

void loop() {

  sensors.requestTemperatures(); // Send the command to get temperatures
  
  // Loop through each device, print out temperature data
  for(int i=0;i<numberOfDevices; i++) {
    // Search the wire for address
    if(sensors.getAddress(tempDeviceAddress, i)){
		
		// Output the device ID
		Serial.print("Temperature for device: ");
		Serial.println(i,DEC);

    // Print the data
    float tempC = sensors.getTempC(tempDeviceAddress);
    Serial.print("Temp C: ");
    Serial.print(tempC);
    Serial.print(" Temp F: ");
    Serial.println(DallasTemperature::toFahrenheit(tempC)); // Converts tempC to Fahrenheit
    } 	
  }
  delay(5000);
}
