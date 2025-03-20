#ifndef SD_STORAGE_H
#define SD_STORAGE_H

#include "ErrorDisplay.h"
#include "PayloadConfig.h"
#include "SD.h"
#include "Storage.h"

/**
 * @brief Implementation of a Storage device to interface with an SD card
 *
 */
class SDStorage : public Storage {
 private:
  String file_name;
#if SD_SPI1
  SPIClassRP2040 sd_spi_1 = SPIClassRP2040(spi1, SPI1_MISO_PIN, SD_CS_PIN,
                                           SPI1_SCK_PIN, SPI1_MOSI_PIN);
#endif

 public:
  SDStorage();
  bool verify() override;
  void store(String data) override;
  void storePacket(uint8_t* packet) override;
};

#endif