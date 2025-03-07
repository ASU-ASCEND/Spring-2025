#ifndef RADIO_STORAGE_H
#define RADIO_STORAGE_H

#include <Arduino.h>

#include "PayloadConfig.h"
#include "Storage.h"

/**
 * @brief Implementation of a Storage device to interface with an SD card
 *
 */
class RadioStorage : public Storage {
 private:
  String file_name;

 public:
  RadioStorage();
  bool verify() override;
  void store(String data) override;
  void storePacket(uint8_t* packet) override;
};

#endif