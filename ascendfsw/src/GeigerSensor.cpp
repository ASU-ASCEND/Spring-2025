#include "GeigerSensor.h"

/**
 * @brief Default constructor for the GeigerSensor class.
 *
 * Initializes the sensor object with a default minimum period of 0
 * milliseconds.
 */
GeigerSensor::GeigerSensor() : GeigerSensor(0) {}

/**
 * @brief Parameterized constructor for the GeigerSensor class.
 *
 * This constructor initializes the GeigerSensor with a specified minimum period
 * between sensor readings. It also passes specific details like the sensor
 * name, CSV header, and number of fields to the base Sensor class constructor.
 *
 * @param minimum_period The minimum time (in milliseconds) between consecutive
 * sensor reads.
 */
GeigerSensor::GeigerSensor(unsigned long minimum_period)
    : Sensor("GeigerSensor", "GeigerSensor(CPS),", 1, minimum_period) {}

/**
 * @brief Verifies the connection to the sensor and sets up the interrupt.
 *
 * @return true - If the sensor is successfully verified.
 * @return false - (Not applicable for this sensor, always returns true).
 */
bool GeigerSensor::verify() {
  attachInterrupt(digitalPinToInterrupt(GEIGER_PIN), geigerCounter, FALLING);
  measuringPeriodStart = millis();
  return true;
}

/**
 * @brief Reads the sensor data in CPS (counts per second) format.
 *
 * This function calculates the CPS value based on the number of counts recorded
 * over a set time period and resets the count for the next measurement period.
 *
 * @return String - The current CPS reading in CSV format.
 */
String GeigerSensor::readData() {
  int samplePeriod = millis() - measuringPeriodStart;

  noInterrupts();
  float CPS = count / (samplePeriod / 1000.0);
  count = 0;
  interrupts();

  measuringPeriodStart = millis();

  return String(CPS) + ",";
}
