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
ENS160Sensor    ens160_sensor     (500);
AS7331Sensor    uv_sensor         (500, UV_I2C_ADDR_1);
MTK3339Sensor   gps_sensor        (2000);
ICM20948Sensor  icm_sensor        (0);
PCF8523Sensor   rtc_sensor        (1000);
BMP390Sensor    bmp_sensor        (500);
TMP117Sensor    tmp_sensor        (500); 
SHTC3Sensor     shtc_sensor       (1000);
SCD40Sensor     sdc_sensor        (1000); 
OzoneSensor     ozone_sensor      (500);

// clang-format on

// sensor array
Sensor* sensors[] = {&rtc_sensor, &ina260_sensor, &temp_sensor,
                     &uv_sensor,  &icm_sensor,    &gps_sensor,
                     &bmp_sensor, &tmp_sensor,    &shtc_sensor,
                     &sdc_sensor, &ens160_sensor, &ozone_sensor};

const int sensors_len = sizeof(sensors) / sizeof(sensors[0]);

String header_condensed = "";

// for flash data recovery
#include "FlashStorage.h"
// defined in main1.cpp
extern FlashStorage flash_storage;

// global variables for main
// loop counter
unsigned int it = 0;

queue_t qt;
// char qt_entry[QT_ENTRY_SIZE];

/**
 * @brief Setup for core 0
 *
 */
void setup() {
  // multicore setup
  queue_init(&qt, QT_ENTRY_SIZE, QT_MAX_SIZE);
  ErrorDisplay::instance().addCode(Error::NONE);  // for safety

  // start serial
  Serial.begin(115200);
  while (!Serial)  // remove before flight
    ;
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

// spi0
#if FLASH_SPI1 == 0
  if (flash_storage.verify()) {
    log_core(flash_storage.getDeviceName() + " verified.");
  }
#endif

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
/**
 * @brief Loop for core 0, handling sensor reads
 *
 */
void loop() {
  it++;

  // toggle error display
  ErrorDisplay::instance().toggle();

  // toggle heartbeats
  digitalWrite(HEARTBEAT_PIN_0, (it & 0x1));

  // switch to data recovery mode is commented out for testing without switch
  // installed
  /*if (digitalRead(DATA_INTERFACE_PIN) == LOW) { // will be replaced with
Software control #if FLASH_SPI1 if (was_dumping == false) { while
(queue_get_level(&qt) != 0)
        ;
      delay(10);
      rp2040.idleOtherCore();
    }
#endif
    was_dumping = true;
    handleDataInterface();
    return;
  }

  if (was_dumping == true) {
    log_core("\nErasing flash chip....");
    was_dumping = false;
    flash_storage.erase();
#if FLASH_SPI1
    rp2040.resumeOtherCore();
#endif
  }*/

  // start print line with iteration number
  log_core("it: " + String(it) + "\t");

  // build csv row
  uint8_t packet[QT_ENTRY_SIZE];
  // for (int i = 0; i < QT_ENTRY_SIZE; i++) packet[i] = 0; // useful for
  // debugging
  uint16_t packet_len = readSensorDataPacket(packet);
  String csv_row = decodePacket(packet);

  // print csv row
  // log_data(csv_row);
  log_data_raw(packet, packet_len);

// send data to core1
#if STORING_PACKETS
  queue_add_blocking(&qt, packet);
#else
  queue_add_blocking(&qt, csv_row.c_str());
#endif

  // delay(1000);                                 // remove before flight
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

/**
 * @brief Handles data interface mode for retrieving data from flash memory
 *
 */
void handleDataInterface() {
  static unsigned long last_dump = 0;
  // dump every 30 seconds
  if (millis() - last_dump > 30'000) {
    flash_storage.dump();
    last_dump = millis();
  }
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