#include "TMP117Sensor.h"

/**
 * @brief Construct a new TMP117Sensor object with default minimum_period of 0
 * ms.
 *
 */
TMP117Sensor::TMP117Sensor(TwoWire* i2c_bus) : TMP117Sensor(0, i2c_bus) {}

/**
 * @brief Construct a new TMP117Sensor object.
 *
 * @param minimum_period Minimum time to wait between readings in ms
 */
TMP117Sensor::TMP117Sensor(unsigned long minimum_period, TwoWire* i2c_bus)
    : Sensor("TMP117", "TMP117Temp(C)", 1, minimum_period) {
  this->tempC = 0.0;
  this->i2c_bus = i2c_bus;
  if (this->i2c_bus == &Wire1) this->device_name += "1";
}

/**
 * @brief Returns if sensor is connected and working.
 *
 * @return true if sensor is connected and working.
 * @return false if sensor is not.
 */
bool TMP117Sensor::verify() {
  this->i2c_bus->begin();
  return tmp.begin(TMP117_I2C_ADDR, *(this->i2c_bus));
}

/**
 * @brief Reads temperature in Celcius from TMP117.
 *
 * @return String Temp (C) in csv format.
 */
String TMP117Sensor::readData() {
  this->tempC = (float)tmp.readTempC();

  return String(this->tempC) + ",";
}

/**
 * @brief Reads sensor data (Temp (C)) and appends to the passed uint8_t array
 * pointer, incrementing it.
 *
 * @param packet Pointer to the packet byte array.
 */
void TMP117Sensor::readDataPacket(uint8_t*& packet) {
  this->tempC = (float)tmp.readTempC();

  std::copy((uint8_t*)(&tempC), (uint8_t*)(&tempC) + sizeof(tempC), packet);

  packet += sizeof(this->tempC);
}

/**
 * @brief Decodes sensor data from the packet into a CSV string.
 *
 * @param packet  Pointer to packet byte array that will be decoded.
 * @return String Decoded sensor data in CSV format.
 */
String TMP117Sensor::decodeToCSV(uint8_t*& packet) {
  float tempC;
  memcpy(&tempC, packet, sizeof(float));

  packet += sizeof(float);

  return String(tempC) + ",";
}

/**
 * @brief Getter for tempC
 *
 * @return float Last recorded temperature in Celsius
 */
float TMP117Sensor::getTempC() { return this->tempC; }