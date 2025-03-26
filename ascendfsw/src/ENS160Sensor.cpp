#include "ENS160Sensor.h"

#include "SHTC3Sensor.h"
#include "TMP117Sensor.h"

extern TMP117Sensor tmp_sensor;
extern SHTC3Sensor shtc_sensor;

/**
 * @brief Construct a new ENS160 Sensor object with default minimum_period of 0
 * ms
 *
 */
ENS160Sensor::ENS160Sensor(TwoWire* i2c_bus) : ENS160Sensor(0, i2c_bus) {}

/**
 * @brief Construct a new ENS160 Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
ENS160Sensor::ENS160Sensor(unsigned long minium_period, TwoWire* i2c_bus)
    : Sensor("ENS160", "ENSAQI,ENSTVOC ppb,ENSECO2 ppm,", 3, minium_period) {
  this->i2c_bus = i2c_bus;
  if (this->i2c_bus == &Wire1) this->device_name += "1";
}

/**
 * @brief Returns if the ENS160 sensor can be reached
 *
 * @return true if the ENS is connected
 * @return false if the ENS isn't connected
 */
bool ENS160Sensor::verify() {
  this->i2c_bus->begin();

  if (!ens.begin(*(this->i2c_bus))) {
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

  // set compensation values after read if we have them so that conversion
  // doesn't slow the read not sure if it actually does but this will let it
  // take until the next read which should be more than enough time, make sure
  // that tmp117 and shtc3
  if (tmp_sensor.getVerified()) {
    ens.setTempCompensationCelsius(tmp_sensor.getTempC());
  }
  if (shtc_sensor.getVerified()) {
    ens.setRHCompensationFloat(shtc_sensor.getRelHum());
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

  // set compensation values after read if we have them so that conversion
  // doesn't slow the read not sure if it actually does but this will let it
  // take until the next read which should be more than enough time, make sure
  // that tmp117 and shtc3
  if (tmp_sensor.getVerified()) {
    ens.setTempCompensationCelsius(tmp_sensor.getTempC());
  }
  if (shtc_sensor.getVerified()) {
    ens.setRHCompensationFloat(shtc_sensor.getRelHum());
  }
}

String ENS160Sensor::decodeToCSV(uint8_t*& packet) {
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