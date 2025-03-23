#include "MTK3339Sensor.h"

/**
 * @brief Default constructor for the MTK3339Sensor, sets minimum_period to 0 ms
 *
 */

MTK3339Sensor::MTK3339Sensor() : MTK3339Sensor(0) {}

/**
 * @brief Constructor for the MTK3339Sensor
 *
 * @param minimum_period Minimum period between sensor reads in ms
 */
MTK3339Sensor::MTK3339Sensor(unsigned long minimum_period)
    : Sensor("MTK3339",
             "MTKTime,MTKLat,MTKLong,MTKGndSpeed,MTKHeading,MTKAlt,MTKSats,",
             minimum_period) {}
/**
 * @brief Verifies if the sensor is connected and working
 *
 * @return true if it is connected and working
 * @return false if it is not connected and working
 */
bool MTK3339Sensor::verify() {
#if GPS_I2C
  Wire.begin();
  if (GPS.begin() == false) {
    return false;
  }

  GPS.setI2COutput(COM_TYPE_UBX);
  GPS.setNavigationFrequency(1);
  GPS.setDynamicModel(DYN_MODEL_AIRBORNE4g);

  return true;
#else
  Serial2.setRX(SERIAL2_RX_PIN);
  Serial2.setTX(SERIAL2_TX_PIN);

  Serial2.begin(38400);
  if (GPS.begin(Serial2) == false) {
    return false;
  }
  GPS.setUART1Output(COM_TYPE_UBX);

  return true;
#endif
}

/**
 * @brief Reads and returns data from the MTK3339 GPS corresponding to the CSV
 * header
 *
 * @return String CSV segment in form of Date, Latitude, Longitude, Speed,
 * Angle, Altitude, Satellites,
 */
String MTK3339Sensor::readData() {
  if (GPS.getPVT()) {
    return String(GPS.getYear()) + "/" + String(GPS.getMonth()) + "/" +
           String(GPS.getDay()) + " " + String(GPS.getHour()) + ":" +
           String(GPS.getMinute()) + ":" + String(GPS.getSecond()) + "," +
           String(GPS.getLatitude() / 10000000.0) + "," +
           String(GPS.getLongitude() / 10000000.0) + "," +
           String(GPS.getGroundSpeed()) + "," + String(GPS.getHeading()) + "," +
           String(GPS.getAltitude()) + "," + String(GPS.getSIV()) + ",";
  }
  return this->readEmpty();
}

/**
 * @brief Appends the MTK3339 sensor data to the packet buffer as raw bytes.
 *
 * The following data are appended in order:
 *   - Date: day (uint8_t), month (uint8_t), year (uint16_t)
 *   - Latitude (float)
 *   - Longitude (float)
 *   - Speed (float)
 *   - Angle (float)
 *   - Altitude (float)
 *   - Satellites (uint8_t)
 *
 * If no valid fix is available, default value (0) is appended for all fields.
 *
 * @param packet Pointer to the packet byte array. This pointer is incremented
 * as each value is copied.
 */
