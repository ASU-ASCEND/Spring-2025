#include "SHT31Sensor.h"

/**
 * @brief Construct a new SHT31Sensor::SHT31Sensor object
 *
 */
SHT31Sensor::SHT31Sensor() : SHT31Sensor(0) {}
/**
 * @brief Construct a new SHT31Sensor::SHT31Sensor object
 *
 * @param minimum_period
 */
SHT31Sensor::SHT31Sensor(unsigned long minimum_period)
    : Sensor("SHT31", "SHTHum(%), SHTTemp(C),", 2, minimum_period) {}

/**
 * @brief Verifies connection of SHT Sensor
 *
 *
 * @return true if connected and working
 * @return false if not connected and working
 */
bool SHT31Sensor::verify() {
  sht31 = Adafruit_SHT31();
  return sht31.begin(0x44);
  // return false;
}

/**
 * @brief Reads sensor data and returns in csv format
 *
 * @return String A CSV section in the format humidity, temperature,
 */
String SHT31Sensor::readData() {
  // return "-. -. ";
  float t = sht31.readTemperature();
  float h = sht31.readHumidity();
  return String(h) + "," + String(t) + ",";
}