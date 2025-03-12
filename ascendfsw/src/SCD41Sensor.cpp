#include "SCD41Sensor.h"

/**
 * @brief Default constructor for the SCD41Sensor class.
 *
 * Initializes the sensor object with a default minimum period of 0
 * milliseconds.
 */
SCD41Sensor::SCD41Sensor() : SCD41Sensor(0) {}

/**
 * @brief Parameterized constructor for the SCD41Sensor class.
 *
 * This constructor initializes the SCD41Sensor with a specified minimum period
 * between sensor readings. It passes sensor-specific information like the name,
 * CSV header, number of fields, and the minimum period between reads to the
 * base Sensor class constructor
 * 
 * @param minimum_period The minimum time (in milliseconds) between consecutive
 * sensor reads.
 */
SCD41Sensor::SCD41Sensor(unsigned long minimum_period) 
    : Sensor("SCD41", "SCD41CO2(ppm),SCD41Temp(C),", 2, minimum_period) {
    scd = SensirionI2cScd4x();
}

/**
 * @brief Verifies the connection and readiness of the SCD41 sensor.
 *
 * This function initializes the sensor and checks if the sensor is properly
 * connected and ready for reading data.
 *
 * @return true  - If the sensor is detected and successfully initialized.
 * @return false - If the sensor is not detected or fails to initialize.
 */
bool SCD41Sensor::verify() {
    // TODO: Implement this function
    return false;
}

/**
 * @brief Reads sensor data and returns it in CSV format.
 *
 * Reads data from the SCD41 sensor and returns it in CSV format. The data
 * includes CO2 concentration in parts per million (ppm) and temperature in
 * degrees Celsius.
 *
 * @return String - A string containing the sensor readings
 */
String SCD41Sensor::readData() {
    // TODO: Implement this function
    return "-,-,";
}

/**
 * @brief Reads sensor data and appends it to the packet byte array.
 *
 * Reads data from the SCD41 sensor and appends it to the passed uint8_t array
 * pointer, incrementing it while doing so. The data includes CO2 concentration
 * in parts per million (ppm) and temperature in degrees Celsius.
 *
 * @param packet - Pointer to the packet byte array.
 */
void SCD41Sensor::readDataPacket(uint8_t*& packet) {
    // TODO: Implement this function
}

/**
 * @brief Decodes the packet data and returns it in CSV format.
 *
 * Decodes the packet data from the SCD41 sensor and returns it in CSV format.
 * The data includes CO2 concentration in parts per million (ppm) and temperature
 * in degrees Celsius.
 *
 * @param packet - Packet to decode.
 * @return String - A string containing the sensor readings
 */
String SCD41Sensor::decodeToCSV(uint8_t*& packet) {
    // TODO: Implement this function
    return "-,-,";
}