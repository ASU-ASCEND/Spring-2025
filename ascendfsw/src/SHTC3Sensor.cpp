#include "SHTC3Sensor.h"

/**
 * @brief Default constructor for the SHTC3Sensor class.
 *
 * Initializes the sensor object with a default minimum period of 0
 * milliseconds.
 */
SHTC3Sensor::SHTC3Sensor(TwoWire* i2c_bus) : SHTC3Sensor(0, i2c_bus) {}

/**
 * @brief Construct a new SHTC3Sensor object.
 * @param minium_period Minimum time to wait between readings in ms
 */
SHTC3Sensor::SHTC3Sensor(unsigned long minimum_period, TwoWire* i2c_bus)
    : Sensor("SHTC3", "Temp(C), Humidity(%)", 2, minimum_period) {
  this->relative_humidity = 0.0;
  this->i2c_bus = i2c_bus;
  if (this->i2c_bus == &Wire1) this->device_name += "_1";
}

/**
 * @brief Returns if sensor is connected and working.
 *
 * @return true if sensor is connected and working.
 * @return false if sensor is not.
 */
bool SHTC3Sensor::verify() {
  shtc3 = Adafruit_SHTC3();
  return shtc3.begin(this->i2c_bus);
}

/**
 * @brief Reads temperature in Celcius and relative humidity in % from SHTC3.
 *
 * @return String Temp (C) and Humidity (%) in csv format.
 */
String SHTC3Sensor::readData() {
  sensors_event_t temp, humidity;
  shtc3.getEvent(&humidity, &temp);

  this->relative_humidity = humidity.relative_humidity;

  return String(temp.temperature) + "," + String(humidity.relative_humidity) +
         ",";
}

/**
 * @brief Copies data to the packet
 *
 * @param packet Pointer to copy at
 */
void SHTC3Sensor::readDataPacket(uint8_t*& packet) {
  sensors_event_t temp, humidity;
  shtc3.getEvent(&humidity, &temp);

  this->relative_humidity = humidity.relative_humidity;

  memcpy(packet, &(temp.temperature), sizeof(temp.temperature));
  packet += sizeof(temp.temperature);

  memcpy(packet, &(humidity.relative_humidity),
         sizeof(humidity.relative_humidity));
  packet += sizeof(humidity.relative_humidity);
}

/**
 * @brief Decodes data from the packet
 *
 * @param packet Pointer to decode at
 * @return String CSV stub Temperature, Humidity
 */
String SHTC3Sensor::decodeToCSV(uint8_t*& packet) {
  float temperature = 0;
  float humidity = 0;

  memcpy(&temperature, packet, sizeof(temperature));
  packet += sizeof(temperature);

  memcpy(&humidity, packet, sizeof(humidity));
  packet += sizeof(humidity);

  return String(temperature) + "," + String(humidity) + ",";
}

/**
 * @brief Getter for relative humidity
 *
 * @return float Relative humidity as %
 */
float SHTC3Sensor::getRelHum() { return this->relative_humidity; }