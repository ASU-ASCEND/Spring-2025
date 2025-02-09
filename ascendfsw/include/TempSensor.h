#ifndef TEMP_SENSOR_H
#define TEMP_SENSOR_H

#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for the onboard temperature sensor
 *
 */
class TempSensor : public Sensor {
 private:
 public:
  TempSensor();
  TempSensor(unsigned long minium_period);
  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);
};

#endif