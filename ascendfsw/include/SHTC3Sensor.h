#ifndef SHTC3SENSOR_H
#define SHTC3SENSOR_H

#include "Adafruit_SHTC3.h"
#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for the SHTC3
 *
 */
class SHTC3Sensor : public Sensor {
 private:
  Adafruit_SHTC3 shtc3;
  float relative_humidity;

 public:
  SHTC3Sensor();
  SHTC3Sensor(unsigned long minimum_period);
  bool verify() override;
  String readData() override;

  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;

  float getRelHum();
};

#endif