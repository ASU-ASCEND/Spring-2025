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
    : Sensor("SCD41", "SCD41CO2(ppm),SCD41Temp(C),SCD41Hum(%RH),", 2, minimum_period) {
    this->scd = SensirionI2cScd4x();
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
    Wire.begin();
    this->scd.begin(Wire, SCD40_I2C_ADDR_62);

    // Verify sensor has been initialized
    uint64_t serial_number;
    int16_t error = this->scd.getSerialNumber(serial_number);
    if(error != 0) {
        return false; // Sensor not responding
    }
    return true; // Sensor initialized successfully
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
    uint16_t co2_concentration;
    float temperature, relative_humidity;
    
    // Read data & check for errors
    int16_t error = this->scd.readMeasurement(co2_concentration, temperature, relative_humidity);
    if(error != 0) {
        return "-,-,-,";
    }

    // Return data in CSV format
    return String(co2_concentration)  + "," + 
           String(temperature)      + "," + 
           String(relative_humidity) + ",";
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
    uint16_t co2_concentration;
    float temperature, relative_humidity;

    // Read data & check for errors
    int16_t error = this->scd.readMeasurement(co2_concentration, temperature, relative_humidity);
    if(error != 0) {
        return;
    }

    // Write data to packet
    memcpy(packet, &co2_concentration, sizeof(uint16_t));
    packet += sizeof(uint16_t);

    memcpy(packet, &temperature, sizeof(float));
    packet += sizeof(float);

    memcpy(packet, &relative_humidity, sizeof(float));
    packet += sizeof(float);
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
    uint16_t co2_concentration;
    float temperature, relative_humidity;

    // Extract data
    memcpy(&co2_concentration, packet, sizeof(uint16_t));
    packet += sizeof(uint16_t);

    memcpy(&temperature, packet, sizeof(float));
    packet += sizeof(float);

    memcpy(&relative_humidity, packet, sizeof(float));
    packet += sizeof(float);

    // Return data in CSV format
    return String(co2_concentration)  + "," + 
           String(temperature)      + "," + 
           String(relative_humidity) + ",";
}