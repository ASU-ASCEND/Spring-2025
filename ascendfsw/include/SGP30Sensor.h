
#ifndef SGP30SENSOR_H
#define SGP30SENSOR_H

#include <Arduino.h>
#include <stdint.h>
#include <string.h>

#include "Adafruit_SGP30.h"
#include "Sensor.h"

/**
 * @brief Implementation of a Sensor for the SGP Air Quality sensor
 *
 */
class SGP30Sensor : public Sensor {
 private:
  Adafruit_SGP30 sgp;

 public:
  SGP30Sensor();
  SGP30Sensor(unsigned long minimum_period);
  bool verify() override;
  String readData() override;

  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};
#endif
