#ifndef SCD41SENSOR_H
#define SCD41SENSOR_H

#include <SensirionI2cScd4x.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>
#include <string.h>

#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @class SCD41Sensor
 * @brief A class to interface with the Sensirion SCD41 sensor for CO2 data
 * collection.
 *
 * The SCD41Sensor class is responsible for interacting with the SCD41 sensor to
 * gather CO2 data. It provides the CO2 concentration in parts per million (ppm)
 * and the temperature in degrees Celsius.
 *
 * This class inherits from the Sensor base class and overrides its virtual
 * methods to implement the specific functionality required to read and verify
 * data from the SCD41 sensor.
 *
 * Key functionalities:
 * - Verify sensor connection and setup.
 * - Retrieve sensor readings in CSV format for easy integration with data logging
 * systems.
 * - Get sensor name and CSV header for consistency in data handling.
 */
class SCD41Sensor : public Sensor {
 private:
 SensirionI2cScd4x scd;

 public:
  SCD41Sensor();
  SCD41Sensor(unsigned long minimum_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};

#endif