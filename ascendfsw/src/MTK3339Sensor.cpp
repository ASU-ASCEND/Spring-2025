#include "MTK3339Sensor.h"

/**
 * @brief Default constructor for the MTK3339Sensor, sets minimum_period to 0 ms
 *
 */

MTK3339Sensor::MTK3339Sensor()
    : Sensor("MTK3339",
             "MTK_Date,MTK_Lat,MTKLong,MTKSpeed,MTKAngle,MTKAlt,MTKSats,MTKAnt,", 8),
      GPS(&Serial2) {}

/**
 * @brief Constructor for the MTK3339Sensor
 *
 * @param minimum_period Minimum period between sensor reads in ms
 */
MTK3339Sensor::MTK3339Sensor(unsigned long minimum_period)
    : Sensor("MTK3339",
             "MTK_Date,MTK_Lat,MTKLong,MTKSpeed,MTKAngle,MTKAlt,MTKSats,MTKAnt,", 8,
             minimum_period),
      GPS(&Serial2) {  // Initialize GPS with SPI using the defined macro
}
/**
 * @brief Verifies if the sensor is connected and working
 *
 * @return true if it is connected and working
 * @return false if it is not connected and working
 */
bool MTK3339Sensor::verify() {
  // Serial2.end();
  Serial2.setRX(SERIAL2_RX_PIN);
  Serial2.setTX(SERIAL2_TX_PIN);
  Serial.println("Done"); 

  if (GPS.begin(38400)) {  
    GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
    GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);  // 1 Hz update rate
    GPS.sendCommand(PGCMD_ANTENNA);

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

    if (!GPS.parse(GPS.lastNMEA())) {  // this also sets the newNMEAreceived()
                                       // flag to false
      return "-,-,-,-,-,-,-,-,";         // we can fail to parse a sentence in
                                       // which case we should just wait for
                                       // another
    }
  }

  if (GPS.fix) {
    return String(GPS.day) + "/ " + String(GPS.month) + "/ " +
           String(GPS.year) + "," + String(GPS.latitude) + "," +
           String(GPS.longitude) + "," + String(GPS.speed) + "," +
           String(GPS.angle) + "," + String(GPS.altitude) + "," +
           String(GPS.satellites) + "," + String(GPS.antenna) + ",";
  }
  return "-,-,-,-,-,-,-,-,";
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
  bool good_parse = true; 
  if (GPS.newNMEAreceived()) {

    if (!GPS.parse(GPS.lastNMEA())) { 
      good_parse = false; 
    }
  }
  
  if (good_parse && GPS.fix) {
    // Pack date values
    uint8_t day = GPS.day;
    uint8_t month = GPS.month;
    uint16_t year = GPS.year;
    memcpy(packet, &day, sizeof(day));
    packet += sizeof(day);
    memcpy(packet, &month, sizeof(month));
    packet += sizeof(month);
    memcpy(packet, &year, sizeof(year));
    packet += sizeof(year);
    // Pack latitude, longitude, speed, angle, and altitude as floats
    float lat = GPS.latitude;
    float lon = GPS.longitude;
    float speed = GPS.speed;
    float angle = GPS.angle;
    float alt = GPS.altitude;
    memcpy(packet, &lat, sizeof(lat));
    packet += sizeof(lat);
    memcpy(packet, &lon, sizeof(lon));
    packet += sizeof(lon);
    memcpy(packet, &speed, sizeof(speed));
    packet += sizeof(speed);
    memcpy(packet, &angle, sizeof(angle));
    packet += sizeof(angle);
    memcpy(packet, &alt, sizeof(alt));
    packet += sizeof(alt);
    // Pack number of satellites as uint8_t
    uint8_t sats = GPS.satellites;
    memcpy(packet, &sats, sizeof(sats));
    packet += sizeof(sats);
    uint8_t antenna_status = GPS.antenna;
    memcpy(packet, &antenna_status, sizeof(antenna_status));
    packet += sizeof(antenna_status);

  } else {
    // If there's no fix, append default zero values
    uint8_t day = 0;
    uint8_t month = 0;
    uint16_t year = 0;
    memcpy(packet, &day, sizeof(day));
    packet += sizeof(day);
    memcpy(packet, &month, sizeof(month));
    packet += sizeof(month);
    memcpy(packet, &year, sizeof(year));
    packet += sizeof(year);
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
    uint8_t antenna_status = 0;
    memcpy(packet, &antenna_status, sizeof(antenna_status));
    packet += sizeof(antenna_status);
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
  uint8_t day = *packet;
  packet += sizeof(uint8_t);
  uint8_t month = *packet;
  packet += sizeof(uint8_t);
  uint16_t year;
  memcpy(&year, packet, sizeof(uint16_t));
  packet += sizeof(uint16_t);

  // Decode values: latitude, longitude, speed, angle, altitude. And then decode
  // number of satellites.
  float lat;
  memcpy(&lat, packet, sizeof(float));
  packet += sizeof(float);

  float lon;
  memcpy(&lon, packet, sizeof(float));
  packet += sizeof(float);

  float speed;
  memcpy(&speed, packet, sizeof(float));
  packet += sizeof(float);

  float angle;
  memcpy(&angle, packet, sizeof(float));
  packet += sizeof(float);

  float alt;
  memcpy(&alt, packet, sizeof(float));
  packet += sizeof(float);

  uint8_t sats = *packet;
  packet += sizeof(uint8_t);

  uint8_t antenna_status = *packet; 
  packet += sizeof(uint8_t); 

  // Construct the CSV string
  String dateStr = String(day) + "/" + String(month) + "/" + String(year);
  String csv = dateStr + "," + String(lat) + "," + String(lon) + "," +
               String(speed) + "," + String(angle) + "," + String(alt) + "," +
               String(sats) + "," + String(antenna_status) + ",";
  return csv;
}