void MTK3339Sensor::readDataPacket(uint8_t*& packet) {
  if (GPS.getPVT()) {
    // Pack date values
    uint16_t year = GPS.getYear();
    uint8_t month = GPS.getMonth();
    uint8_t day = GPS.getDay();
    memcpy(packet, &year, sizeof(year));
    packet += sizeof(year);
    memcpy(packet, &month, sizeof(month));
    packet += sizeof(month);
    memcpy(packet, &day, sizeof(day));
    packet += sizeof(day);

    uint8_t hour = GPS.getHour();
    uint8_t minute = GPS.getMinute();
    uint8_t second = GPS.getSecond();
    memcpy(packet, &hour, sizeof(hour));
    packet += sizeof(hour);
    memcpy(packet, &minute, sizeof(minute));
    packet += sizeof(minute);
    memcpy(packet, &second, sizeof(second));
    packet += sizeof(second);

    // Pack latitude, longitude, speed, heading, and altitude as floats
    float lat = GPS.getLatitude() / 10000000.0;
    float lon = GPS.getLongitude() / 10000000.0;
    float speed = GPS.getGroundSpeed();
    float heading = GPS.getHeading();
    float alt = GPS.getAltitude();
    memcpy(packet, &lat, sizeof(lat));
    packet += sizeof(lat);
    memcpy(packet, &lon, sizeof(lon));
    packet += sizeof(lon);
    memcpy(packet, &speed, sizeof(speed));
    packet += sizeof(speed);
    memcpy(packet, &heading, sizeof(heading));
    packet += sizeof(heading);
    memcpy(packet, &alt, sizeof(alt));
    packet += sizeof(alt);
    // Pack number of satellites as uint8_t
    uint8_t sats = GPS.getSIV();
    memcpy(packet, &sats, sizeof(sats));
    packet += sizeof(sats);

  } else {
    // If there's no fix, append default zero values
    uint16_t year = 0;
    uint8_t month = 0;
    uint8_t day = 0;
    memcpy(packet, &year, sizeof(year));
    packet += sizeof(year);
    memcpy(packet, &month, sizeof(month));
    packet += sizeof(month);
    memcpy(packet, &day, sizeof(day));
    packet += sizeof(day);

    uint8_t hour = 0;
    uint8_t minute = 0;
    uint8_t second = 0;
    memcpy(packet, &hour, sizeof(hour));
    packet += sizeof(hour);
    memcpy(packet, &minute, sizeof(minute));
    packet += sizeof(minute);
    memcpy(packet, &second, sizeof(second));
    packet += sizeof(second);

    float zero = 0.0;
    memcpy(packet, &zero, sizeof(zero));  // Latitude
    packet += sizeof(zero);
    memcpy(packet, &zero, sizeof(zero));  // Longitude
    packet += sizeof(zero);
    memcpy(packet, &zero, sizeof(zero));  // Speed
    packet += sizeof(zero);
    memcpy(packet, &zero, sizeof(zero));  // Angle
    packet += sizeof(zero);
    memcpy(packet, &zero, sizeof(zero));  // Altitude
    packet += sizeof(zero);
    uint8_t sats = 0;
    memcpy(packet, &sats, sizeof(sats));
    packet += sizeof(sats);
  }
}

/**
 * @brief Decodes the MTK3339 sensor data from the packet buffer into a CSV
 * string.
 *
 * The data are read in the same order they were written and  reconstructed as a
 * string "day/month/year" and the remaining fields are appended as CSV values.
 *
 * @param packet Pointer to the packet byte array and this packet pointer is
 * incremented.
 * @return String The decoded sensor data in CSV format.
 */
String MTK3339Sensor::decodeToCSV(uint8_t*& packet) {
  // Decode date components
  uint16_t year;
  memcpy(&year, packet, sizeof(uint16_t));
  packet += sizeof(uint16_t);
  uint8_t month = *packet;
  packet += sizeof(uint8_t);
  uint8_t day = *packet;
  packet += sizeof(uint8_t);

  uint8_t hour = *packet;
  packet += sizeof(uint8_t);
  uint8_t minute = *packet;
  packet += sizeof(uint8_t);
  uint8_t second = *packet;
  packet += sizeof(uint8_t);

  // Decode values: latitude, longitude, speed, heading, altitude. And then
  // decode number of satellites.
  float lat;
  memcpy(&lat, packet, sizeof(float));
  packet += sizeof(float);

  float lon;
  memcpy(&lon, packet, sizeof(float));
  packet += sizeof(float);

  float speed;
  memcpy(&speed, packet, sizeof(float));
  packet += sizeof(float);

  float heading;
  memcpy(&heading, packet, sizeof(float));
  packet += sizeof(float);

  float alt;
  memcpy(&alt, packet, sizeof(float));
  packet += sizeof(float);

  uint8_t sats = *packet;
  packet += sizeof(uint8_t);

  // Construct the CSV string
  String timeStamp = String(year) + "/" + String(month) + "/" + String(day) +
                     " " + String(hour) + ":" + String(minute) + ":" +
                     String(second);
  String csv = timeStamp + "," + String(lat, 10) + "," + String(lon, 10) + "," +
               String(speed) + "," + String(heading) + "," + String(alt) + "," +
               String(sats) + ",";
  return csv;
}