#ifndef STORAGE_H
#define STORAGE_H

#include <Arduino.h>

/**
 * @brief Parent class for all data storage devices (sd card, radio, etc)
 *
 */
class Storage {
 private:
  String storage_name;

 public:
  Storage(String storage_name) { this->storage_name = storage_name; }

  /**
   * @brief Get the name of the storage device
   *
   * @return const String&
   */
  const String& getStorageName() const { return this->storage_name; }

  /**
   * @brief Verifies connection with storage device
   *
   * @return true
   * @return false
   */
  virtual bool verify() = 0;

  /**
   * @brief Send data to storage device
   *
   * @param data Data to store
   */
  virtual void store(String data) = 0;
};

#endif