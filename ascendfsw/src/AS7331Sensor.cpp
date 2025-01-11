#include <AS7331Sensor.h>
#include <SparkFun_AS7331.h>

/**
 * @brief Construct a new AS7331Sensor (UVA/B/C Sensor) object with default
 * minimum_period of 0 ms
 *
 */
AS7331Sensor::AS7331Sensor(uint8_t i2c_addr) : AS7331Sensor(0, i2c_addr) {}

/**
 * @brief Construct a new AS7331Sensor (UVA/B/C Sensor) object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
AS7331Sensor::AS7331Sensor(unsigned long minium_period, uint8_t i2c_addr)
    : Sensor("AS7331" + String(i2c_addr & 0b11),
             "UVA" + String(i2c_addr & 0b11) + "(nm),UVB" +
                 String(i2c_addr & 0b11) + "(nm),UVC" +
                 String(i2c_addr & 0b11) + "(nm),",
             3, minium_period) {
  this->i2c_addr = i2c_addr;
}

/**
 * @brief Returns if the sensor can be reached
 *
 * @return true always
 * @return false never
 */
bool AS7331Sensor::verify() { return myUVSensor.begin(this->i2c_addr); }

/**
 * @brief Reads UV data
 *
 * @return String CSV line - UVA, UVB, UVC,
 */
String AS7331Sensor::readData() {
  myUVSensor.readAllUV();

  float uva = myUVSensor.getUVA();
  float uvb = myUVSensor.getUVB();
  float uvc = myUVSensor.getUVC();

  return String(uva) + "," + String(uvb) + "," + String(uvc) + ",";
}
