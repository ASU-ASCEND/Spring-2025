#include "BME280Sensor.h"

/**
 * @brief Construct a new Temp Sensor object with default minimum_period of 0 ms
 *
 */
BME280Sensor::BME280Sensor() : BME280Sensor(0) {}

/**
 * @brief Construct a new Temp Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
BME280Sensor::BME280Sensor(unsigned long minium_period)
    : Sensor("BME280",
             "BME280RelHum %,BME280Pres Pa,BME280Alt m,BME280TempC,DewPointC,",
             5, minium_period) {}

/**
 * @brief Returns if sensor is connected and working
 *
 * @return true if sensor is connected and working
 * @return false if the sensor isn't connected and working
 */
bool BME280Sensor::verify() {
  Wire.begin();

  return bme.beginI2C();
}

/**
 * @brief Reads Relative Humidity (%), Pressure (Pa), Altitude (m), Temp (C)
 * from the BME280
 *
 * @return String Relative Humidity (%), Pressure (Pa), Altitude (m), Temp (C)
 * in csv format
 */
String BME280Sensor::readData() {
  return String(bme.readFloatHumidity()) + "," +
         String(bme.readFloatPressure()) + "," +
         String(bme.readFloatAltitudeMeters()) + "," + String(bme.readTempC()) +
         "," + String(bme.dewPointC()) + ",";
}
