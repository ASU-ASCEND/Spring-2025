#ifndef ERROR_DISPLAY_H
#define ERROR_DISPLAY_H

#include <Arduino.h>

#include "PayloadConfig.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "TLC59108.h"
#include <Wire.h>

/**
 * Error codes:
 * Smallest will be displayed as the binary value 7 - (enum value)
 * ex. CRITICAL_FAIL (enum value of 0) will be all 3 leds blinking (7 - 0 = 7 or
 * 0b111) Max 7 errors (1 led should always be blinking) NONE will always be 1
 */
typedef enum {
  CRITICAL_FAIL = 0,  // no sensors or no storage
  SD_CARD_FAIL,  // triggered if SD card verify function returns false or if an
                 // SD card write fails
  LOW_SENSOR_COUNT,  // triggered for less than 5 sensors verified
  POWER_CYCLED,  // determined based on if there are multiple data files on the
                 // SD card
  NONE           // default state, lowest priority
} Error;

// LED driver: https://www.ti.com/product/TLC59108
#define TLC_I2C_ADDR 0x40


/**
 * @brief Singleton class for the 3 GPIO LED Error Display
 *
 */
class ErrorDisplay {
 private:
  mutex_t error_display_mutex;
  int pin_level;
  Error code;
  TLC59108 leds = TLC59108(Wire, TLC_I2C_ADDR); 

  ErrorDisplay() {
    mutex_init(&error_display_mutex);
    this->pin_level = 1;
    this->code = NONE;
    #if USE_LED_DRIVER
    Wire.begin(); 
    this->leds.init(); 
    this->leds.setLedOutputMode(TLC59108::LED_MODE::PWM_IND); 
    this->leds.setAllBrightness((uint8_t)0); 
    #else 
    pinMode(ERROR_PIN_2, OUTPUT);
    pinMode(ERROR_PIN_1, OUTPUT);
    pinMode(ERROR_PIN_0, OUTPUT);
    #endif 
  }

 public:
  /**
   * @brief Accesses the only instance of ErrorDisplay (Singleton)
   *
   * @return ErrorDisplay& Instance of ErrorDisplay
   */
  static ErrorDisplay& instance() {
    static ErrorDisplay only_instance;
    return only_instance;
  }

  /**
   * @brief Sets the error code to the given value if the given error is of a
   * higher priority then the current error
   *
   * @param e The error code to display
   */
  void addCode(Error e) {
    mutex_enter_blocking(&error_display_mutex);

    if (e < this->code) {
      this->code = e;
    }

    mutex_exit(&error_display_mutex);
  }

  /**
   * @brief Toggles the level of the error display
   *
   */
  void toggle() {
    mutex_enter_blocking(&error_display_mutex);

    this->pin_level = !(this->pin_level);

    uint8_t display_code = 7 - this->code;  // 0 is highest

    if (this->code == Error::NONE) display_code = 0b001;

    #if USE_LED_DRIVER
    leds.setBrightness(LED_2_CHANNEL, (this->pin_level && (display_code & 0b100)) * 0xFF);
    leds.setBrightness(LED_1_CHANNEL, (this->pin_level && (display_code & 0b010)) * 0xFF);
    leds.setBrightness(LED_0_CHANNEL, (this->pin_level && (display_code & 0b001)) * 0xFF);
    #else
    digitalWrite(ERROR_PIN_2, this->pin_level && (display_code & 0b100));
    digitalWrite(ERROR_PIN_1, this->pin_level && (display_code & 0b010));
    digitalWrite(ERROR_PIN_0, this->pin_level && (display_code & 0b001));
    #endif 

    mutex_exit(&error_display_mutex);
  }
};

#endif