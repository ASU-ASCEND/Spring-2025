#ifndef ICM20948SENSOR_H
#define ICM20948SENSOR_H

#include <Adafruit_ICM20948.h>
#include <Adafruit_ICM20X.h>
#include <Adafruit_Sensor.h>
#include <string.h>

#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @brief Implementation of the Sensor class for the Adafruit ICM20948 (9-axis
 * IMU)
 *
 */
class ICM20948Sensor : public Sensor {
 private:
  Adafruit_ICM20948 icm;
  Adafruit_Sensor *icm_accel, *icm_gyro, *icm_mag, *icm_temp;

 public:
  ICM20948Sensor();
  ICM20948Sensor(unsigned long minimum_period);
  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);
};

#endif