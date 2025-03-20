#ifndef FLASH_STORAGE_H
#define FLASH_STORAGE_H

#include <SPI.h>

#include <vector>

#include "PayloadConfig.h"
#include "SparkFun_SPI_SerialFlash.h"
#include "Storage.h"

struct FileHeader {
  uint32_t file_number;
  uint32_t start_address;
  uint32_t end_address;
  uint8_t complete_flag;   // 0 = writing/incomplete, 1 = complete
  uint16_t crc;           

class FlashStorage : public Storage {
 private:
#if FLASH_SPI1
  SPIClassRP2040 flash_spi_1 = SPIClassRP2040(spi1, SPI1_MISO_PIN, FLASH_CS_PIN,
                                              SPI1_SCK_PIN, SPI1_MOSI_PIN);
#endif

  inline static const uint32_t FILE_HEADER = 0xDEADBEEF;  // 4 bytes
  inline static const uint32_t MAX_SIZE =
      15'000'000;  // 16 MByte (less for safety)
  inline static const uint32_t SECTOR_SIZE = 4'096;  // 4KB
  inline static const uint32_t START_ADDRESS = 0;

  std::vector<FileHeader> file_data;
  SFE_SPI_FLASH flash;
  uint32_t address = 0;
  bool active_file = false;

  void indexFlash();
  void loadAddress();
  bool isSectorEmpty();

  bool readFileHeader();
  void writeFileHeader();

  // New function to calculate a simple CRC (or checksum) for a file.
  uint16_t computeCRC(uint32_t start, uint32_t end);
  
 public:
  FlashStorage();
  bool verify() override;
  void store(String) override;
  void storePacket(uint8_t*) override;
  void dump();
  void erase();
  
  // New function for deletion of a file (sector aligned deletion)
  void deleteFile(uint32_t fileNumber);
  void getStatus();
};

#endif