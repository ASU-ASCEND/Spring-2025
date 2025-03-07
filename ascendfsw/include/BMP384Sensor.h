#ifndef BMP384_SENSOR_H
#define BMP384_SENSOR_H

#include "Sensor.h"
#include "Wire.h"
#include "SparkFunBMP384.h"

/**
 * @brief Implementation of a Sensor for BMP384 Pressure and Temperature sensor
 *
 */
class BMP384Sensor : public Sensor {
  private:
   BMP384 bmp; 
  public:
   BMP384Sensor();
   BMP384Sensor(unsigned long minium_period);

   bool verify() override;
   String readData() override;
   void readDataPacket(uint8_t*& packet) override;
   String decodeToCSV(uint8_t*& packet) override;
};




#endif // BMP384_SENSOR_H