#include "SGP30Sensor.h"

/**
 * @brief Default constructor for SGP30Sensor.
 *
 * Initializes the sensor with the name "SGP30", a CSV header of "SGP_eCO2,SGP_TVOC," and indicates that it returns 2 fields.
 */
SGP30Sensor::SGP30Sensor()
    : Sensor("SGP30", "SGP_eCO2,SGP_TVOC,", 2) {
}

/**
 * @brief Constructor for SGP30Sensor with a specified minimum period between sensor reads.
 *
 * @param minimum_period Minimum period (in milliseconds) between sensor reads.
 */
SGP30Sensor::SGP30Sensor(unsigned long minimum_period)
    : Sensor("SGP30", "SGP_eCO2,SGP_TVOC,", 2, minimum_period) {
}

/**
 * @brief Verifies that the SGP30 sensor is connected and operational.
 *
 * Calls sgp.begin() to initialize the sensor.
 *
 * @return true if the sensor was successfully initialized, false otherwise.
 */
bool SGP30Sensor::verify() {
  if (!sgp.begin()) {
    return false;
  }
  return true;
}

/**
 * @brief Reads sensor data from the SGP30 sensor and returns it as a CSV-formatted String.
 *
 * The function calls sgp.IAQmeasure() to obtain the sensor readings.
 * If the measurement is successful, it returns a CSV string containing eCO2 and TVOC values.
 * If not, it returns default zeros.
 *
 * @return String A CSV string in the form "eCO2,TVOC,".
 */
String SGP30Sensor::readData() {
  if (sgp.IAQmeasure()) {
    return String(sgp.eCO2) + "," + String(sgp.TVOC) + ",";
  }
  return "0,0,";
}
