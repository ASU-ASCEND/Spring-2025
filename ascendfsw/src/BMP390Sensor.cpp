#include "BMP390Sensor.h"

#define SEALEVELPRESSURE_HPA (1013.25)

/**
 * @brief Construct a new BMP384 Sensor object with default minimum_period of 0 ms
 *
 */
BMP390Sensor::BMP390Sensor() : BMP390Sensor(0) {}

/**
 * @brief Construct a new BMP384 Sensor object
 *
 * @param minium_period Minimum time to wait between readings in ms
 */
BMP390Sensor::BMP390Sensor(unsigned long minium_period)
  : Sensor("BMP390", "BMP TempC,BMP PresPa,BMP Altm", minium_period) {}

/**
 * @brief Tests if the sensor is can be reached on I2C, sets up oversampling and filter configuration  
 * 
 * @return true 
 * @return false 
 */
bool BMP390Sensor::verify() {

  if(bmp.begin_I2C() == false){
    return false; 
  }

  bmp.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
  bmp.setPressureOversampling(BMP3_OVERSAMPLING_4X);
  bmp.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
  bmp.setOutputDataRate(BMP3_ODR_50_HZ);
  
  return true; 
}

/**
 * @brief Reads data to a csv stub 
 * 
 * @return String CSV stub Temperature, Pressure,
 */
String BMP390Sensor::readData() {
  if(bmp.performReading()){
    return String(bmp.temperature, 5) + "," + String(bmp.pressure, 5) + "," + String(bmp.readAltitude(SEALEVELPRESSURE_HPA)) + ","; 
  } else {
    return this->readEmpty(); 
  }
}

/**
 * @brief Copies data to the packet 
 * 
 * @param packet Pointer to copy at 
 */
void BMP390Sensor::readDataPacket(uint8_t*& packet) {

  if(bmp.performReading()){
    memcpy(packet, &(bmp.temperature), sizeof(bmp.temperature)); 
    packet += sizeof(bmp.temperature);

    memcpy(packet, &(bmp.pressure), sizeof(bmp.pressure)); 
    packet += sizeof(bmp.pressure);
    
    float alt = bmp.readAltitude(SEALEVELPRESSURE_HPA); 
    memcpy(packet, &alt, sizeof(alt)); 
    packet += sizeof(alt); 

  } else {
    double zero = 0; 
    memcpy(packet, &(zero), sizeof(zero)); 
    packet += sizeof(zero); 

    memcpy(packet, &(zero), sizeof(zero)); 
    packet += sizeof(zero); 

    float fzero = 0; 
    memcpy(packet, &fzero, sizeof(fzero)); 
    packet += sizeof(fzero); 
  }
}

/**
 * @brief Decodes data from the packet
 * 
 * @param packet Pointer to decode at 
 * @return String CSV stub Temperature, Pressure, 
 */
String BMP390Sensor::decodeToCSV(uint8_t*& packet) {
  double temperature = 0; 
  double pressure = 0; 
  float alt = 0; 

  memcpy(&temperature, packet, sizeof(temperature)); 
  packet += sizeof(temperature);

  memcpy(&pressure, packet, sizeof(pressure)); 
  packet += sizeof(pressure); 

  memcpy(&alt, packet, sizeof(alt)); 
  packet += sizeof(alt); 

  return String(temperature, 5) + "," + String(pressure, 5) + "," + String(alt, 5); 
}