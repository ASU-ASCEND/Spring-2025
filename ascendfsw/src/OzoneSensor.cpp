#include "OzoneSensor.h"

/**
 * @brief Construct a new Ozone Sensor object with default minimum_period of 0
 * ms
 *
 */
OzoneSensor::OzoneSensor() : OzoneSensor(0) {}

/**
 * @brief Construct a new Ozone Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
OzoneSensor::OzoneSensor(unsigned long minium_period)
    : Sensor("OzoneSensor", "O3 PPB, ", minium_period) {}

/**
 * @brief Tests if the Ozone sensor can be reached
 *
 * @return true
 * @return false
 */
bool OzoneSensor::verify() {
  if (this->ozone.begin(OZONE_I2C_ADDR) == false) {
    return false;
  }

  ozone.setModes(MEASURE_MODE_PASSIVE);

  return true;
}

/**
 * @brief Reads ozone concentration data in PPB
 *
 * @return String CSV stub of O3 PPB
 */
String OzoneSensor::readData() {
  return String(this->ozone.readOzoneData()) + ",";
}

/**
 * @brief Reads ozone concentration data in PPB to packet
 *
 * @param packet The packet to copy data into
 */
void OzoneSensor::readDataPacket(uint8_t*& packet) {
  int16_t ozone_val = this->ozone.readOzoneData();
  memcpy(packet, &ozone_val, sizeof(int16_t));
  packet += sizeof(int16_t);
}

/**
 * @brief Decodes ozone concentration data from a packet to CSV
 *
 * @param packet The packet to decode from
 * @return String CSV stub of O3 PPB
 */
String OzoneSensor::decodeToCSV(uint8_t*& packet) {
  int16_t ozone_val = 0;
  memcpy(&ozone_val, packet, sizeof(int16_t));
  packet += sizeof(int16_t);

  return String(ozone_val) + ",";
}