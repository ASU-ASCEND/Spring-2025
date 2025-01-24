#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

static inline void log_core(String str) {
  Serial.print("[Core " + String(get_core_num()) + "] " + str + "\n");
}

static inline void log_data_raw(uint8_t* packet, uint8_t len) {
  Serial.write((const char*)packet, len);
}

static inline void log_data_bytes(uint8_t* packet, uint8_t len) {
  String packet_as_hex = ""; 
  for (size_t i = 0; i < len; i++) {
    packet_as_hex += String(packet[i], HEX) + " ";
  }
  packet_as_hex += "\n";
  Serial.print(packet_as_hex);
}

static inline void log_data(String data) { Serial.print("[Data] " + data + "\n"); }

#endif  // LOGGER_H