#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h> 

#define MINUTE_IN_MILLIS (1000 * 60)

class Device {

private:
  bool verified; 
  unsigned long last_attempt; 
  // set max_attempts to -1 to have remove attempt limit 
  int max_attempts; 
  int attempt_number; 
  unsigned long wait_factor; 

public:
  int test; 

  Device() {
    this->verified = false; 
    this->last_attempt = -1; 
    this->max_attempts = 1; 
    this->attempt_number = 0; 
    this->wait_factor = 1 * MINUTE_IN_MILLIS; 
  }

  Device(int max_attempts, int wait_factor) {
    this->max_attempts = max_attempts; 
    this->wait_factor = wait_factor;
  }

  virtual bool verify() = 0; 

  bool getVerified(){
    return this->verified; 
  }

  void recoveryConfig(int max_attempts, int wait_factor){
    this->setMaxAttempts(max_attempts); 
    this->setWaitFactor(wait_factor); 
  }

  void setWaitFactor(int wait_factor){
    if(wait_factor > 1) this->wait_factor = wait_factor;  
  }

  void setMaxAttempts(int max_attempts){
    if(max_attempts > 1) this->max_attempts = max_attempts; 
  }

  bool attemptConnection(){
    // if it isn't verified and is time to try again
    // time between tests scales with attempt_number to spread out attempts
    if(this->verified == false && 
        (this->max_attempts == -1 || this->attempt_number < this->max_attempts) && 
        (millis() - this->last_attempt > (this->wait_factor*this->attempt_number))){
      // try to verify again 
      this->verified = this->verify(); 
      // update record
      this->last_attempt = millis(); 
      this->attempt_number++; 
    }
    // return result 
    return this->verified; 
  }

};

#endif // DEVICE_H