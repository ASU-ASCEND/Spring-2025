#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

#include "Device.h"
#include "Logger.h"

/**
 * @brief Parent class for sensor objects
 *
 */
class Sensor : public Device {
 private:
  unsigned long minimum_period, last_execution;
  String csv_header, empty_csv;

 protected:
  int num_fields;

 public:
  Sensor(String sensor_name, String csv_header, unsigned long minimum_period)
      : Device(sensor_name) {
    this->minimum_period = minimum_period;
    this->last_execution = 0;
    this->csv_header = csv_header;
    this->empty_csv = "";
    this->num_fields = 0;
    for (size_t i = 0; i < csv_header.length(); i++) {
      if (csv_header[i] == ',') {
        this->empty_csv += "-,";
        this->num_fields++;
      }
    }
  }

  Sensor(String sensor_name, String csv_header)
      : Sensor(sensor_name, csv_header, 0UL) {}

  /**
   * @brief Construct a new Sensor object with default minimum_period of 0
   * (depreciated)
   *
   * @param sensor_name The name of the sensor
   * @param csv_header The header for the sensor's csv cells
   * @param fields number of csv cells the sensor will return
   */
  Sensor(String sensor_name, String csv_header, int fields)
      : Sensor(sensor_name, csv_header) {}

  /**
   * @brief Construct a new Sensor object (Depreciated)
   *
   * @param sensor_name The name of the sensor
   * @param csv_header The header for the sensor's csv cells
   * @param fields number of csv cells the sensor will return
   * @param minimum_period Set the minimum time between sensor reads in ms
   */
  Sensor(String sensor_name, String csv_header, int fields,
         unsigned long minimum_period)
      : Sensor(sensor_name, csv_header, minimum_period) {}

  /**
   * @brief Get the minimum minimum_period between sensor reads in ms
   *
   * @return unsigned long
   */
  unsigned long getPeriod() const { return minimum_period; }

  /**
   * @brief Set the minimum minimum_period between sensor reads in ms
   *
   * @param minimum_period The new minimum_period in ms
   */
  void setPeriod(int minimum_period) { this->minimum_period = minimum_period; }

  /**
   * @brief Get the system time of the last execution in ms
   *
   * @return unsigned long
   */
  unsigned long getLastExecution() const { return last_execution; }

  /**
   * @brief Set the system time of the last execution in ms
   *
   * @param last_execution System time of last execution in ms
   */
  void setLastExecution(int last_execution) {
    this->last_execution = last_execution;
  }

  /**
   * @brief Get the csv header string associated with this sensor
   *
   * @return const String&
   */
  const String& getSensorCSVHeader() const { return this->csv_header; }

  /**
   * @brief Verifies if the sensor is connected and working
   *
   * @return true, if connected
   * @return false, if not connected
   */
  virtual bool verify() = 0;

  /**
   * @brief Returns the collected data from the sensor in CSV format
   *
   * @return String
   */
  virtual String readData() = 0;

  /**
   * @brief Used for creating packets, reads data from the sensor and appends it
   * to the passed uint8_t array pointer, incrementing it while doing so
   *
   * @param packet Pointer to the packet byte array
   * @return int The number of bytes appended
   */
  virtual void readDataPacket(uint8_t*& packet) {};

  /**
   * @brief Used for onboard decoding of packets
   *
   * @param packet  Pointer to the packet byte array
   * @return String The senors data decoded from the packet in csv format
   */
  virtual String decodeToCSV(uint8_t*& packet) {
    return "(" + this->getDeviceName() + " data), ";
  };

  /**
   * @brief Append the data from a sensor to the packet if the minium period is
   * satisfied
   *
   * @param sensor_id Header sensor packet section
   * @param packet Pointer to the packet byte array
   * @return int The number of bytes appended
   */
  void getDataPacket(uint32_t& sensor_id, uint8_t*& packet) {
    if (millis() - this->last_execution >= this->minimum_period) {
      uint8_t* before = packet;
      readDataPacket(packet);

      sensor_id = (sensor_id << 1);
      if (packet != before) {
        this->last_execution = millis();
        sensor_id |= 1;
      }
    } else {
      sensor_id = (sensor_id << 1) | 0;
    }
  }

  /**
   * @brief Returns CSV line in the same format as readData() but with "-"
   * instead of data
   *
   * @return String
   */
  String readEmpty() const { return this->empty_csv; }

  /**
   * @brief Uses readData and readEmpty to get the data-filled or empty-celled
   * CSV line for the sensor
   *
   * @return String
   */
  String getDataCSV() {
    if (millis() - this->last_execution >= this->minimum_period) {
      this->last_execution = millis();
      return readData();
    } else {
      return readEmpty();
    }
  }
};

#endif
