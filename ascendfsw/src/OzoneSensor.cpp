#include "OzoneSensor.h"

OzoneSensor::OzoneSensor() : OzoneSensor(0) {}

OzoneSensor::OzoneSensor(unsigned long minium_period)
  : Sensor("OzoneSensor", "O3 PPB, ", minium_period) {

}

bool OzoneSensor::verify(){
  if(this->ozone.begin() == false){
    return false; 
  }

  ozone.setModes(MEASURE_MODE_PASSIVE); 

  return true; 
}

String OzoneSensor::readData(){
  return String(this->ozone.readOzoneData()) + ","; 
}

void OzoneSensor::readDataPacket(uint8_t*& packet){
  int16_t ozone_val = this->ozone.readOzoneData(); 
  memcpy(packet, &ozone_val, sizeof(int16_t)); 
  packet += sizeof(int16_t); 
}

String OzoneSensor::decodeToCSV(uint8_t*& packet){
  int16_t ozone_val = 0;  
  memcpy(&ozone_val, packet, sizeof(int16_t)); 
  packet += sizeof(int16_t); 

  return String(ozone_val) + ","; 
}