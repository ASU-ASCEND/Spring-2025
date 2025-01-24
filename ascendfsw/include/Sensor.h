#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

/**
 * @brief Parent class for sensor objects
 *
 */
class Sensor {
 private:
  unsigned long minimum_period, last_execution;
  String sensor_name, csv_header;
  int fields;

 public:

  Sensor(String sensor_name, String csv_header, unsigned long minimum_period) {
    this->minimum_period = minimum_period; 
    this->last_execution = 0; 
    this->sensor_name = sensor_name; 
    this->csv_header = csv_header;
    this->fields = 1; 
    for(size_t i = 0; i < csv_header.length(); i++){
      this->fields++; 
    }
  }

  /**
   * @brief Construct a new Sensor object with default minimum_period of 0
   *
   * @param sensor_name The name of the sensor
   * @param csv_header The header for the sensor's csv cells
   * @param fields number of csv cells the sensor will return
   */
  Sensor(String sensor_name, String csv_header, int fields) {
    this->minimum_period = 0;
    this->last_execution = 0;
    this->sensor_name = sensor_name;
    this->csv_header = csv_header;
    this->fields = fields;
  }

  /**
   * @brief Construct a new Sensor object
   *
   * @param sensor_name The name of the sensor
   * @param csv_header The header for the sensor's csv cells
   * @param fields number of csv cells the sensor will return
   * @param minimum_period Set the minimum time between sensor reads in ms
   */
  Sensor(String sensor_name, String csv_header, int fields,
         unsigned long minimum_period) {
    this->minimum_period = minimum_period;
    this->last_execution = 0;
    this->sensor_name = sensor_name;
    this->csv_header = csv_header;
    this->fields = fields;
  }

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
   * @brief Get the Sensor Name as an Arduino string
   *
   * @return const String&
   */
  const String& getSensorName() const { return this->sensor_name; }

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
   * @brief Used for creating packets, reads data from the sensor and appends it to the passed uint8_t array pointer, incrementing it while doing so 
   * 
   * @param packet Pointer to the packet byte array 
   * @return int The number of bytes appended 
   */
  void readDataPacket(uint8_t*& packet){}; 

  String decodeToCSV(uint8_t*& packet){ return ""; };

  /**
   * @brief Append the data from a sensor to the packet if the minium period is satisfied 
   * 
   * @param sensor_id Header sensor packet section 
   * @param packet Pointer to the packet byte array 
   * @return int The number of bytes appended 
   */
  void getDataPacket(uint32_t& sensor_id, uint8_t*& packet){
    if (millis() - this->last_execution >= this->minimum_period) {
      this->last_execution = millis();
      sensor_id = (sensor_id << 1) | 1; 
      readDataPacket(packet);
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
  String readEmpty() const {
    String empty = "";
    for (int i = 0; i < fields; i++) {
      empty += "-,";
    }
    return empty;
  }

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
