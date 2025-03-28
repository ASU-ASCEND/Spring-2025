/**
 * @file main.cpp
 * @brief Main functions for Core 0, responsible for reading data from sensor
 * peripherals
 */
#include <Arduino.h>

// error code framework
#include "ErrorDisplay.h"
#include "Logger.h"
#include "PayloadConfig.h"

// Shared stuctures indicating command-based system status
#include "CommandMessage.h"

// parent classes
#include "Sensor.h"

// include sensor headers here
#include "AS7331Sensor.h"
#include "BMP390Sensor.h"
#include "ENS160Sensor.h"
#include "ICM20948Sensor.h"
#include "INA260Sensor.h"
#include "MTK3339Sensor.h"
#include "OzoneSensor.h"
#include "PCF8523Sensor.h"
#include "SCD40Sensor.h"
#include "SHTC3Sensor.h"
#include "TMP117Sensor.h"
#include "TempSensor.h"

// helper function definitions
int verifySensors();
void handleCommand();
int verifySensorRecovery();
String readSensorData();
uint16_t readSensorDataPacket(uint8_t* packet);
String decodePacket(uint8_t* packet);

void handleDataInterface();

// Global variables
// sensor classes
// clang-format off
// class        sensor            minimum period in ms
INA260Sensor    ina260_sensor     (1000);
TempSensor      temp_sensor       (1000);
MTK3339Sensor   gps_sensor        (2000);
ICM20948Sensor  icm_sensor        (0);
PCF8523Sensor   rtc_sensor        (1000);
BMP390Sensor    bmp_sensor        (500);
OzoneSensor     ozone_sensor      (500);
SCD40Sensor     scd_sensor        (1000,  &Wire); 
TMP117Sensor    tmp_sensor        (500,   &Wire); 
ENS160Sensor    ens160_sensor     (500,   &Wire);
SHTC3Sensor     shtc_sensor       (1000,  &Wire);

// StratoSense
AS7331Sensor    uv_sensor_out     (500, UV_I2C_ADDR);
SCD40Sensor     scd_sensor_out    (1000,  &Wire1);  
TMP117Sensor    tmp_sensor_out    (500,   &Wire1); 
ENS160Sensor    ens160_sensor_out (500,   &Wire1);
SHTC3Sensor     shtc_sensor_out   (1000,  &Wire1);
// clang-format on

// sensor array
Sensor* sensors[] = {
    &rtc_sensor,     &ina260_sensor,  &temp_sensor,     &icm_sensor,
    &gps_sensor,     &bmp_sensor,     &tmp_sensor,      &shtc_sensor,
    &scd_sensor,     &ens160_sensor,  &ozone_sensor,    &uv_sensor_out,
    &scd_sensor_out, &tmp_sensor_out, &shtc_sensor_out, &ens160_sensor_out};

const int sensors_len = sizeof(sensors) / sizeof(sensors[0]);

String header_condensed = "";

// global variables for main

// Global variables shared with core 1

queue_t qt;

uint32_t time_paused;
uint32_t max_pause_duration = 60'000 * 2;

// char qt_entry[QT_ENTRY_SIZE];

/**
 * @brief Setup for core 0
 *
 */
void setup() {
  // multicore setup
  queue_init(&qt, QT_ENTRY_SIZE, QT_MAX_SIZE);
  mutex_init(&cmd_data_mutex);
  ErrorDisplay::instance().addCode(Error::NONE);  // for safety

  // setup i2c1
  Wire1.setSCL(I2C1_SCL_PIN);
  Wire1.setSDA(I2C1_SDA_PIN);

  Wire.begin();
  Wire1.begin();

  // start serial
  Serial.begin(115200);
  // while (!Serial)  // remove before flight
  //   ;
  log_core("setup begin");

  // setup heartbeat pins
  pinMode(HEARTBEAT_PIN_0, OUTPUT);

  // verify sensors
  // recovery config for sensors
  for (int i = 0; i < sensors_len; i++) {
    sensors[i]->recoveryConfig(5, 1000);
  }

  int verified_count = verifySensorRecovery();

  if (verified_count == 0) {
    log_core("All sensor communications failed");
    ErrorDisplay::instance().addCode(Error::CRITICAL_FAIL);
    while (1) {
      ErrorDisplay::instance().toggle();
      log_core("Error");
      delay(1000);
    }
  } else {
    log_core("At least one sensor works, continuing");
    if (verified_count < 5) {
      ErrorDisplay::instance().addCode(Error::LOW_SENSOR_COUNT);
    }
  }

#if 0  // header stuff
  // build csv header
  String header = "Header,Millis,";
  for (int i = 0; i < sensors_len; i++) {
    if (sensors_verify[i]) {
      header += sensors[i]->getSensorCSVHeader();
    }
  }
  log_data(header);

// store header
// storeData(header);
#if FLASH_SPI1 == 0
  flash_storage.store(header);
#endif

  // send header to core1
  queue_add_blocking(&qt, header.c_str());
#endif

  pinMode(ON_BOARD_LED_PIN, OUTPUT);
  log_core("Setup done.");
}

