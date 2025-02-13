#include "BME280Sensor.h"

#include <algorithm>  //For std::copy
#include <cstdint>    // For uint8_t

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

// Reads sensor data and appends it to the packet byte array using std::copy
void BME280Sensor::readDataPacket(uint8_t*& packet) {
  // Read sensor values into float variables.
  float relHum = bme.readFloatHumidity();
  float pressure = bme.readFloatPressure();
  float altitude = bme.readFloatAltitudeMeters();
  float temp = bme.readTempC();
  float dewPoint = bme.dewPointC();

  // Store the sensor values in an array.
  float data[5] = {relHum, pressure, altitude, temp, dewPoint};

  // Copy the raw bytes of the sensor data into the packet buffer
  std::copy(reinterpret_cast<uint8_t*>(data),
            reinterpret_cast<uint8_t*>(data) + 5 * sizeof(float), packet);

  // Increment the packet pointer by the number of bytes copied
  packet += 5 * sizeof(float);
}

// Decodes sensor data from the packet and returns it as a CSV string
String BME280Sensor::decodeToCSV(uint8_t*& packet) {
  // Decode each float value from the packet.
  float relHum = *(reinterpret_cast<float*>(packet));
  packet += sizeof(float);

  float pressure = *(reinterpret_cast<float*>(packet));
  packet += sizeof(float);

  float altitude = *(reinterpret_cast<float*>(packet));
  packet += sizeof(float);

  float temp = *(reinterpret_cast<float*>(packet));
  packet += sizeof(float);

  float dewPoint = *(reinterpret_cast<float*>(packet));
  packet += sizeof(float);

  // Construct and return the CSV string.
  return String(relHum) + "," + String(pressure) + "," + String(altitude) +
         "," + String(temp) + "," + String(dewPoint) + ",";
}
