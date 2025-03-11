#ifndef BMP384_SENSOR_H
#define BMP384_SENSOR_H

#include "Adafruit_BMP3XX.h"
#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for BMP384 Pressure and Temperature sensor
 *
 */
class BMP390Sensor : public Sensor {
 private:
  Adafruit_BMP3XX bmp;

 public:
  BMP390Sensor();
  BMP390Sensor(unsigned long minium_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};

#endif  // BMP384_SENSOR_H