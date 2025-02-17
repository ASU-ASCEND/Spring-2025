
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
/**
   * @brief Appends the SGP30 sensor data (eCO2, TVOC) plus a simple 16-bit checksum to the packet buffer.
   * 
   * @param packet Pointer to the current position in the packet buffer.
   * @return The number of bytes appended to the packet.
   */
  int readDataPacket(uint8_t*& packet) override;

  /**
   * @brief Decodes the SGP30 sensor data (eCO2, TVOC) plus checksum from the packet buffer into a CSV string.
   *
   * @param packet Pointer to the packet buffer.
   * @return A CSV string in the form "eCO2,TVOC,Checksum: X (computed: Y),".
   */
  String decodeToCSV(uint8_t*& packet) override;
};
#endif
