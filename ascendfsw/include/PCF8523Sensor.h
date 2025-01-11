#ifndef PCF8523SENSOR_H
#define PCF8523SENSOR_H

#include <RTClib.h>

#include "Sensor.h"

class PCF8523Sensor : public Sensor {
 private:
  RTC_PCF8523 rtc;

 public:
  PCF8523Sensor();
  PCF8523Sensor(unsigned long minimum_period);
  bool verify();
  String readData();
  void calibrate();
};

#endif  // PCF8523SENSOR_H
