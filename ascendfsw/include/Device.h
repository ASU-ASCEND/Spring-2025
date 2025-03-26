#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>

#include "Logger.h"

#define MINUTE_IN_MILLIS (1000 * 60)

/**
 * @brief Implements the recovery system for Sensor and Storage objects
 *
 */
class Device {
 private:
  unsigned long last_attempt;
  // set max_attempts to -1 to have remove attempt limit
  int max_attempts;
  int attempt_number;
  unsigned long wait_factor;

 protected:
  // current verification of the sensor, can be set by children to trigger a
  // verify
  bool verified;
  String device_name;

 public:
  /**
   * @brief Default constructor, sets a max_attempt of 1 (device recovery won't
   * be attempted)
   *
   */
  Device(String device_name) {
    this->verified = false;
    this->last_attempt = -1;
    this->max_attempts = 1;
    this->attempt_number = 0;
    this->wait_factor = 1 * MINUTE_IN_MILLIS;
    this->device_name = device_name;
  }

  /**
   * @brief
   *
   * @param max_attempts Maximum attempts before no longer trying to recover the
   * sensor
   * @param wait_factor Amount to increase wait time between each attempt by for
   * each failed attempt
   */
  Device(String device_name, int max_attempts, int wait_factor)
      : Device(device_name) {
    this->max_attempts = max_attempts;
    this->wait_factor = wait_factor;
  }

  const String& getDeviceName() { return this->device_name; }

  /**
   * @brief Verifies if the Device is connected and working
   *
   * @return true, if connected
   * @return false, if not connected
   */
  virtual bool verify() = 0;

  /**
   * @brief Get if the Device is Verified
   *
   * @return true
   * @return false
   */
  bool getVerified() { return this->verified; }

  /**
   * @brief Set recovery config (used keep default constructor)
   *
   * @param max_attempts Maximum attempts before no longer trying to recover the
   * sensor
   * @param wait_factor Amount to increase wait time between each attempt by for
   * each failed attempt
   */
  void recoveryConfig(int max_attempts, int wait_factor) {
    this->setMaxAttempts(max_attempts);
    this->setWaitFactor(wait_factor);
  }

  /**
   * @brief Set wait_factor
   *
   * @param wait_factor Amount to increase wait time between each attempt by for
   * each failed attempt
   */
  void setWaitFactor(int wait_factor) { this->wait_factor = wait_factor; }

  /**
   * @brief Set max_attempts
   *
   * @param max_attempts Maximum attempts before no longer trying to recover the
   * sensor
   */
  void setMaxAttempts(int max_attempts) { this->max_attempts = max_attempts; }

  /**
   * @brief If the sensor is verified, return true, if not and it has been long
   * enough since the last attempt (decided by wait_factor and attempt_number),
   * attempt to reverify (reinitialize) it.
   *
   *
   * @return true If verified
   * @return false If unverified
   */
  bool attemptConnection() {
    // if it isn't verified and is time to try again
    // time between tests scales with attempt_number to spread out attempts
    if (this->verified == false &&
        (this->max_attempts == -1 ||
         this->attempt_number < this->max_attempts) &&
        ((millis() - this->last_attempt) >
         (this->wait_factor * this->attempt_number))) {
      log_core("Attempt on " + this->device_name);
      // try to verify again
      this->verified = this->verify();
      // update record
      this->last_attempt = millis();
      if (this->verified) {
        this->attempt_number = 0;
      } else {
        this->attempt_number++;
      }
    }
    // return result
    return this->verified;
  }
};

#endif  // DEVICE_H