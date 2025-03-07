#include "BMP384Sensor.h"

/**
 * @brief Construct a new BMP384 Sensor object with default minimum_period of 0 ms
 *
 */
BMP384Sensor::BMP384Sensor() : BMP384Sensor(0) {}

/**
 * @brief Construct a new BMP384 Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
BMP384Sensor::BMP384Sensor(unsigned long minium_period)
  : Sensor("BMP384", "BMP TempC,BMP PresPa,", minium_period) {}

/**
 * @brief Tests if the sensor is can be reached on I2C, sets up oversampling and filter configuration  
 * 
 * @return true 
 * @return false 
 */
bool BMP384Sensor::verify() {
  Wire.begin(); 

  if(bmp.beginI2C() != BMP3_OK){
    return false; 
  }

  // oversampling
  bmp3_odr_filter_settings osrMultipliers = {
    .press_os = BMP3_OVERSAMPLING_32X,
    .temp_os = BMP3_OVERSAMPLING_32X,
    0,0
  };
  bmp.setOSRMultipliers(osrMultipliers); 

  // filtering 
  bmp.setFilterCoefficient(BMP3_IIR_FILTER_COEFF_127);

  // either of these failing is likely not a critical error so can still return true if failed 
  // if this ends up being a problem there is a way to test it 
  // https://github.com/sparkfun/SparkFun_BMP384_Arduino_Library/blob/main/examples/Example5_Oversampling/Example5_Oversampling.ino 


  return true; 
}

/**
 * @brief Reads data to a csv stub 
 * 
 * @return String CSV stub Temperature, Pressure,
 */
String BMP384Sensor::readData() {
  bmp3_data data; 
  int8_t err = bmp.getSensorData(&data); 

  if(err == BMP3_OK){
    return String(data.temperature, 5) + "," + String(data.pressure, 5) + ","; 
  } else {
    return this->readEmpty(); 
  }
}

/**
 * @brief Copies data to the packet 
 * 
 * @param packet Pointer to copy at 
 */
void BMP384Sensor::readDataPacket(uint8_t*& packet) {
  bmp3_data data; 
  int8_t err = bmp.getSensorData(&data); 

  if(err == BMP3_OK){
    memcpy(packet, &(data.temperature), sizeof(data.temperature)); 
    packet += sizeof(data.temperature);

    memcpy(packet, &(data.pressure), sizeof(data.pressure)); 
    packet += sizeof(data.pressure); 

  } else {
    double zero = 0; 
    memcpy(packet, &(zero), sizeof(zero)); 
    packet += sizeof(zero); 

    memcpy(packet, &(zero), sizeof(zero)); 
    packet += sizeof(zero); 
  }
}

/**
 * @brief Decodes data from the packet
 * 
 * @param packet Pointer to decode at 
 * @return String CSV stub Temperature, Pressure, 
 */
String BMP384Sensor::decodeToCSV(uint8_t*& packet) {
  double temperature = 0; 
  double pressure = 0; 

  memcpy(&temperature, packet, sizeof(temperature)); 
  packet += sizeof(temperature);

  memcpy(&pressure, packet, sizeof(pressure)); 
  packet += sizeof(pressure); 

  return String(temperature, 5) + "," + String(pressure, 5) + ","; 
}