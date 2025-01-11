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
 * @return String CSV line - year/month/day hour:minute:second, tempC
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
