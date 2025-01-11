#include "SGP30Sensor.h"

/**
 * @brief Default constructor for the SGP30Sensor, sets the miniumum period to 0
 * ms
 *
 */
SGP30Sensor::SGP30Sensor() : SGP30Sensor(0) {}

/**
 * @brief Constructor for the SGP30Sensor
 *
 * @param minimum_period Minumum period between sensor reads in ms
 */
SGP30Sensor::SGP30Sensor(unsigned long minimum_period)
    : Sensor("SGP30",
             "SGPTVOC(ppb),SGPeCO2(ppm),SGPTVOCBase(ppb),"
             "SGPeCO2Base(ppb),SGPH2,SGPEtha,",
             6, minimum_period) {}

/**
 * @brief Verifies if the SGP30 sensor is connected
 *
 * @return true if the sensor is connected
 * @return false if the sensor is not connected
 */
bool SGP30Sensor::verify() { return sgp.begin(); }

/**
 * @brief Reads and returns data from the SGP30 sensor
 *
 * @return String A CSV section in the form SGPTVOC(ppb), SGPeCO2(ppm),
 * SGPTVOCBase(ppb), SGPeCO2Base(ppb), SGPH2, SGPEtha,
 */
String SGP30Sensor::readData() {
  String output = "";
  if (sgp.IAQmeasure()) {
    output += String(sgp.TVOC) + ",";
    output += String(sgp.eCO2) + ",";
  } else {
    output += "-,-,";
  }

  uint16_t TVOC_base, eCO2_base;
  if (sgp.getIAQBaseline(&eCO2_base, &TVOC_base)) {
    output += String(TVOC_base) + "," + String(eCO2_base) + ",";
  } else {
    output += "-,-,";
  }

  if (sgp.IAQmeasureRaw()) {
    output += String(sgp.rawH2) + ",";
    output += String(sgp.rawEthanol) + ",";
  } else {
    output += "-,-,";
  }

  return output;
}