bool was_dumping = false;
// loop counter
unsigned int it = 0;
/**
 * @brief Loop for core 0, handling sensor reads
 *
 */
void loop() {
  // toggle error display
  ErrorDisplay::instance().toggle();

  // toggle heartbeats
  it++;
  digitalWrite(HEARTBEAT_PIN_0, (it & 0x1));

  // Check for serial input commands
  if (Serial.available() > 0) {
    handleCommand();
  }

  // Check if system is paused & skip data collection if so

  if (getCmdData().system_paused) {
    uint32_t remaining_time = millis() - time_paused;

    // Force resume if timeout
    if (remaining_time > max_pause_duration) {
      setCmdData({CMD_NONE, 0, false});
      log_core("ERROR: System Timeout");
    }
  }
  // Read sensor data
  else {
    // start print line with iteration number
    log_core("it: " + String(it) + "\t");

    // build csv row
    uint8_t packet[QT_ENTRY_SIZE];
    // for (int i = 0; i < QT_ENTRY_SIZE; i++) packet[i] = 0; // useful for
    // debugging
    uint16_t packet_len = readSensorDataPacket(packet);
#if STORING_PACKETS == false
    String csv_row = decodePacket(packet);
#endif

    // print csv row
    // log_data(csv_row);
    log_data_raw(packet, packet_len);

// send data to core1
#if STORING_PACKETS
    // queue_add_blocking(&qt, packet);
    queue_try_add(&qt, packet);
#else
    queue_add_blocking(&qt, csv_row.c_str());
#endif

    delay(500);  // remove before flight
  }

  digitalWrite(ON_BOARD_LED_PIN, (it & 0x1));  // toggle light with iteration
}

/**
 * @brief Uses Device abstraction to verify sensors
 * Incompatible with initial header generation
 *
 * @return int Number of sensors verified
 */
int verifySensorRecovery() {
  int count = 0;
  for (int i = 0; i < sensors_len; i++) {
    if (sensors[i]->attemptConnection()) {
      count++;
    }
  }

  log_core("Pin Verification Results:");
  for (int i = 0; i < sensors_len; i++) {
    log_core((sensors[i]->getDeviceName()) + ": " +
             (sensors[i]->getVerified()
                  ? "Successful in Communication"
                  : "Failure in Communication (check wirings and/ or pin "
                    "definitions)"));
  }
  log_core("");
  return count;
}

/**
 * @brief Handle commands read from serial input
 *
 * Pause data collection and storage for a specified duration to execute
 * a command, including STATUS, DOWNLOAD, DELETE, etc.
 *
 */
void handleCommand() {
  // Fetch & format command
  String cmd = Serial.readStringUntil('\n');
  cmd.trim();
  cmd.toUpperCase();

  CommandMessage cmd_data = getCmdData();

  // Process the command
  if (cmd.equals("STATUS")) {
    cmd_data.type = CMD_STATUS;
    log_core("Command: " + cmd + " - System Paused");
    max_pause_duration = 60'000 / 2;
  } else if (cmd.startsWith("DOWNLOAD F")) {
    // Extract the file number from command
    String extracted_num = cmd.substring(String("DOWNLOAD F").length());
    extracted_num.trim();

    log_core("Command [DOWNLOAD]: Download File " + extracted_num +
             " - System Paused");

    // Store the command info
    cmd_data.type = CMD_DOWNLOAD;
    cmd_data.file_number = extracted_num.toInt();

    // Set pause duration
    max_pause_duration = 60'000 * 30;
  } else if (cmd.startsWith("DELETE F")) {
    // Extract the file number from command
    String extracted_num = cmd.substring(String("DELETE F").length());
    extracted_num.trim();

    log_core("Command [DELETE]: Delete File " + extracted_num +
             " - System Paused");

    // Store the command info
    cmd_data.type = CMD_DELETE;
    cmd_data.file_number = extracted_num.toInt();
    max_pause_duration = 60'000 * 15;
  } else if (cmd.equals("FLASH DELETE ALL")) {
    cmd_data.type = CMD_ERASE_ALL;
    max_pause_duration = 60'000 * 5;
  } else {
    log_core("ERROR: Invalid command - " + cmd);
    return;
  }

  cmd_data.system_paused = true;
  time_paused = millis();

  setCmdData(cmd_data);
}

#if 0  // part of the old verification system 
/**
 * @brief Verifies each sensor by calling each verify() function
 *
 * @return int The number of verified sensors
 */
int verifySensors() {
  int count = 0;
  uint32_t bit_array = 0b11;  // start with a bit for header and for millis
                              // (they will always be there)
  for (int i = 0; i < sensors_len; i++) {
    sensors_verify[i] = sensors[i]->verify();
    if (sensors_verify[i]) {
      count++;
      bit_array =
          (bit_array << 1) | 0b1;  // if the sensor is verified shift a 1 in
    } else {
      bit_array = (bit_array << 1);  // otherwise shift a 0 in
    }
  }
  header_condensed =
      String(bit_array, HEX);  // translate it to hex to condense it for the csv

  log_core("Pin Verification Results:");
  for (int i = 0; i < sensors_len; i++) {
    log_core((sensors[i]->getDeviceName()) + ": " +
             (sensors_verify[i]
                  ? "Successful in Communication"
                  : "Failure in Communication (check wirings and/ or pin "
                    "definitions)"));
  }
  log_core("");
  return count;
}
#endif

