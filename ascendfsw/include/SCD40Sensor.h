#ifndef SCD40Sensor_H
#define SCD40Sensor_H

#include <Adafruit_Sensor.h>
#include <SensirionI2cScd4x.h>
#include <Wire.h>
#include <string.h>

#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @class SCD40Sensor
 * @brief A class to interface with the Sensirion SCD40 sensor for CO2 data
 * collection.
 *
 * The SCD40Sensor class is responsible for interacting with the SCD40 sensor to
 * gather CO2 data. It provides the CO2 concentration in parts per million (ppm)
 * and the temperature in degrees Celsius.
 *
 * This class inherits from the Sensor base class and overrides its virtual
 * methods to implement the specific functionality required to read and verify
 * data from the SCD40 sensor.
 *
 * Key functionalities:
 * - Verify sensor connection and setup.
 * - Retrieve sensor readings in CSV format for easy integration with data
 * logging systems.
 * - Get sensor name and CSV header for consistency in data handling.
 */
class SCD40Sensor : public Sensor {
 private:
  SensirionI2cScd4x scd;
  TwoWire* i2c_bus;

 public:
  SCD40Sensor(TwoWire* i2c_bus = &Wire);
  SCD40Sensor(unsigned long minimum_period, TwoWire* i2c_bus = &Wire);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};

#endif