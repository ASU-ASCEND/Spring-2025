#ifndef ENS160_SENSOR_H
#define ENS160_SENSOR_H

#include <Wire.h>

#include "Sensor.h"
#include "SparkFun_ENS160.h"

/**
 * @brief Implementation of a Sensor for the ENS160
 *
 */
class ENS160Sensor : public Sensor {
 private:
  SparkFun_ENS160 ens;

 public:
  ENS160Sensor();
  ENS160Sensor(unsigned long minium_period);
  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};

#endif