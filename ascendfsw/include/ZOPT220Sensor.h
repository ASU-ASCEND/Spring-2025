#ifndef ZOPT220SENSOR_H
#define ZOPT220SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "Sensor.h"
#include "ZOPT220x_Control.h"

/**
 * @class ZOPT220Sensor
 * @brief A class to interface with the ZOPT220x UV and ambient light sensor.
 *
 * The ZOPT220Sensor class provides functionality to interact with the ZOPT220x
 * sensor, allowing it to read UV index and ambient light levels (lux). This
 * class inherits from the Sensor base class and overrides the `verify()` and
 * `readData()` methods to handle the sensor’s specific functionality.
 *
 * Key functionalities:
 * - Verifies and initializes the connection with the ZOPT220x sensor.
 * - Reads UV index and ambient light data from the sensor and returns it in CSV
 * format.
 */
class ZOPT220Sensor : public Sensor {
 private:
  ZOPT220x_Control zopt;

 public:
  ZOPT220Sensor();
  ZOPT220Sensor(unsigned long mimimum_period);

  bool verify() override;
  arduino::String readData() override;
};

#endif