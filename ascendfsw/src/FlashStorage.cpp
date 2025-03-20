#include "FlashStorage.h"

/**
 * @brief Constructs a new FlashStorage object.
 *
 * Initializes the internal write address to 0 and sets the storage name to
 * "Flash Storage."
 */
FlashStorage::FlashStorage() : address(0), Storage("Flash Storage") {}

/**
 * @brief Locates the next available file block & tracks existing files.
 *
 * Reads 4KB blocks from flash starting at 0 and increments through memory
 * until a free (0xFF) location is found or the max size is reached.
 * Also checks for file headers to record their locations for quick reference.
 */
void FlashStorage::indexFlash() {
  bool empty = this->isSectorEmpty();
  // Iterate through flash by sectors until free space is reached.
  while (!empty && (this->address < this->MAX_SIZE)) {
    // Check for a file header at the start of the sector.
    if (!empty && this->readFileHeader()) {
      // Update the end address of the previously located file.
      if (!this->file_data.empty()) {
        this->file_data.back().end_address = this->address - 4;
        log_flash("File " + String(this->file_data.back().file_number) +
                  " Size: " + String(this->file_data.back().start_address) +
                  " to " + String(this->file_data.back().end_address));
      }
      // Record a new file using the detected header.
      this->file_data.push_back({(this->file_data.size() + 1), this->address,
                                 this->address, 1, 0});
      log_flash("File " + String(this->file_data.size()) + " at address " +
                String(this->address));
    }
    // Advance to the next sector (4KB).
    this->address += this->SECTOR_SIZE;
    empty = this->isSectorEmpty();
  }
  // Write a new file header at the current sector, marking it as in-progress.
  this->writeFileHeader(false);
  this->active_file = true;  // Prevent creation of a new file until power is cycled.
}


/**
 * @brief Locates the next available write address in flash.
 *
 * Reads bytes from flash starting at the current address and increments
 * through memory until finding a free (0xFF) location or reaching the max size.
 */
void FlashStorage::loadAddress() {
  uint8_t currentByte = this->flash.readByte(this->address);

  // Shuffle through data at last recorded address until free space is reached
  while ((currentByte != 0xFF) && (this->address < this->MAX_SIZE)) {
    currentByte = this->flash.readByte(++this->address);
  }
}

/**
 * @brief Determines if the current sector is empty.
 *
 * Checks the next 16 bytes for 0xFF to determine if the sector is empty.
 *
 * @return true  - Sector is empty
 * @return false - Sector is not empty
 */
bool FlashStorage::isSectorEmpty() {
  // Check the next 16 bytes for 0xFF
  for (int i = 0; i < 16; ++i) {
    if (this->flash.readByte(this->address + i) != 0xFF) return false;
  }
  return true;
}

/**
 * @brief Writes the file header at the start of a new sector.
 *
 * Writes the file header magic and metadata. Optionally marks the file as complete.
 *
 * @param markComplete When true, sets the header as complete; otherwise, it is in-progress.
 */
void FlashStorage::writeFileHeader(bool markComplete) {
  uint8_t num_bytes = sizeof(FILE_HEADER_MAGIC) * 8;
  // Write the file header magic to the start of the sector.
  while (num_bytes != 0) {
    num_bytes -= 8;
    this->flash.writeByte(this->address++,
                          (FILE_HEADER_MAGIC >> num_bytes) & 0xFF);
    this->flash.blockingBusyWait();
  }
  // Create header metadata.
  FileHeader header;
  header.file_number = this->file_data.size() + 1;
  header.start_address = this->address - 4;  // position where header magic begins.
  header.end_address = header.start_address; // will get updated as data is written.
  header.complete_flag = markComplete ? 1 : 0;
  header.crc = 0;  // To be computed when the file is finalized.

  // Write the complete_flag (1 byte).
  this->flash.writeByte(this->address++, header.complete_flag);
  this->flash.blockingBusyWait();

  // Write the crc (2 bytes), initially set to 0.
  for (int i = 1; i >= 0; i--) {
    this->flash.writeByte(this->address++, 0);
    this->flash.blockingBusyWait();
  }

  // Record the header locally.
  this->file_data.push_back(header);
  log_flash("New file " + String(header.file_number) + " created at address " +
            String(header.start_address));
}


/**
 * @brief Determines if the first 4 bytes of a sector contain the file header magic.
 *
 * @return true if the file header is present, false otherwise.
 */
bool FlashStorage::readFileHeader() {
  uint8_t num_bytes = sizeof(FILE_HEADER_MAGIC) * 8;
  for (int i = 0; i < sizeof(FILE_HEADER_MAGIC); ++i) {
    num_bytes -= 8;
    if (this->flash.readByte(this->address + i) !=
        ((FILE_HEADER_MAGIC >> num_bytes) & 0xFF)) {
      return false;
    }
  }
  return true;
}


/**
 * @brief Verifies the flash connection and prepares for data storage.
 *
 * Initializes the flash memory, resets the write address, and indexes existing files.
 *
 * @return true if initialization is successful, false otherwise.
 */
bool FlashStorage::verify() {
  #if FLASH_SPI1
    if (this->flash.begin(FLASH_CS_PIN, 2000000UL, this->flash_spi_1) == false)
      return false;
  #else
    if (this->flash.begin(FLASH_CS_PIN) == false) return false;
  #endif
  
    if (this->address >= this->MAX_SIZE) {
      log_flash("Flash memory is full.");
      return false;
    }
  
    if (!active_file) {
      this->address = this->START_ADDRESS;
      log_flash("Initial flash address: " + String(this->address));
      this->indexFlash();
      log_flash("Updated address: " + String(this->address) + " in sector " +
                String(this->address / this->SECTOR_SIZE));
    } else {
      log_flash("Flash storage is active, writing to File " +
                String(this->file_data.back().file_number) + " at address " +
                String(this->address) + " in sector " +
                String(this->address / this->SECTOR_SIZE));
    }
    log_flash("Remaining space: " + String(this->MAX_SIZE - this->address) +
              " bytes");
    return true;
  }
  
