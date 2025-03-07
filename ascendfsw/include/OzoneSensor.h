#ifndef OZONE_SENSOR_H
#define OZONE_SENSOR_H

#include "Sensor.h"
#include "DFRobot_OzoneSensor.h"

class OzoneSensor : public Sensor {
private:
  DFRobot_OzoneSensor ozone; 
public:
  OzoneSensor();
  OzoneSensor(unsigned long minium_period);

  bool verify() override;
  String readData() override;
  void readDataPacket(uint8_t*& packet) override;
  String decodeToCSV(uint8_t*& packet) override;
};


#endif // OZONE_SENSOR_H