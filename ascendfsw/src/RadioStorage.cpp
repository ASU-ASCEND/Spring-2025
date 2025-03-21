#include "RadioStorage.h"

/**
 * @brief Construct a new RadioStorage object
 *
 */
RadioStorage::RadioStorage() : Storage("Radio") {}

/**
 * @brief Initialize UART1 (Serial1)
 *
 * @return true UART1 bus was successfully initialized
 * @return false otherwise
 */
bool RadioStorage::verify() {
  Serial1.end();
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
  transmission_count++; 
}

/**
 * @brief Send data to the radio for transmitting
 *
 * @param packet Packet to send
 */
void RadioStorage::storePacket(uint8_t* packet) {
  static const unsigned long transmission_mod = 1;
  static unsigned long transmission_count = 0;
  // length of packet will be after sync bytes (4) and sensor presence (4)
  // it is uint16_t
  uint16_t packet_len;
  memcpy(&packet_len, (packet + 8), sizeof(uint16_t));

  // write to packet 
  if (transmission_count % transmission_mod == 0) {
    Serial1.write(packet, packet_len);
  }
  transmission_count++; 
}