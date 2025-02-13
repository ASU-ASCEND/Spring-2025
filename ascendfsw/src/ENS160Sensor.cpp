#include "ENS160Sensor.h"

/**
 * @brief Construct a new ENS160 Sensor object with default minimum_period of 0
 * ms
 *
 */
ENS160Sensor::ENS160Sensor() : ENS160Sensor(0) {}

/**
 * @brief Construct a new ENS160 Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
ENS160Sensor::ENS160Sensor(unsigned long minium_period)
    : Sensor("ENS160", "ENSAQI,ENSTVOC ppb,ENSECO2 ppm,", 3, minium_period) {}

/**
 * @brief Returns if the ENS160 sensor can be reached
 *
 * @return true if the ENS is connected
 * @return false if the ENS isn't connected
 */
bool ENS160Sensor::verify() {
  Wire.begin();

  if (!ens.begin()) {
    return false;
  }

  ens.setOperatingMode(SFE_ENS160_STANDARD);

  return true;
}

/**
 * @brief Reads Air Quality Index, TVOC (ppb), and ECOS (ppm)
 *
 * @return String AQI, TVOC, ECOS data in CSV format
 */
String ENS160Sensor::readData() {
  if (ens.checkDataStatus()) {
    return String(ens.getAQI()) + "," + ens.getTVOC() + "," + ens.getECO2() +
           ",";
  } else {
    return this->readEmpty();
  }
}
void ENS160Sensor::readDataPacket(uint8_t*& packet) {
  if (!ens.checkDataStatus()) {
    return;
  }
  uint8_t aqi = ens.getAQI();
  uint16_t tvoc = ens.getTVOC();
  uint16_t eco2 = ens.getECO2();

  memcpy(packet, &aqi, sizeof(aqi));
  packet += sizeof(aqi);

  memcpy(packet, &tvoc, sizeof(tvoc));
  packet += sizeof(tvoc);

  memcpy(packet, &eco2, sizeof(eco2));
  packet += sizeof(eco2);
}

String ENS160Sensor::decodeToCSV(uint8_t* packet) {
  uint8_t aqi = *packet;
  packet += sizeof(uint8_t);

  uint16_t tvoc;
  memcpy(&tvoc, packet, sizeof(uint16_t));
  packet += sizeof(uint16_t);

  uint16_t eco2;
  memcpy(&eco2, packet, sizeof(uint16_t));
  packet += sizeof(uint16_t);

  return String(aqi) + "," + String(tvoc) + "," + String(eco2) + ",";
}