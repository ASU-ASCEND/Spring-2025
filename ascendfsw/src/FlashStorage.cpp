#include "FlashStorage.h"

#include <string.h>
/**
 * @brief Constructs a new FlashStorage object.
 *
 * Initializes the internal write address to 0 and sets the storage name to
 * "Flash Storage."
 */
FlashStorage::FlashStorage() : address(0), Storage("Flash Storage") {}

/**
 * @brief Locates the next available file block & tracks existing files
 *
 * Reads 4KB blocks from flash starting at 0 and increments through memory
 * until finding a free (0xFF) location or reaching the max size. Furthermore,
 * checking for file headers to record their locations as variables for quick
 * reference.
 */
void FlashStorage::indexFlash() {
  bool empty = this->isSectorEmpty();
  // Shuffle through data by 4KB until free space is reached
  while (!empty && (this->address < this->MAX_SIZE)) {
    // Check for file at sector start
    if (!empty && this->readFileHeader()) {
      // Update end address of previous file
      if (!this->file_data.empty()) {
        this->file_data.back().end_address = this->address - 4;
        log_core("File " + String(this->file_data.back().file_number) +
                  " Size: " + String(this->file_data.back().start_address) +
                  " to " + String(this->file_data.back().end_address));
      }

      // Store new detected file
      this->file_data.push_back(
          {(this->file_data.size() + 1), this->address, this->address});
      log_core("File " + String(this->file_data.size()) + " at address " +
                String(this->address));
    }

    // Iterate the address
    this->address += this->SECTOR_SIZE;  // 4 KB (shift to next block)

    // Check if the next sector is empty
    empty = this->isSectorEmpty();
  }

  // Set end_address for the last file if any
  if (!this->file_data.empty()) {
    this->file_data.back().end_address = this->address;
  }

  // Write a file header at the beginning of the sector
  this->writeFileHeader();
  this->active_file =
      true;  // Prevent new file from being created unless power is disconnected
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
 * @brief Writes the file header to the start of a new sector.
 *
 * Write a file header (0xDEADBEEF) to indicate the start of a new file at the
 * beginning of a sector. Then, store the file number, along with the start
 * and end address in a data structure.
 *
 */
void FlashStorage::writeFileHeader() {
  uint8_t num_bytes = sizeof(this->FILE_HEADER) * 8;

  // Write 0xDEADBEEF to the start of the sector
  while (num_bytes != 0) {
    num_bytes -= 8;
    this->flash.writeByte(this->address++,
                          (this->FILE_HEADER >> num_bytes) & 0xFF);
    this->flash.blockingBusyWait();
  }

  // Update end address of previous file
  if (!this->file_data.empty()) {
    this->file_data.back().end_address = this->address - 4;
    log_core("File " + String(this->file_data.back().file_number) +
              " Size: " + String(this->file_data.back().start_address) +
              " to " + String(this->file_data.back().end_address));
  }

  // Store necessary file data for quick reference
  this->file_data.push_back(
      {(this->file_data.size() + 1), (this->address - 4), (this->address - 4)});
  log_core("New file " + String(this->file_data.size()) + " at address " +
            String(this->address - 4) + " created");
}

/**
 * @brief Determines if the first 4 bytes of a sector are a file header.
 *
 * @return true  - File header is present
 * @return false - File header is not present
 */
bool FlashStorage::readFileHeader() {
  uint8_t num_bytes = sizeof(this->FILE_HEADER) * 8;

  // Read 0xDEADBEEF from the start of the sector
  for (int i = 0; i < sizeof(FILE_HEADER); ++i) {
    num_bytes -= 8;
    if (this->flash.readByte(this->address + i) !=
        ((this->FILE_HEADER >> num_bytes) & 0xFF)) {
      return false;
    }
  }
  return true;
}

/**
 * @brief Verifies the flash connection and prepares for data storage.
 *
 * Attempts to initialize the flash memory. If successful, resets the internal
 * write address to 0, locates the next free address, and returns true.
 * Returns false otherwise.
 *
 * @return true  - Flash initialization is successful
 * @return false - Flash initialization failed
 */
bool FlashStorage::verify() {
#if FLASH_SPI1
  if (this->flash.begin(FLASH_CS_PIN, 2000000UL, this->flash_spi_1) == false)
    return false;
#else
  if (this->flash.begin(FLASH_CS_PIN) == false) return false;
#endif

  // Check if flash is full
  if (this->address >= this->MAX_SIZE) {
    log_core("Flash memory is full.");
    return false;
  }

  // Check, update, and log current flash storage status
  if (!active_file) {  // Activate if no file is currently being written to
    this->address = this->START_ADDRESS;
    log_core("Initial flash address: " + String(this->address));

    this->indexFlash();  // Get address from flash and track files

    log_core("Updated address: " + String(this->address) + " in sector " +
              String(this->address / this->SECTOR_SIZE));
  } else {  // Provide status information if file is currently active
    log_core("Flash storage is active, writing to File " +
              String(this->file_data.back().file_number) + " at address " +
              String(this->address) + " in sector " +
              String(this->address / this->SECTOR_SIZE));
  }

  // Log flash size
  log_core("Remaining space: " + String(this->MAX_SIZE - this->address) +
            " bytes");

  return true; 
}

bool FlashStorage::reinitFlash(){
  // Check if flash is full
  if (this->address >= this->MAX_SIZE) {
    log_core("Flash memory is full.");
    return false;
  }

  // Check, update, and log current flash storage status
  this->address = this->START_ADDRESS;
  log_core("Initial flash address: " + String(this->address));

  this->file_data.clear(); // wipe previous indexing 

  this->indexFlash();  // Get address from flash and track files

  log_core("Updated address: " + String(this->address) + " in sector " +
            String(this->address / this->SECTOR_SIZE));

  // Log flash size
  log_core("Remaining space: " + String(this->MAX_SIZE - this->address) +
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
  log_core("File " + String(this->file_data.back().file_number) +
            " Size: " + String(this->file_data.back().start_address) + " to " +
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
    this->flash.writeByte(this->address++, packet[i]);
    this->flash.blockingBusyWait();
  }

  // Update file length tracking
  this->file_data.back().end_address += packet_len;

  // Log the number of bytes written
  log_core("Writing " + String(packet_len) + " bytes at " +
            String(this->address));
  log_core("File " + String(this->file_data.back().file_number) +
            " Size: " + String(this->file_data.back().start_address) + " to " +
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
  // this->flash.erase();
  for(unsigned long sector = 0; sector < this->MAX_SIZE; sector += SECTOR_SIZE){
    this->flash.eraseSector(sector); 
    this->flash.blockingBusyWait(); 
    digitalWrite(HEARTBEAT_PIN_1, (sector & 0x20000) != 0);
  }
  this->address = 0;
}

/**
 * @brief Downloads data from flash memory with progress tracking
 *
 * This function provides a more user-friendly way to download data from flash
 * memory. It includes:
 * - Progress tracking
 * - File-by-file transfer
 * - Visual feedback
 * - Transfer statistics
 */
void FlashStorage::downloadFile(int file_number) {
  log_core("==== DOWNLOAD File " + String(file_number) + " ====");

  // Check if file number is valid & proceed with download
  if ((file_number > 0) && (file_number <= this->file_data.size())) {
    int index = file_number - 1;
    FileHeader selected_file = this->file_data[index];

    // Calculate file size based on addresses
    uint32_t start_addr = selected_file.start_address;
    uint32_t end_addr = selected_file.end_address;
    uint32_t file_size = end_addr - start_addr;

    // Send ACK with file info
    log_core("Downloading file " + String(file_number));
    log_core("File size: " + String(file_size) + " bytes");

    // Track the download time
    unsigned long start_time = millis();

    // Download Occurs Here
    uint32_t current_pos = start_addr;
    uint32_t bytes_read = 0;
    log_flash("START_DATA");
    while (current_pos < end_addr) {
      // Read data from flash
      uint8_t data = this->flash.readByte(current_pos);

      // Send data to Serial
      if (Serial.write(data) != 1) {
        log_core("ERROR: Unsuccessful serial write");
        return;
      }

      // Update progress
      bytes_read++;
      current_pos++;

      // Show progress every 1000 bytes
      // if (bytes_read % 1000 == 0) {
      //   log_core("Progress: " + String(bytes_read) + "/" + String(file_size));
      // }

      // Visual feedback
      digitalWrite(HEARTBEAT_PIN_1, (current_pos & 0x60) != 0);
    }
    log_flash("STOP_DATA");

    unsigned long end_time = millis();
    unsigned long total_time = (end_time - start_time) / 1000;
    log_core("Download Complete | Time: " + String(total_time) + "s");
  } else {
    log_core("ERROR: Invalid File Number");
    log_flash("START_DATA");
    log_flash("ERROR"); 
    log_flash("STOP_Data"); 
  }
}

/**
 * @brief Prints the current status of the flash storage.
 *
 * Prints the current address, remaining storage, and stored files.
 */
void FlashStorage::getStatus() {
  log_core("==== STATUS ====");

  log_core("Address: " + String(this->address));
  log_core("Remaining Storage: " + String(this->MAX_SIZE - this->address) +
           " bytes");

  log_core("Stored Files:");

  log_flash("START_DATA");
  for (const FileHeader& file : this->file_data) {
    int file_size = file.end_address - file.start_address;

    log_flash("File " + String(file.file_number) +
              " || Size: " + String(file_size) + " bytes");
  }
  log_flash("STOP_DATA");
}

/**
 * @brief Writes a string to flash atomically.
 *
 * This function creates a new file entry by first writing a header; then it
 * writes a marker (0xAA) indicating that the file is in progress and reserves
 * one byte for a checksum. It then writes the given data (with a trailing
 * newline) while computing a simple checksum. Once data writing completes, it
 * updates the checksum and changes the marker to 0xBB (complete).
 */
void FlashStorage::atomicStore(String data) {
  // Create a new file via the existing header write function.
  writeFileHeader();

  // Reserve 2 bytes after the file header for marker & checksum.
  // marker: 0xAA (writing state), then checksum: 0x00 as a placeholder.
  uint32_t markerAddress = this->address;
  this->flash.writeByte(markerAddress, 0xAA);  // 0xAA = writing/in-progress
  this->flash.blockingBusyWait();
  this->address++;  // advance after marker

  uint32_t checksumAddress = this->address;
  this->flash.writeByte(checksumAddress, 0x00);  // placeholder checksum
  this->flash.blockingBusyWait();
  this->address++;  // advance after checksum

  // Prepare data (append newline as in store())
  String dataToStore = data + "\n";
  uint8_t checksum = 0;

  // Write out each character and update checksum.
  for (const uint8_t& character : dataToStore) {
    this->flash.writeByte(this->address, character);
    this->flash.blockingBusyWait();
    checksum += character;  // simple checksum: sum of bytes
    this->address++;
  }

  // Update the file_data record for the file just written.
  if (!this->file_data.empty()) {
    this->file_data.back().end_address = this->address;
  }

  // Write the computed checksum.
  this->flash.writeByte(checksumAddress, checksum);
  this->flash.blockingBusyWait();

  // Mark the file entry as complete.
  this->flash.writeByte(markerAddress, 0xBB);  // 0xBB = complete
  this->flash.blockingBusyWait();

  log_core("AtomicStore complete. Data written with checksum: " +
            String(checksum));
}

/**
 * @brief Deletes a file by erasing the sectors it occupies.
 *
 * This function locates the specified file in the file_data vector. It then
 * calculates the range of sectors affected (using the sector size) and erases
 * them. For every sector erased, a log is produced.
 *
 * @param file_number The number identifying the file to delete.
 */
void FlashStorage::removeFile(uint32_t file_number) {
  bool found = false;
  FileHeader target;
  // Search for the file by number and remove it from file_data
  for (auto it = this->file_data.begin(); it != this->file_data.end(); ++it) {
    if (it->file_number == file_number) {
      target = *it;
      this->file_data.erase(it);
      found = true;
      break;
    }
  }

  if (!found) {
    log_core("ERROR: File number " + String(file_number) +
              " not found for deletion.");
    return;
  }

  // Calculate the sector range to be erased.
  uint32_t startSector = (target.start_address / SECTOR_SIZE) * SECTOR_SIZE;
  // Ensure endSector covers the last byte of the file.
  uint32_t endSector = ((target.end_address - 1) / SECTOR_SIZE) * SECTOR_SIZE;

  for (uint32_t sector = startSector; sector <= endSector;
       sector += SECTOR_SIZE) {
    // Erase sector by writing 0xFF to each byte in the sector
    // for (uint32_t addr = sector; addr < sector + this->SECTOR_SIZE; addr++) {
    //   this->flash.writeByte(addr, 0xFF);
    //   this->flash.blockingBusyWait();
    // }
    this->flash.eraseSector(sector); 
    this->flash.blockingBusyWait(); 
    log_core("Erased sector at address: " + String(sector));
    digitalWrite(HEARTBEAT_PIN_1, (sector & 0x20000) != 0);
  }

  this->reinitFlash(); 

  log_core("File " + String(file_number) + " removed successfully.");
}

/**
 * @brief Checks available flash space and logs warnings when thresholds are
 * reached.
 *
 * Thresholds: 64KB, 16KB, 4KB, and 0 bytes.
 */
void FlashStorage::checkFreeSpaceWarnings() {
  uint32_t remaining =
      (this->MAX_SIZE > this->address) ? (this->MAX_SIZE - this->address) : 0;

  if (remaining == 0) {
    log_core("WARNING: Flash memory is full (0 bytes remaining)!");
  } else if (remaining <= 4096) {  // 4 KB threshold
    log_core("WARNING: Only 4KB remaining!");
  } else if (remaining <= 16384) {  // 16 KB threshold
    log_core("WARNING: Only 16KB remaining!");
  } else if (remaining <= 65536) {  // 64 KB threshold
    log_core("WARNING: Only 64KB remaining!");
  }
}
