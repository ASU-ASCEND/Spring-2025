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

/**
 * @brief Reads sensor data and appends it to the packet byte array.
 *
 * Reads data from the PCF8523 sensor and appends it to the passed uint8_t array
 * pointer, incrementing it while doing so. The data includes Year, Month, Day,
 * Hour, Minutes, Seconds.
 *
 * @param packet - Pointer to the packet byte array.
 */
void PCF8523Sensor::readDataPacket(uint8_t*& packet) {
  DateTime now = rtc.now();

  uint16_t year = now.year();
  uint8_t data[5] = {now.month(), now.day(), now.hour(),
                     now.minute(),  // float or int?
                     now.second()};

  memcpy(packet, &year, sizeof(uint16_t));
  packet += sizeof(uint16_t);

  for (int i = 0; i < 5; i++) {
    memcpy(packet, &data[i], sizeof(uint8_t));  // Float or int?
    packet += sizeof(uint8_t);
  }
}

/**
 * @brief Decode the packet data and returns it in CSV format
 *
 * Decodes the packet data from the PCF8523 sesor and returns it in CSV format.
 * The data includes date and time.
 *
 * @param packet - Packet to decode
 * @return String - A String containing the sensor readings
 */
String PCF8523Sensor::decodeToCSV(uint8_t*& packet) {
  uint16_t year;
  memcpy(&year, packet, sizeof(uint16_t));
  packet += sizeof(uint16_t);

  uint8_t data[5];
  for (int i = 0; i < 5; i++) {
    memcpy(&data[i], packet, sizeof(uint8_t));
    packet += sizeof(uint8_t);
  }

  return String(year) + "/" + String(data[0]) + "/" + String(data[1]) + " " +
         String(data[2]) + ":" + String(data[3]) + ":" + String(data[4]) + ",";
}