#ifndef INA260SENSOR_H
#define INA260SENSOR_H

#include <Arduino.h>
#include <string.h>

#include "Adafruit_INA260.h"
#include "Sensor.h"

/**
 * @class INA260Sensor
 * @brief A class to interface with the Adafruit INA260 sensor for current,
 * voltage, and power measurements.
 *
 * The INA260Sensor class is responsible for interacting with the Adafruit
 * INA260 sensor to gather electrical data, including current (in mA), voltage
 * (in mV), and power (in mW). This class inherits from the Sensor base class
 * and overrides the `verify()` and `readData()` methods to provide specific
 * functionality for the INA260 sensor.
 *
 * Key functionalities:
 * - Initializes and verifies the connection with the INA260 sensor.
 * - Reads current, voltage, and power data from the sensor and returns it in
 * CSV format.
 */
class INA260Sensor : public Sensor {
 private:
  Adafruit_INA260 ina260;

 public:
  INA260Sensor();
  INA260Sensor(unsigned long minimum_period);
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);

  bool verify() override;
  String readData() override;
};

#endif
