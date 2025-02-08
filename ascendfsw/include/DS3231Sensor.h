#ifndef DS3231SENSOR_H
#define DS3231SENSOR_H

#include <RTClib.h>

#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for the DS3231 Real Time Clock
 *
 */
class DS3231Sensor : public Sensor {
 private:
  RTC_DS3231 rtc;

 public:
  DS3231Sensor();
  DS3231Sensor(unsigned long minimum_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet);
  String decodeToCSV(uint8_t*& packet);

  void setTime(int year, int month, int day, int hour, int minute, int second);
};

#endif