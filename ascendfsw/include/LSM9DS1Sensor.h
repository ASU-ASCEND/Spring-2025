#ifndef LSM9DS1SENSOR_H
#define LSM9DS1SENSOR_H

#include <Adafruit_LSM9DS1.h>  // Updated to use the Adafruit library
#include <Adafruit_Sensor.h>
#include <string.h>

#include "PayloadConfig.h"
#include "Sensor.h"

/**
 * @brief Implementation of the Sensor class for the LSM9DS1 (9-axis IMU)
 *
 */
class LSM9DS1Sensor : public Sensor {
 private:
  Adafruit_LSM9DS1 lsm;  // Using Adafruit_LSM9DS1 instead of SparkFunLSM9DS1

  void setupSensor();      // Function to set up sensor ranges and data rates
  float accel_offsets[3];  // Offsets for accelerometer calibration
  float gyro_offsets[3];   // Offsets for gyroscope calibration
  float mag_offsets[3];    // Offsets for magnetometer calibration

 public:
  LSM9DS1Sensor();
  LSM9DS1Sensor(unsigned long minimum_period);
  bool verify() override;
  String readData() override;
  void calibrate();  // calibration function
};

#endif