/**
 * @brief Reads sensor data into a packet byte array
 *
 * @param packet Pointer to the packet array
 */
uint16_t readSensorDataPacket(uint8_t* packet) {
  // set sync bytes
  uint8_t* temp_packet = packet;
  std::copy(SYNC_BYTES, SYNC_BYTES + sizeof(SYNC_BYTES), temp_packet);
  temp_packet += sizeof(SYNC_BYTES);

  uint32_t sensor_id = 0;
  uint16_t packet_len = 0;
  temp_packet += sizeof(sensor_id) + sizeof(packet_len);

  // build packet
  // millis()
  uint32_t now = millis();
  std::copy((uint8_t*)(&now), (uint8_t*)(&now) + sizeof(now), temp_packet);
  temp_packet += sizeof(now);
  sensor_id = (sensor_id << 1) | 1;
  // rest of the packet
  for (int i = 0; i < sensors_len; i++) {
    if (sensors[i]->attemptConnection()) {
      sensors[i]->getDataPacket(sensor_id, temp_packet);
    } else {
      sensor_id <<= 1;
    }
  }

  // calc data len
  packet_len = (temp_packet - packet) + 1;  // + 1 for checksum
  log_core("Packet Len: " + String(packet_len));

  // write sensor_id
  temp_packet = packet + sizeof(SYNC_BYTES);
  std::copy((uint8_t*)(&sensor_id), (uint8_t*)(&sensor_id) + sizeof(sensor_id),
            temp_packet);

  // write data len
  temp_packet += sizeof(sensor_id);
  std::copy((uint8_t*)(&packet_len),
            (uint8_t*)(&packet_len) + sizeof(packet_len), temp_packet);

  // calculate checksum with sum complement parity
  int8_t checksum = 0;
  for (size_t i = 0; i < packet_len - 1; i++) {
    checksum += packet[i];
  }
  *(packet + packet_len - 1) = -checksum;

  return packet_len;
}

/**
 * @brief Decodes the packet to a CSV row
 *
 * @param packet Pointer to the packet array
 * @return String The resulting CSV row
 */
String decodePacket(uint8_t* packet) {
  uint8_t* temp_packet = packet;

  uint32_t sync_bytes = *((uint32_t*)temp_packet);
  temp_packet += sizeof(sync_bytes);
  uint32_t sensor_id = *((uint32_t*)temp_packet);
  temp_packet += sizeof(sensor_id);
  uint16_t packet_len = *((uint16_t*)temp_packet);
  temp_packet += sizeof(packet_len);

  // start with sensor_id in a cell in Hex
  String csv_row = String(sensor_id, HEX) + ",";

  uint32_t sensor_id_temp = sensor_id;
  uint8_t id_offset = 0;
  for (int i = 0; i < 32; i++) {
    if (sensor_id_temp & (1 << i)) {
      id_offset = i;
    }
  }

  // millis decode
  // the words aren't aligned because of the uint16_t len
  // so casting will crash the pico
  uint32_t r_now;
  memcpy(&r_now, temp_packet, sizeof(uint32_t));

  temp_packet += sizeof(r_now);
  csv_row += String(r_now) + ",";

  int curr_offset = id_offset - 1;
  while (curr_offset >= 0) {
    if (sensor_id & (1 << curr_offset)) {
      // log_core("\tDecoding " + sensors[id_offset - curr_offset -
      // 1]->getDeviceName());
      csv_row += sensors[id_offset - curr_offset - 1]->decodeToCSV(temp_packet);
    } else if (sensors[id_offset - curr_offset - 1]->getVerified()) {
      csv_row += sensors[id_offset - curr_offset - 1]->readEmpty();
    } else {
    }
    curr_offset--;
  }

  // check parity
  uint8_t sum = 0;
  for (uint16_t i = 0; i < packet_len - 1; i++) {
    sum += packet[i];
  }
  sum += *(int8_t*)(packet + packet_len - 1);
  log_core("Sum = " + String(sum));

  return csv_row;
}

/**
 * @brief Read data from each verified Sensor
 *
 * @return String Complete CSV row for iteration
 */
String readSensorData() {
  String csv_row = header_condensed + "," + String(millis()) + ",";
  for (int i = 0; i < sensors_len; i++) {
    if (sensors[i]->attemptConnection()) {
      csv_row += sensors[i]->getDataCSV();
    }
  }
  return csv_row;
}

//-------------------------------------
// Core 1 Calls
//-------------------------------------
// declarations - definitions are in main1.cpp
void real_setup1();
void real_loop1();

void setup1() { real_setup1(); }

void loop1() { real_loop1(); }

//-------------------------------------