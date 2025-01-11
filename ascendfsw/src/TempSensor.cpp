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
