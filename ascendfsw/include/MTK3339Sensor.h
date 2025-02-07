#ifndef MTK3339SENSOR_H
#define MTK3339SENSOR_H

#include <Adafruit_GPS.h>
#include <Arduino.h>
#include <SPI.h>
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
  Adafruit_GPS GPS;

 public:
  MTK3339Sensor();  // pass pin in constructor
  MTK3339Sensor(unsigned long minimum_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);
};

#endif