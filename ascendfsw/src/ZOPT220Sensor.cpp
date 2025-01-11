#include "ZOPT220Sensor.h"

/**
 * @brief Default constructor for the ZOPT220Sensor class.
 *
 * Initializes the ZOPT220 sensor object with a default minimum period of 0
 * milliseconds by calling the parameterized constructor with 0.
 */
ZOPT220Sensor::ZOPT220Sensor() : ZOPT220Sensor(0) {}

/**
 * @brief Parameterized constructor for the ZOPT220Sensor class.
 *
 * Initializes the sensor object with a specified minimum period and sets up the
 * sensor name, CSV header, and the number of data fields (2: UV index and
 * ambient light).
 *
 * @param minimum_period The minimum time period between sensor readings, in
 * milliseconds.
 */
ZOPT220Sensor::ZOPT220Sensor(unsigned long minimum_period)
    : Sensor("ZOPT220", "ZOPTUV,ZOPTLight(lx),", 2, minimum_period) {}

/**
 * @brief Verifies the connection to the ZOPT220x sensor.
 *
 * This function checks whether the ZOPT220x sensor is correctly initialized
 * and ready for use by calling `zopt220xSetup()`.
 *
 * @return true If the sensor is successfully verified.
 * @return false If the sensor fails to initialize.
 */
bool ZOPT220Sensor::verify() { return this->zopt.zopt220xSetup(); }

/**
 * @brief Reads data from the ZOPT220x sensor and returns it in CSV format.
 *
 * This function collects the UV index and ambient light level (in lux) from the
 * sensor. It configures the sensor resolution, gain, and enables the respective
 * sensing modes. A delay is introduced to ensure data availability, and the
 * data is then formatted into a CSV string.
 *
 * @return String A string containing the sensor readings in the
 * format: "UV Index, Ambient Light (lux), ".
 */
String ZOPT220Sensor::readData() {
  // UV index
  this->zopt.setResolution(2);
  this->zopt.setGain(4);
  this->zopt.enableUVBSensing();

  // Might need delay
  float uvIndex = this->zopt.getUVIndex();

  // ALS - Ambient Light
  this->zopt.setResolution(2);
  this->zopt.setGain(1);
  this->zopt.enableALSSensing();

  // Ensure data is available before reading
  int it = 0;
  while (!this->zopt.dataAvailable() && it < 40) {
    delay(10);
    it++;
  }
  long als = this->zopt.getALS();

  return String(uvIndex) + "," + String(als) + ",";
}
