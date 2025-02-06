#ifndef AS7331SENSOR_H
#define AS7331SENSOR_H

#include <SparkFun_AS7331.h>
#include <Wire.h>
#include <string.h>

#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for UVA/B/C sensor
 *
 */
class AS7331Sensor : public Sensor {
 private:
  SfeAS7331ArdI2C myUVSensor;
  uint8_t i2c_addr;

 public:
  AS7331Sensor(uint8_t i2c_addr);
  AS7331Sensor(unsigned long minimum_period, uint8_t i2c_addr);

  bool verify() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);
  String readData() override;
};

#endif