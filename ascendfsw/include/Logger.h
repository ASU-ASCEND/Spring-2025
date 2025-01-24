#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>

static inline void log_core(String str){
  Serial.println("[Core " + String(get_core_num()) + "] " + str);
}

static inline void log_data_raw(uint8_t* packet, uint8_t len){
  Serial.write((const char *)packet, len); 
}

static inline void log_data_bytes(uint8_t* packet, uint8_t len){
  for(size_t i = 0; i < len; i++){
    Serial.print(packet[i], HEX);
    Serial.print(" "); 
  }
  Serial.println(); 
}

#endif // LOGGER_H