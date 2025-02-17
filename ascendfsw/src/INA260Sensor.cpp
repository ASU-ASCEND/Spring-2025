#include "INA260Sensor.h"

/**
 * @brief Default constructor for the INA260Sensor class.
 *
 * Initializes the sensor object with a default minimum period of 0 milliseconds
 * by calling the parameterized constructor with 0.
 */
INA260Sensor::INA260Sensor() : INA260Sensor(0) {}

/**
 * @brief Parameterized constructor for the INA260Sensor class.
 *
 * Initializes the sensor object with a specified minimum period and sets up the
 * sensor name, CSV header, and number of data fields (3: current, voltage, and
 * power).
 *
 * @param minimum_period The minimum time period between sensor readings, in
 * milliseconds.
 */
INA260Sensor::INA260Sensor(unsigned long minimum_period)
    : Sensor("INA260", "INACurr(mA),INAVolt(mV),INAPow(mW),", 3,
             minimum_period) {}

/**
 * @brief Verifies the connection to the INA260 sensor.
 *
 * This function initializes the INA260 sensor object and checks whether the
 * sensor is successfully detected on the I2C bus.
 *
 * @return true If the sensor is successfully initialized.
 * @return false If the sensor fails to initialize or is not detected.
 */
bool INA260Sensor::verify() {
  ina260 = Adafruit_INA260();
  return ina260.begin();
}

/**
 * @brief Reads data from the INA260 sensor and returns it in CSV format.
 *
 * This function reads the current (in mA), bus voltage (in mV), and power (in
 * mW) values from the INA260 sensor and formats them into a CSV string.
 *
 * @return String A string containing the sensor data in the format:
 * "Current (mA), Voltage (mV), Power (mW), ".
 */
String INA260Sensor::readData() {
  
  float current = ina260.readCurrent();
  float busVoltage = ina260.readBusVoltage();
  float power = ina260.readPower();

  return String(current) + "," + String(busVoltage) + "," + String(power) + ",";
}

/**
 * @brief Reads sensor data and appends to packet byte array, data includes current(mA),
 * voltage(mV), and power (mW).
 *
 * @param packet Pointer to the packet byte array.
 */

void INA260Sensor::readDataPacket(uint8_t*& packet) {
 
float current = ina260.readCurrent();
float busVoltage = ina260.readBusVoltage();
float power = ina260.readPower();

float data[3] = {current, busVoltage, power};

for(int i = 0; i < 3; i++){
  memcpy(packet, &data[i], sizeof(float));
  packet +=sizeof(float);
}

}

/**
 * @brief Decodes sensor data from the packet into a CSV string.
 * 
 * Data includes current(mA), voltage(mV), and power (mW).
 * 
 * @param packet Pointer to packet byte array that will be decoded.
 * @return String Decoded sensor data in CSV format.
 */
String INA260Sensor::decodeToCSV(uint8_t*& packet) {
  float data[3];

  for(int i = 0; i < 3; i++){
    memcpy(packet, &data[i], sizeof(float));
    packet +=sizeof(float);
  }

  return String(data[0]) + "," + String(data[1]) + "," + String(data[2]) + "," ;
  
}