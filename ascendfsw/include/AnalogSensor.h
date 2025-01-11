// Wind Sensor and temp

#ifndef ANALOGSENSOR_H
#define ANALOGSENSOR_H

#include <Arduino.h>
#include <string.h>

#include "Sensor.h"
// need to changed to the correct ones
#define WIND_PIN A0
#define PT1000_PIN A3

/**
 * @brief Implementation of a Sensor for the analog readings
 *
 */
class AnalogSensor : public Sensor {
 public:
  AnalogSensor();
  AnalogSensor(unsigned long minium_period);
  bool verify() override;
  String readData() override;
};

#endif