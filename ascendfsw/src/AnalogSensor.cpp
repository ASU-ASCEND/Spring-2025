#include "AnalogSensor.h"

/**
 * @brief Construct a new Analog Sensor object with default minimum period of
 * 0ms
 *
 */
AnalogSensor::AnalogSensor() : AnalogSensor(0) {}

/**
 * @brief Construct a new Analog Sensor object
 *
 * @param minium_period Minium period between reading data in ms
 */
AnalogSensor::AnalogSensor(unsigned long minium_period)
    : Sensor("Analog Sensors", "WindADC,TempADC,", 2, minium_period) {}

/**
 * @brief Verify if sensor is connected (for ADC this just returns true)
 *
 * @return true Always returns true
 * @return false Should never return false
 */
bool AnalogSensor::verify() { return true; }

/**
 * @brief Reads ADC values for wind and temp sensor (temp in C)
 *
 * @return String CSV segment for Wind ADC and Temp ADC
 */
String AnalogSensor::readData() {
  int windADCunits = analogRead(WIND_PIN);
  int pt1000tempADC = analogRead(PT1000_PIN);

  return String(windADCunits) + "," + String(pt1000tempADC) + ",";
  // float windMPH =  pow((((float)windADunits - 264.0) / 85.6814), 3.36814);

  // int tempRawAD = analogRead(TEMP_PIN);

  // float tempC = ((((float)tempRawAD * 5.0) / 1024.0) - 0.400) / .0195;

  // return String(windMPH) + ", " + String(tempC) + ", ";
}