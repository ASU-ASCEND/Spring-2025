#ifndef BME280_SENSOR_H
#define BME280_SENSOR_H

#include "Sensor.h"
#include "SparkFunBME280.h"

/**
 * @brief Implementation of a Sensor for the BME280
 *
 */
class BME280Sensor : public Sensor {
 private:
  BME280 bme;

 public:
  BME280Sensor();
  BME280Sensor(unsigned long minium_period);
  bool verify();
  String readData();

  //Function to integrate sensor data into a packet buffer.
  void readDataPacket(uint8_t*& packet);
};

#endif