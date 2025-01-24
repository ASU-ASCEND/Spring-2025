#include <Arduino.h>

// error code framework
#include "ErrorDisplay.h"
#include "PayloadConfig.h"

// parent classes
#include "Sensor.h"
#include "Storage.h"

// include sensor headers here
#include "AS7331Sensor.h"
#include "BME280Sensor.h"
#include "BME680Sensor.h"
#include "ENS160Sensor.h"
#include "ICM20948Sensor.h"
#include "INA260Sensor.h"
#include "LSM9DS1Sensor.h"
#include "MTK3339Sensor.h"
#include "PCF8523Sensor.h"
#include "SGP30Sensor.h"
#include "TempSensor.h"
// #include "DS3231Sensor.h"

// helper function definitions
int verifySensors();
int verifyStorage();
void storeData(String data);
String readSensorData();
void handleDataInterface();

// Global variables
// sensor classes
// clang-format off
// class        sensor            minimum period in ms
BME680Sensor    bme_sensor        (1000);
INA260Sensor    ina260_sensor     (1000);
LSM9DS1Sensor   lsm9ds1_sensor    (0);
TempSensor      temp_sensor       (1000);
SGP30Sensor     sgp30_sensor      (1000);
BME280Sensor    bme280_sensor     (1000);
ENS160Sensor    ens160_sensor     (1000);
AS7331Sensor    uv_sensor_1       (1000, UV_I2C_ADDR_1);
AS7331Sensor    uv_sensor_2       (1000, UV_I2C_ADDR_2);
MTK3339Sensor   gps_sensor        (5000);
ICM20948Sensor  icm_sensor        (20);
PCF8523Sensor   rtc_sensor        (1000);
// DS3231Sensor    rtc_backup_sensor (1000);
// clang-format on

// sensor array
Sensor* sensors[] = {&rtc_sensor,     &bme_sensor,    &ina260_sensor,
                     &lsm9ds1_sensor, &temp_sensor,   &sgp30_sensor,
                     &bme280_sensor,  &ens160_sensor, &uv_sensor_1,
                     &uv_sensor_2,    &icm_sensor,    &gps_sensor};
//&gps_sensor};
const int sensors_len = sizeof(sensors) / sizeof(sensors[0]);
bool sensors_verify[sensors_len];
String header_condensed = "";

// include storage headers here
#include "FlashStorage.h"
#include "RadioStorage.h"
#include "SDStorage.h"

// storage classes
SDStorage sd_storage;
RadioStorage radio_storage;
FlashStorage flash_storage;

// storage array
#if FLASH_SPI1 == 0
Storage* storages[] = {&sd_storage, &radio_storage};
#else
Storage* storages[] = {&sd_storage, &radio_storage, &flash_storage};
#endif

const int storages_len = sizeof(storages) / sizeof(storages[0]);
bool storages_verify[storages_len];

// global variables for main
// loop counter
unsigned int it = 0;

// multicore transfer queue
#define QT_ENTRY_SIZE 1000
#define QT_MAX_SIZE 10

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

  // setup heartbeat pins
  pinMode(HEARTBEAT_PIN_0, OUTPUT);

  // verify sensors
  int verified_count = verifySensors();
  if (verified_count == 0) {
    Serial.println("All sensor communications failed");
    ErrorDisplay::instance().addCode(Error::CRITICAL_FAIL);
    while (1) {
      ErrorDisplay::instance().toggle();
      Serial.println("Error");
      delay(1000);
    }
  } else {
    Serial.println("At least one sensor works, continuing");
    if (verified_count < 5) {
      ErrorDisplay::instance().addCode(Error::LOW_SENSOR_COUNT);
    }
  }

  // verify storage
  Serial.println("Verifying storage...");
  verified_count = verifyStorage();
  if (verified_count == 0) {
    Serial.println("No storages verified, output will be Serial only.");
    ErrorDisplay::instance().addCode(Error::CRITICAL_FAIL);
  }

