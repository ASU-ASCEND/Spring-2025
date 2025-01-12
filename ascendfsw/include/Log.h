#ifndef LOG_H
#define LOG_H

#include <Arduino.h>

static inline void log(String str){
  Serial.println("[Core " + String(get_core_num()) + "] " + str);
}

#endif // LOG_H