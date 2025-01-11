#ifndef GEIGERSENSOR_H
#define GEIGERSENSOR_H

#include "Arduino.h"
#include "Sensor.h"

#define GEIGER_PIN A2

static volatile int count;

/**
 * @class GeigerSensor
 * @brief A class for interfacing with a Geiger counter sensor.
 *
 * The GeigerSensor class is responsible for measuring counts per second (CPS)
 * from a Geiger counter to detect radiation levels. This class utilizes an
 * interrupt to increment the count whenever radiation particles are detected by
 * the sensor.
 *
 * Key functionalities:
 * - Attaches an interrupt to the sensor pin to track counts.
 * - Reads the current CPS value over a set period and provides the data in CSV
 * format.
 * - Provides functionality to verify the sensor connection and reset counts
 * after each reading.
 */
class GeigerSensor : public Sensor {
 private:
  int measuringPeriodStart = 0;

  /**
   * @brief Interrupt function to count pulses from the Geiger counter.
   *
   * This function is triggered on each falling edge of the Geiger counter's
   * signal and increments the count, which represents the number of radiation
   * events detected.
   */
  static void geigerCounter() { count++; }

 public:
  GeigerSensor();
  GeigerSensor(unsigned long minimum_period);

  bool verify() override;
  String readData() override;
};

#endif
