#include "DS3231Sensor.h"

/**
 * @brief Default constructor for the BME680Sensor class.
 *
 * Initializes the sensor object with a default minimum period of 0
 * milliseconds.
 */
DS3231Sensor::DS3231Sensor() : DS3231Sensor(0) {}

/**
 * @brief Parameterized constructor for the BME680Sensor class.
 *
 * This constructor initializes the BME680Sensor with a specified minimum period
 * between sensor readings. It passes sensor-specific information like the name,
 * CSV header, number of fields, and the minimum period between reads to the
 * base Sensor class constructor.
 *
 * @param minimum_period The minimum time (in milliseconds) between consecutive
 * sensor reads.
 */
DS3231Sensor::DS3231Sensor(unsigned long minimum_period)
    : Sensor("DS3231", "DS3231Time,DS3231TempC,", 2, minimum_period) {}

/**
 * @brief Verifies that the sensor is connected
 *
 * @return true - If the sensor is detected and successfully initialized.
 * @return false - If the sensor is not detected or fails to initialize.
 */
bool DS3231Sensor::verify() { return rtc.begin(); }


/**
 * @brief Reads timestamp data from RTC (plus temperature)
 *
 */
void DS3231Sensor::readDataPacket(uint8_t*& packet) {
  DateTime now = rtc.now();

  float temperature = rtc.getTemperature();

  uint16_t year = now.year();
  uint8_t month = now.month();
  uint8_t day = now.day();
  uint8_t hour = now.hour();
  uint8_t minute = now.minute();
  uint8_t second = now.second();

  std::copy((uint8_t*)(&year), (uint8_t*)(&year) + sizeof(year), packet);
  packet += sizeof(year);
  *(packet++) = month;
  *(packet++) = day;
  *(packet++) = hour;
  *(packet++) = minute;
  *(packet++) = second;

  std::copy((uint8_t*)(&temperature),
            (uint8_t*)(&temperature) + sizeof(temperature), packet);
  packet += sizeof(temperature);
}

/**
 * @brief Decodes a packet into a CSV string
 *
 */
String DS3231Sensor::decodeToCSV(uint8_t*& packet) {
  uint16_t year;
  memcpy(&year, packet, sizeof(uint16_t));
  packet += sizeof(year);

  uint8_t month = *(packet++);
  uint8_t day = *(packet++);
  uint8_t hour = *(packet++);
  uint8_t minute = *(packet++);
  uint8_t second = *(packet++);

  float temperature;
  memcpy(&temperature, packet, sizeof(float)); 
  packet += sizeof(temperature);

  return String(year) + "/" + String(month) + "/" + String(day) + " " +
         String(hour) + ":" + String(minute) + ":" + String(second) + "," +
         String(temperature) + ",";
}

/**
 * @brief Reads timestamp data from RTC (plus temperature)
 */
String DS3231Sensor::readData() {
  DateTime now = rtc.now();

  return String(now.year()) + "/" + String(now.month()) + "/" +
         String(now.day()) + " " + String(now.hour()) + ":" +
         String(now.minute()) + ":" + String(now.second()) + "," +
         String(rtc.getTemperature()) + ",";
}

/**
 * @brief Utility function to set the RTC's time, easier to use a separate
 * Arduino program to do this
 *
 * @param year Year
 * @param month Month
 * @param day Day
 * @param hour Hour in 24h time
 * @param minute Minute
 * @param second Second
 */
void DS3231Sensor::setTime(int year, int month, int day, int hour, int minute,
                           int second) {
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
}
