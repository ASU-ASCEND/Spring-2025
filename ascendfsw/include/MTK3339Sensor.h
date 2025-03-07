#ifndef MTK3339SENSOR_H
#define MTK3339SENSOR_H

#include <Arduino.h>
#include <SparkFun_u-blox_GNSS_Arduino_Library.h>
#include <Wire.h>

#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @brief Implementation of the Adafruit MTK3339 GPS sensor
 *
 */
class MTK3339Sensor : public Sensor {
 private:
  // int pin; chip select pin
  SFE_UBLOX_GNSS GPS;

 public:
  MTK3339Sensor();  // pass pin in constructor
  MTK3339Sensor(unsigned long minimum_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);
};

#endif