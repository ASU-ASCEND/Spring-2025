#include "FlashStorage.h"

/**
 * @brief Construct a new Flash Storage:: Flash Storage object
 *
 */
FlashStorage::FlashStorage() : position(0), Storage("Flash Storage") {}

/**
 * @brief Read position from flash
 *
 */
void FlashStorage::loadPosition() {
  uint8_t currentByte = this->flash.readByte(this->position);

  // Shuffle through data at last recorded position until free space is reached
  while ((currentByte != 0xFF) && (this->position < this->MAX_SIZE)) {
    currentByte = this->flash.readByte(++this->position);
  }
}

/**
 * @brief Verify flash memory connection and create a new file
 *
 * @return true
 * @return false
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

  Serial.println("Initial position: " + String(this->position));
  this->position = 0;
  this->loadPosition();  // Get position from flash

  Serial.println("Updated position: " + String(this->position));

  return true;
}

/**
 * @brief Store data on flash
 *
 * @param data
 */
void FlashStorage::store(String data) {
  data = data + "\n";

  for (const uint8_t& character : data) {
    this->flash.writeByte(this->position, character);
    ++this->position;
    this->flash.blockingBusyWait();
  }

  Serial.println("Writing " + String(data.length()) + " bytes at " +
                 String(this->position));

  this->flash.blockingBusyWait();
}

void FlashStorage::dump() {
  Serial.println("\nStarting data transfer: ");
  Serial.println("Position is at " + String(this->position));
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

void FlashStorage::erase() {
  this->flash.erase();
  this->position = 0;
}