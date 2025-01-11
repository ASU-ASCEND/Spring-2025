#include "MTK3339Sensor.h"

/**
 * @brief Default constructor for the MTK3339Sensor, sets minimum_period to 0 ms
 *
 */

MTK3339Sensor::MTK3339Sensor()
    : Sensor("MTK3339",
             "MTK_Date,MTK_Lat,MTKLong,MTKSpeed,MTKAngle,MTKAlt,MTKSats,", 7),
      GPS(&Wire) {}

/**
 * @brief Constructor for the MTK3339Sensor
 *
 * @param minimum_period Minimum period between sensor reads in ms
 */
MTK3339Sensor::MTK3339Sensor(unsigned long minimum_period)
    : Sensor("MTK3339",
             "MTK_Date,MTK_Lat,MTKLong,MTKSpeed,MTKAngle,MTKAlt,MTKSats,", 7,
             minimum_period),
      GPS(&Wire) {  // Initialize GPS with SPI using the defined macro
}
/**
 * @brief Verifies if the sensor is connected and working
 *
 * @return true if it is connected and working
 * @return false if it is not connected and working
 */
bool MTK3339Sensor::verify() {
  if (!GPS.begin(0x10)) {  // returns 0 on success
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
    delay(1000);
    return true;
  }
  return false;
}

/**
 * @brief Reads and returns data from the MTK3339 GPS corresponding to the CSV
 * header
 *
 * @return String CSV segment in form of Date, Latitude, Longitude, Speed,
 * Angle, Altitude, Satellites,
 */
String MTK3339Sensor::readData() {
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    Serial.println(
        GPS.lastNMEA());  // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) {  // this also sets the newNMEAreceived()
                                       // flag to false
      return "-,-,-,-,-,-,-,";         // we can fail to parse a sentence in
                                       // which case we should just wait for
                                       // another
    }
  }

  // Serial.println(GPS.milliseconds);
  //  Serial.print("Date: ");
  // Serial.print(GPS.day, DEC); Serial.print('/');
  // Serial.print(GPS.month, DEC); Serial.print("/20");
  // Serial.println(GPS.year, DEC);
  // Serial.print("Fix: "); //Serial.print((int)GPS.fix);
  // Serial.print(" quality: "); Serial.println((int)GPS.fixquality);

  if (GPS.fix) {
    // Serial.print("Location: ");
    // Serial.print(GPS.latitude, 4); Serial.print(GPS.lat);
    // Serial.print(", ");
    // Serial.print(GPS.longitude, 4); Serial.println(GPS.lon);
    // Serial.print("Speed (knots): "); Serial.println(GPS.speed);
    // Serial.print("Angle: "); Serial.println(GPS.angle);
    // Serial.print("Altitude: "); Serial.println(GPS.altitude);
    // Serial.print("Satellites: "); Serial.println((int)GPS.satellites);
    return String(GPS.day) + "/ " + String(GPS.month) + "/ " +
           String(GPS.year) + "," + String(GPS.latitude) + "," +
           String(GPS.longitude) + "," + String(GPS.speed) + "," +
           String(GPS.angle) + "," + String(GPS.altitude) + "," +
           String(GPS.satellites) + ",";
  }
  return "-,-,-,-,-,-,-,";
}
