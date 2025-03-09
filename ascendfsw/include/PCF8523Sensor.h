#ifndef PCF8523SENSOR_H
#define PCF8523SENSOR_H

#include <RTClib.h>

#include "Sensor.h"

/**
 * @brief Implementation of the PCF8523 sensor
 *
 */

class PCF8523Sensor : public Sensor {
 private:
  RTC_PCF8523 rtc;

 public:
  PCF8523Sensor();
  PCF8523Sensor(unsigned long minimum_period);

  bool verify() override;
  String decodeToCSV(uint8_t*& packet) override;
  void readDataPacket(uint8_t*& packet) override;
  String readData() override;
  void calibrate();
};

#endif  // PCF8523SENSOR_H