// spi0
#if FLASH_SPI1 == 0
  if (flash_storage.verify()) {
    Serial.println(flash_storage.getStorageName() + " verified.");
  }
#endif

  // build csv header
  String header = "Header,Millis,";
  for (int i = 0; i < sensors_len; i++) {
    if (sensors_verify[i]) {
      header += sensors[i]->getSensorCSVHeader();
    }
  }
  Serial.println(header);

// store header
// storeData(header);
#if FLASH_SPI1 == 0
  flash_storage.store(header);
#endif

  // send header to core1
  queue_add_blocking(&qt, header.c_str());

  pinMode(ON_BOARD_LED_PIN, OUTPUT);
  Serial.println("Setup done.");

  Serial.println("Testing...");
  uint8_t packet[10]; 
  uint8_t* packet_temp = packet; 
  temp_sensor.readDataPacket(packet_temp); 
  Serial.println("[Core 1] Packet size: " + String(packet_temp - packet)); 
  packet_temp = packet; 
  Serial.println("[Core 1] " + temp_sensor.decodeToCSV(packet_temp)); 
  while(1); 
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

  // switch to data recovery mode
  /*if (digitalRead(DATA_INTERFACE_PIN) == LOW) {
#if FLASH_SPI1
    if (was_dumping == false) {
      while (queue_get_level(&qt) != 0);
      delay(10);
      rp2040.idleOtherCore();
    }
#endif
    was_dumping = true;
    handleDataInterface();
    return;
  }*/

  if (was_dumping == true) {
    Serial.println("\nErasing flash chip....");
    was_dumping = false;
    flash_storage.erase();
#if FLASH_SPI1
    rp2040.resumeOtherCore();
#endif
  }

  // start print line with iteration number
  Serial.print("it: " + String(it) + "\t");

  // build csv row
  String csv_row = readSensorData();

  // print csv row
  Serial.println(csv_row);

  // send data to flash
  flash_storage.store(csv_row);

  // send data to core1
  queue_add_blocking(&qt, csv_row.c_str());

  // delay(500);                                  // remove before flight
  digitalWrite(ON_BOARD_LED_PIN, (it & 0x1));  // toggle light with iteration
}

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

  Serial.println("Pin Verification Results:");
  for (int i = 0; i < sensors_len; i++) {
    Serial.print(sensors[i]->getSensorName());
    Serial.print(": ");
    Serial.println(sensors_verify[i]
                       ? "Successful in Communication"
                       : "Failure in Communication (check wirings and/ or pin "
                         "definitions in the respective sensor header file)");
  }
  Serial.println();
  return count;
}

#define INTER_PACKET 1
#if INTER_PACKET
/**
 * @brief Read data from each verified Sensor (using packet as an intermediary)
 *
 * @return String Complete CSV row for iteration
 */
