#ifndef SHT31SENSOR_H
#define SHT31SENSOR_H

#include <Arduino.h>
#include <Wire.h>
#include <string.h>

#include "Adafruit_SHT31.h"
#include "Sensor.h"

/**
 * @class SHT31Sensor
 * @brief Interfaces with SHT31 Sensor to collect temperature and humidity data
 *
 */
class SHT31Sensor : public Sensor {
 private:
  Adafruit_SHT31 sht31;
  String csvHeaderCompiled = "SHT31 Hum %, SHT31 Temp C, ";

 public:
  SHT31Sensor();
  SHT31Sensor(unsigned long minimum_period);

  bool verify() override;
  arduino::String readData() override;
};

#endif
