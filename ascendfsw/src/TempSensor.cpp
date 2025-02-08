#include "TempSensor.h"

/**
 * @brief Construct a new Temp Sensor object with default minimum_period of 0 ms
 *
 */
TempSensor::TempSensor() : TempSensor(0) {}

/**
 * @brief Construct a new Temp Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
TempSensor::TempSensor(unsigned long minium_period)
    : Sensor("PicoTemp", "PicoTemp(C),", 1, minium_period) {}

/**
 * @brief Returns if sensor can be reached, the temperature sensor is on the
 * pico so it is assumed to be always true, implemented only for compatability
 * with parent
 *
 * @return true always
 * @return false never
 */
bool TempSensor::verify() { return true; }

/**
 * @brief Reads temperature data in Celsius
 *
 * @return String Temperature data in CSV format
 */
String TempSensor::readData() { return String(analogReadTemp()) + ","; }

void TempSensor::readDataPacket(uint8_t*& packet) {
  // read sensor value
  float temp = analogReadTemp();

  // cast the pointer to that value to a uint8_t pointer, copy the bytes at that
  // pointer to packet
  std::copy((uint8_t*)(&temp), (uint8_t*)(&temp) + sizeof(temp), packet);
  // increment the packet pointer past the new bytes
  packet += sizeof(temp);
}

String TempSensor::decodeToCSV(uint8_t*& packet) {
  // cast the packet pointer to pointer of the data type to read (float) then
  // dereference it
  float temp;  // = *((float*)packet);
  memcpy(&temp, packet, sizeof(float));

  // increment packet by the size of the read data type (float)
  packet += sizeof(float);

  // return in csv snippet format
  return String(temp) + ",";
}