#include "SGP30Sensor.h"

/**
 * @brief Default constructor for the SGP30Sensor.
 */
SGP30Sensor::SGP30Sensor()
    : Sensor("SGP30", "SGP_eCO2,SGP_TVOC,", 2) {}

/**
 * @brief Constructor that sets a minimum period between sensor reads.
 *
 * @param minimum_period Minimum period in ms.
 */
SGP30Sensor::SGP30Sensor(unsigned long minimum_period)
    : Sensor("SGP30", "SGP_eCO2,SGP_TVOC,", 2, minimum_period) {}

/**
 * @brief Verifies that the SGP30 sensor is connected and operational.
 *
 * @return true if sensor initializes correctly, false otherwise.
 */
bool SGP30Sensor::verify() {

  if (!sgp.begin()) {
    return false;
  }
  return true;
}

/**
 * @brief Reads the SGP30 data as a CSV string ("eCO2,TVOC,").
 */
String SGP30Sensor::readData() {
  if (sgp.IAQmeasure()) {
    return String(sgp.eCO2) + "," + String(sgp.TVOC) + ",";
  }
  return "0,0,";
}

/**
 * @brief Appends the SGP30 sensor data (eCO2, TVOC) plus a simple 16-bit
 *        checksum to the packet buffer.
 *
 * Data layout in the packet:
 *   - eCO2: uint16_t (2 bytes)
 *   - TVOC: uint16_t (2 bytes)
 *   - checksum: uint16_t (2 bytes) sum of the above 4 bytes
 *
 * @param packet Pointer to the current position in the packet buffer.
 * @return The number of bytes appended (6).
 */
void SGP30Sensor::readDataPacket(uint8_t*& packet) {
 
  uint16_t eCO2 = 0;
  uint16_t TVOC = 0;
  if (sgp.IAQmeasure()) {
    eCO2 = sgp.eCO2;
    TVOC = sgp.TVOC;
  }

  
  memcpy(packet, &eCO2, sizeof(eCO2));
  packet += sizeof(eCO2);

  memcpy(packet, &TVOC, sizeof(TVOC));
  packet += sizeof(TVOC);

  uint16_t checksum = 0;
  for (int i = 0; i < sizeof(eCO2); i++) {
    checksum += ((uint8_t*)&eCO2)[i];
  }
  for (int i = 0; i < sizeof(TVOC); i++) {
    checksum += ((uint8_t*)&TVOC)[i];
  }

  memcpy(packet, &checksum, sizeof(checksum));
  packet += sizeof(checksum);

  return (sizeof(eCO2) + sizeof(TVOC) + sizeof(checksum));
}

/**
 * @brief Decodes the SGP30 sensor data (eCO2, TVOC) plus checksum from
 *        the packet buffer, and returns a CSV string.
 *
 * Data layout must match the above function:
 *   - eCO2: uint16_t
 *   - TVOC: uint16_t
 *   - checksum: uint16_t
 *
 * @param packet Pointer to the packet buffer. This pointer is advanced by 6 bytes.
 * @return String "eCO2,TVOC,Checksum: X (computed: Y),"
 */
String SGP30Sensor::decodeToCSV(uint8_t*& packet) {
  uint16_t eCO2;
  memcpy(&eCO2, packet, sizeof(eCO2));
  packet += sizeof(eCO2);

  uint16_t TVOC;
  memcpy(&TVOC, packet, sizeof(TVOC));
  packet += sizeof(TVOC);
    
  uint16_t storedChecksum;
  memcpy(&storedChecksum, packet, sizeof(storedChecksum));
  packet += sizeof(storedChecksum);

  uint16_t computedChecksum = 0;
  for (int i = 0; i < sizeof(eCO2); i++) {
    computedChecksum += ((uint8_t*)&eCO2)[i];
  }
  for (int i = 0; i < sizeof(TVOC); i++) {
    computedChecksum += ((uint8_t*)&TVOC)[i];
  }

  String csv = String(eCO2) + "," + String(TVOC) + ",";
  csv += "Checksum: " + String(storedChecksum);
  csv += " (computed: " + String(computedChecksum) + "),";
  return csv;
}