/**
 * @brief Stores a string in flash memory, appending a newline at the end.
 *
 * Iterates through each character of the string and writes it to flash. The
 * internal write address is then advanced accordingly.
 *
 * @param data - The string to be stored in flash.
 */
void FlashStorage::store(String data) {
  data = data + "\n";

  for (const uint8_t& character : data) {
    this->flash.writeByte(this->address++, character);
    this->flash.blockingBusyWait();
  }

  // Update file length tracking
  this->file_data.back().end_address += data.length();

  // Log the number of bytes written
  log_core("Writing " + String(data.length()) + " bytes at " +
           String(this->address));
  log_core("File " + String(this->file_data.back().file_number) + " Size: " +
           String(this->file_data.back().start_address) + " to " +
           String(this->file_data.back().end_address));

  this->flash.blockingBusyWait();
}

/**
 * @brief Stores a packet of bytes in flash memory.
 *
 * Iterates through each byte of the packet and writes it to flash. Takes into
 * account initial header bytes, consisting of sync(4), presence(4), and length
 * fields(2).
 *
 * @param packet - Pointer to the byte array containing the packet.
 */
void FlashStorage::storePacket(uint8_t* packet) {
  // -- 4 bytes (sync) -- 4 bytes (presence) -- 2 bytes (length) -- data...
  uint16_t packet_len;
  memcpy(&packet_len, (packet + 8), sizeof(uint16_t));
  uint16_t packet_header = 4 + 4 + 2;  // 10 bytes (sync, presence, length)

  // Iterate through packet and write to flash
  for (uint16_t i = 0; i < packet_len; i++) {
    this->flash.writeByte(this->address++, packet[packet_header + i]);
    this->flash.blockingBusyWait();
  }

  // Update file length tracking
  this->file_data.back().end_address += packet_len;

  // Log the number of bytes written
  log_core("Writing " + String(packet_len) + " bytes at " +
           String(this->address));
  log_core("File " + String(this->file_data.back().file_number) + " Size: " +
           String(this->file_data.back().start_address) + " to " +
           String(this->file_data.back().end_address));
           
  this->flash.blockingBusyWait();
}

/**
 * @brief Dumps the contents of flash memory to the serial monitor.
 *
 * Reads each byte of flash memory starting from the beginning and prints it to
 * the serial monitor. The process continues until the end of the memory or the
 * end code (0xFF) is reached.
 */
void FlashStorage::dump() {
  log_core("\nStarting data transfer: ");
  log_core("Address is at " + String(this->address));
  char data = '^';
  uint32_t pos = this->START_ADDRESS;
  // read until it hits end_code
  while ((pos < this->MAX_SIZE) && (pos < this->address) && (data != 0xFF)) {
    digitalWrite(HEARTBEAT_PIN_0, (pos & 0x60) != 0);
    digitalWrite(HEARTBEAT_PIN_1, (pos & 0x60) != 0);
    data = this->flash.readByte(pos++);
    Serial.write(data);  // print as a character
    // delay(1);
  }
}

/**
 * @brief Erases all data stored in flash memory.
 *
 * Erases all data stored in flash memory by calling the flash erase function
 * and resetting the internal write address to 0.
 */
void FlashStorage::erase() {
  this->flash.erase();
  this->address = 0;
}

/**
 * @brief Computes a simple CRC (or checksum) for the data between two addresses.
 *
 * @param start Starting flash address.
 * @param end Ending flash address.
 * @return A 16-bit CRC value computed over the data.
 */
uint16_t FlashStorage::computeCRC(uint32_t start, uint32_t end) {
  uint16_t crc = 0;
  for (uint32_t addr = start; addr < end; addr++) {
    crc += this->flash.readByte(addr);
  }
  return crc;
}

/**
 * @brief Prints the current status of flash storage.
 *
 * Logs the current address, remaining space, and details for each stored file.
 */
void FlashStorage::getStatus() {
  log_flash("==== FLASH STORAGE STATUS ====");
  log_flash("Address: " + String(this->address));
  log_flash("Remaining Storage: " + String(this->MAX_SIZE - this->address) +
            " bytes");

  log_flash("Stored Files:");
  for (const FileHeader& file : this->file_data) {
    int file_size = file.end_address - file.start_address;
    log_flash("File " + String(file.file_number) +
              " || Size: " + String(file_size) + " bytes");
  }
}

/**
 * @brief Deletes a file from flash memory by erasing its occupied sectors.
 *
 * Locates the file header based on file number and erases each sector within its boundaries.
 *
 * @param fileNumber The file number to delete.
 */
void FlashStorage::deleteFile(uint32_t fileNumber) {
  for (const FileHeader& file : this->file_data) {
    if (file.file_number == fileNumber) {
      uint32_t sector_start = file.start_address - (file.start_address % SECTOR_SIZE);
      uint32_t sector_end = file.end_address;
      while (sector_start < sector_end) {
        this->flash.eraseSector(sector_start);
        log_flash("Erased sector at address " + String(sector_start));
        sector_start += SECTOR_SIZE;
      }
      log_flash("File " + String(fileNumber) + " deletion complete.");
      return;
    }
  }
  log_flash("File " + String(fileNumber) + " not found for deletion.");
}
