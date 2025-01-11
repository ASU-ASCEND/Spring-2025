#include "PCF8523Sensor.h"

PCF8523Sensor::PCF8523Sensor() : PCF8523Sensor(0) {}

PCF8523Sensor::PCF8523Sensor(unsigned long minimum_period)
    : Sensor("PCF8523", "PCFTime, ", 1, minimum_period) {}

bool PCF8523Sensor::verify() {
  if (rtc.begin() == false) return false;
  rtc.start();
  return true;
}

String PCF8523Sensor::readData() {
  DateTime now = rtc.now();
  return String(now.year()) + "/" + String(now.month()) + "/" +
         String(now.day()) + " " + String(now.hour()) + ":" +
         String(now.minute()) + ":" + String(now.second()) + ",";
}

void PCF8523Sensor::calibrate() {
  if (!rtc.initialized() || rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}
