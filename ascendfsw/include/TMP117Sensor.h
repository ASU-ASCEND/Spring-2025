#ifndef TMP117_SENSOR_H
#define TMP117_SENSOR_H

#include <SparkFun_TMP117.h>
#include <Wire.h>

#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for the TMP117
 *
 */
class TMP117Sensor : public Sensor {
 private:
  TMP117 tmp;

 public:
  TMP117Sensor();
  TMP117Sensor(unsigned long minium_period);
  bool verify();
  String readData();

  // Function to integrate sensor data into a packet buffer.
  void readDataPacket(uint8_t*& packet);

  // Function to decode sensor data from a packet and return a CSV string
  String decodeToCSV(uint8_t*& packet);
};

#endif