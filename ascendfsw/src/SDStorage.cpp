#include "SDStorage.h"

/**
 * @brief Construct a new SDStorage object
 *
 */
SDStorage::SDStorage() : Storage("SD Card") {}

/**
 * @brief Verify SD card connection and create a new, unique file
 *
 * @return true if SD card is connected and file is successfully created
 * @return false otherwise
 */
bool SDStorage::verify() {
// initialize SD card w/ instance
// setup SPI1
#if SD_SPI1
  if (!SD.begin(SD_CS_PIN, this->sd_spi_1)) {
    ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
    return false;
  }
#else
  if (!SD.begin(SD_CS_PIN)) {
    ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
    return false;
  }
#endif

  // find unused file name
  int num = 0;
  while (SD.exists("DATA" + String(num) + ".CSV")) num++;
  if (num != 0) ErrorDisplay::instance().addCode(Error::POWER_CYCLED);
  this->file_name = "DATA" + String(num) + ".CSV";

  // create file
  File f = SD.open(this->file_name, FILE_WRITE);
  if (!f) return false;  // check to see if the open operation worked
  f.close();

  return true;
}

/**
 * @brief Store data on the SD card, ending with newline
 *
 * @param data Data to store
 */
void SDStorage::store(String data) {
  File output = SD.open(this->file_name, FILE_WRITE);
  if (!output) {
    log_core("SD card write failed");
    // set error if fails at all, but might still be working
    ErrorDisplay::instance().addCode(Error::SD_CARD_FAIL);
    SD.end();  // close instance

    if (this->verify()) {  // try to reconnect
      log_core("Reverify succeeded");
    } else {
      log_core("Reverify failed");
    }
  }

  output.println(data);

  output.close();
}