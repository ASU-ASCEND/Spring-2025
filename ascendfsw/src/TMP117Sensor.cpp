#include "TMP117Sensor.h"

/**
 * @brief Construct a new TMP117Sensor object with default minimum_period of 0
 * ms.
 *
 */
TMP117Sensor::TMP117Sensor() : TMP117Sensor(0) {}

/**
 * @brief Construct a new TMP117Sensor object.
 *
 * @param minimum_period Minimum time to wait between readings in ms
 */
TMP117Sensor::TMP117Sensor(unsigned long minimum_period)
    : Sensor("TMP117", "TMP117Temp(C)", 1, minimum_period) {}

/**
 * @brief Returns if sensor is connected and working.
 *
 * @return true if sensor is connected and working.
 * @return false if sensor is not.
 */
bool TMP117Sensor::verify() {
  Wire.begin();
  return tmp.begin();
}

/**
 * @brief Reads temperature in Celcius from TMP117.
 *
 * @return String Temp (C) in csv format.
 */
String TMP117Sensor::readData() {
  float tempC = tmp.readTempC();
  return String(tempC) + ",";
}

/**
 * @brief Reads sensor data (Temp (C)) and appends to the passed uint8_t array
 * pointer, incrementing it.
 *
 * @param packet Pointer to the packet byte array.
 */
void TMP117Sensor::readDataPacket(uint8_t*& packet) {
  float tempC = tmp.readTempC();

  std::copy((uint8_t*)(&tempC), (uint8_t*)(&tempC) + sizeof(tempC), packet);

  packet += sizeof(tempC);
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
