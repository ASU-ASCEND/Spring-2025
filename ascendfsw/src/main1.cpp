/**
 * @file main1.cpp
 * @brief Main functions for Core 1, responsible for sending data to storage
 * peripherals
 */
#include <Arduino.h>

// error code framework
#include "ErrorDisplay.h"
#include "Logger.h"
#include "PayloadConfig.h"
#include "Storage.h"

int verifyStorage();
int verifyStorageRecovery();
void storeData(String data);
void storeDataPacket(uint8_t* packet);

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
#if RECOVERY_SYSTEM == 0
bool storages_verify[storages_len];
#endif 

// use definition in main.cpp
extern queue_t qt;

// separate 8k stacks
bool core1_separate_stack = true;

/**
 * @brief Setup for core 1
 *
 *
 */
void real_setup1() {
  while(!Serial); 
  delay(500);  // wait for other setup to run
  log_core("Setup1 begin"); 

  // set up heartbeat
  pinMode(HEARTBEAT_PIN_1, OUTPUT);

  // set storages to be tried forever, with 1 second recovery factor
  for (size_t i = 0; i < storages_len; i++) {
    storages[i]->recoveryConfig(-1, 1000);
  }

  // verify storage
  log_core("Verifying storage...");
  int verified_count = verifyStorageRecovery();
  if (verified_count == 0) {
    log_core("No storages verified, output will be Serial only.");
    ErrorDisplay::instance().addCode(Error::CRITICAL_FAIL);
  }

  delay(500);  // wait for other setup to run
}

int it2 = 0;
/**
 * @brief Loop for core 1
 *
 */
void real_loop1() {
  log_core("it2: " + String(it2)); 
  it2++;
  digitalWrite(HEARTBEAT_PIN_1, (it2 & 0x1));

  char received_data[QT_ENTRY_SIZE];
  queue_remove_blocking(&qt, received_data);

  log_core("Received: " + String(received_data)); 

  // store csv row
  storeData(String(received_data));
}

/**
 * @brief Verifies the connection with each storage device, and defines the
 * header_condensed field, uses recovery system
 *
 * @return int The number of verified storage devices
 */
int verifyStorageRecovery() {
  int count = 0;
  for (int i = 0; i < storages_len; i++) {
    if (storages[i]->attemptConnection()) {
      log_core(storages[i]->getDeviceName() + " verified.");
      count++;
    } else {
      log_core(storages[i]->getDeviceName() + " NOT verified");
    }
  }
  return count;
}

/**
 * @brief Verifies the connection with each storage device, and defines the
 * header_condensed field
 *
 * @return int The number of verified storage devices
 */
int verifyStorage() {
  int count = 0;
  for (int i = 0; i < storages_len; i++) {
    #if RECOVERY_SYSTEM 
    if (storages[i]->attemptConnection()) {
    #else
    storages_verify[i] = storages[i]->verify();
    if (storages_verify[i]) {
    #endif 
      log_core(storages[i]->getDeviceName() + " verified.");
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
#if RECOVERY_SYSTEM
    if (storages[i]->attemptConnection()) {
#else
    if (storages_verify[i]) {
#endif
      storages[i]->store(data);
    }
  }
}

/**
 * @brief Sends data to each storage device
 *
 * @param packet Pointer to packet bytes
 */
void storeDataPacket(uint8_t* packet) {
  // pull length of packet out
  uint16_t packet_len;
  memcpy(&packet_len, packet + sizeof(SYNC_BYTES) + sizeof(uint32_t),
         sizeof(uint16_t));

  for (int i = 0; i < storages_len; i++) {
#if RECOVERY_SYSTEM
    if (storages[i]->attemptConnection()) {
#else
    if (storages_verify[i]) {
#endif
      storages[i]->store(packet, packet_len);
    }
  }
}