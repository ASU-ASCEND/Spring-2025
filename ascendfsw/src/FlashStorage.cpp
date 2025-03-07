#include "FlashStorage.h"

/**
 * @brief Constructs a new FlashStorage object.
 *
 * Initializes the internal write position to 0 and sets the storage name to
 * "Flash Storage."
 */
FlashStorage::FlashStorage() : position(0), Storage("Flash Storage") {}

/**
 * @brief Locates the next available file block & tracks existing files
 * 
 * Reads 4KB blocks from flash starting at 0 and increments through memory
 * until finding a free (0xFF) location or reaching the max size. Furthermore, 
 * checking for file headers to record their locations as variables for quick
 * reference. 
 */
void FlashStorage::indexFlash() {
  uint8_t currentByte = this->flash.readByte(this->DATA_START_POSITION);

  // Shuffle through data by 4KB until free space is reached
  while ((currentByte != 0xFF) && (this->position < this->MAX_SIZE)) {
    currentByte = this->flash.readByte(this->position);
    position += 4'000; // 4 KB (shift to next block)
  }
}

/**
 * @brief Locates the next available write position in flash.
 *
 * Reads bytes from flash starting at the current position and increments
 * through memory until finding a free (0xFF) location or reaching the max size.
 */
void FlashStorage::loadPosition() {
  uint8_t currentByte = this->flash.readByte(this->position);

  // Shuffle through data at last recorded position until free space is reached
  while ((currentByte != 0xFF) && (this->position < this->MAX_SIZE)) {
    currentByte = this->flash.readByte(++this->position);
  }
}

/**
 * @brief Verifies the flash connection and prepares for data storage.
 *
 * Attempts to initialize the flash memory. If successful, resets the internal
 * write position to 0, locates the next free position, and returns true.
 * Returns false otherwise.
 *
 * @return true  - Flash initialization is successful
 * @return false - Flash initialization failed
 */
bool FlashStorage::verify() {
  // #if SD_SPI1
  // if (!SD.begin(SD_CS_PIN, this->sd_spi_1)) {
  //   ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
  //   return false;
  // }
  // #else
  //   if (!SD.begin(SD_CS_PIN)) {
  //     ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
  //     return false;
  //   }
  // #endif

#if FLASH_SPI1
  if (this->flash.begin(FLASH_CS_PIN, 2000000UL, this->flash_spi_1) == false)
    return false;
#else
  if (this->flash.begin(FLASH_CS_PIN) == false) return false;
#endif

  log_core("Initial position: " + String(this->position));
  this->position = 0;
  this->loadPosition();  // Get position from flash

  log_core("Updated position: " + String(this->position));

  return true;
}

/**
 * @brief Stores a string in flash memory, appending a newline at the end.
 *
 * Iterates through each character of the string and writes it to flash. The
 * internal write position is then advanced accordingly.
 *
 * @param data - The string to be stored in flash.
 */
void FlashStorage::store(String data) {
  data = data + "\n";

  for (const uint8_t& character : data) {
    this->flash.writeByte(this->position, character);
    ++this->position;
    this->flash.blockingBusyWait();
  }

  log_core("Writing " + String(data.length()) + " bytes at " +
           String(this->position));

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
void FlashStorage::store(uint8_t* packet) {
  // -- 4 bytes (sync) -- 4 bytes (presence) -- 2 bytes (length) -- data...
  uint16_t packet_len = *((uint16_t*)(packet + 4 + 4));
  uint16_t packet_header = 4 + 4 + 2;  // 10 bytes (sync, presence, length)

  // Iterate through packet and write to flash
  for (uint16_t i = 0; i < packet_len; i++) {
    this->flash.writeByte(this->position, packet[packet_header + i]);
    ++this->position;
    this->flash.blockingBusyWait();
  }

  // Log the number of bytes written
  log_core("Writing " + String(packet_len) + " bytes at " +
           String(this->position));
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
  log_core("Position is at " + String(this->position));
  char data = '^';
  uint32_t pos = this->DATA_START_POSITION;
  // read until it hits end_code
  while ((pos < this->MAX_SIZE) && (pos < this->position) && (data != 0xFF)) {
    digitalWrite(HEARTBEAT_PIN_0, (pos & 0x60) != 0);
    digitalWrite(HEARTBEAT_PIN_1, (pos & 0x60) != 0);
    data = this->flash.readByte(pos);
    pos++;
    Serial.write(data);  // print as a character
    // delay(1);
  }
}

/**
 * @brief Erases all data stored in flash memory.
 *
 * Erases all data stored in flash memory by calling the flash erase function
 * and resetting the internal write position to 0.
 */
void FlashStorage::erase() {
  this->flash.erase();
  this->position = 0;
}