String readSensorData() {
  //String csv_row = header_condensed + "," + String(millis()) + ",";
  uint8_t packet[500]; 
  uint32_t sync_bytes = 0x89ABCDEF; 
  std::copy((uint8_t*)(&sync_bytes), (uint8_t*)(&sync_bytes) + sizeof(sync_bytes), packet); 
  uint32_t sensor_id = 0; 
  uint16_t data_len = 0; 
  uint8_t* temp_packet = packet + sizeof(sync_bytes) + sizeof(sensor_id) + sizeof(data_len); 

  // build packet 
  // millis()
  unsigned long now = millis(); 
  std::copy((uint8_t*)(&now), (uint8_t*)(&now) + sizeof(now), temp_packet);
  temp_packet += sizeof(now); 
  sensor_id = (sensor_id << 1) | 1; 
  // rest of the packet 
  for (int i = 0; i < sensors_len; i++) {
    if (sensors_verify[i]) {
      sensors[i]->getDataPacket(sensor_id, temp_packet);
    } else {
      sensor_id <<= 1; 
    }
  }
  // calc data len 
  data_len = temp_packet - packet; 

  // write sensor_id
  temp_packet = packet + sizeof(sync_bytes);
  std::copy((uint8_t*)(&sensor_id), (uint8_t*)(&sensor_id) + sizeof(sensor_id), temp_packet); 

  // write data len 
  temp_packet += sizeof(sensor_id); 
  std::copy((uint8_t*)(&data_len), (uint8_t*)(&data_len) + sizeof(data_len), temp_packet); 
  

  // ---------------------------------------------------------------------
  // decode packet (using only packet) 
  temp_packet = packet; 

  uint32_t r_sync_bytes = *((uint32_t*)temp_packet); 
  temp_packet += sizeof(r_sync_bytes); 
  uint32_t r_sensor_id = *((uint32_t*)temp_packet);
  temp_packet += sizeof(r_sensor_id);  
  uint16_t r_data_len = *((uint16_t*)temp_packet);
  temp_packet += sizeof(r_data_len);  
  
  String csv_row = ""; 

  uint32_t temp_sensor_id = r_sensor_id; 
  uint8_t id_offset = 0; 
  for(int i = 0; i < 32; i++){
    if(temp_sensor_id & (1 << i)){
      id_offset = i; 
    }
  }

  // millis decode
  unsigned long r_now = *((unsigned long*)temp_packet); 
  temp_packet += sizeof(r_now); 
  csv_row += String(r_now) + ","; 

  int curr_offset = id_offset - 1; 
  while(temp_packet < packet + r_data_len){
    if(r_sensor_id & (1 << curr_offset)){
      csv_row += sensors[id_offset - curr_offset - 1]->decodeToCSV(temp_packet); 
    } else if(sensors_verify[id_offset - curr_offset -1]){
      csv_row += sensors[id_offset - curr_offset -1]->readEmpty(); 
    }
  }
  
  return csv_row;
}
#else
/**
 * @brief Read data from each verified Sensor
 *
 * @return String Complete CSV row for iteration
 */
String readSensorData() {
  String csv_row = header_condensed + "," + String(millis()) + ",";
  for (int i = 0; i < sensors_len; i++) {
    if (sensors_verify[i]) {
      csv_row += sensors[i]->getDataCSV();
    }
  }
  return csv_row;
}
#endif

/**
 * @brief Verifies the connection with each storage device, and defines the
 * header_condensed field
 *
 * @return int The number of verified storage devices
 */
int verifyStorage() {
  int count = 0;
  for (int i = 0; i < storages_len; i++) {
    storages_verify[i] = storages[i]->verify();
    if (storages_verify[i]) {
      Serial.println(storages[i]->getStorageName() + " verified.");
      count++;
    }
  }
  return count;
}

/**
 * @brief Sends data to each storage device, assumes storage devices take care
 * of newline/data end themselves
 *
 * @param data Data in a CSV formatted string
 */
void storeData(String data) {
  for (int i = 0; i < storages_len; i++) {
    if(storages_verify[i]){
      storages[i]->store(data);
    }
  }
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

/** -------------------------------------------------------------------
 * CORE 1 CODE ONLY AFTER THIS, DO NOT MIX CODE FOR THE CORES
 *  -------------------------------------------------------------------
 * TODO: Implement multicore for communication here
 * setup queue for data transfer between cores
 *
 */

/**
 * @brief Setup for core 1
 *
 *
 */
void setup1() {
  delay(500);  // wait for other setup to run

  pinMode(HEARTBEAT_PIN_1, OUTPUT);

  // verify storage
  if (verifyStorage() == 0) {
    Serial.println("No storages verified, output will be Serial only.");
  }
}

int it2 = 0;
/**
 * @brief Loop for core 1
 *
 */
void loop1() {
  it2++;
  digitalWrite(HEARTBEAT_PIN_1, (it2 & 0x1));

  char received_data[QT_ENTRY_SIZE];
  queue_remove_blocking(&qt, received_data);

  // store csv row
  storeData(String(received_data));

  // Serial.println("[CORE1]\t" + queue_get_level(&qt) + String(received_data));

  // radioStorage.store(String(received_data));
}
