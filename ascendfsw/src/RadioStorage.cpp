#include "RadioStorage.h"

/**
 * @brief Construct a new RadioStorage object
 *
 */
RadioStorage::RadioStorage() : Storage("Radio") {}

/**
 * @brief Initialize UART1 (Serial1)
 *
 * @return true UART1 bus was succesfully initialized
 * @return false otherwise
 */
bool RadioStorage::verify() {
  Serial.end();
  Serial1.setRX(SERIAL1_RX_PIN);
  Serial1.setTX(SERIAL1_TX_PIN);
  Serial1.begin(57600);
  return Serial1;
}

/**
 * @brief Send data to the radio for transmitting
 *
 * @param data Data to transmit
 */
void RadioStorage::store(String data) {
  static const unsigned long transmission_mod = 1;
  static unsigned long transmission_count = 0;
  if (transmission_count % transmission_mod == 0) {
    Serial1.println(data);
  }
}
