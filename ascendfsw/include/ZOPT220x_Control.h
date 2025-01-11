#ifndef ZOPT220_FUNC
#define ZOPT220_FUNC

#include <Arduino.h>
#include <Wire.h>
#include <stdint.h>

// #define byte uint8_t

// Register addresses
#define ZOPT220x_MAIN_CTRL 0x00
#define ZOPT220x_LS_MEAS_RATE 0x04
#define ZOPT220x_LS_GAIN 0x05
#define ZOPT220x_PART_ID 0x06
#define ZOPT220x_MAIN_STATUS 0x07
#define ZOPT220x_ALS_DATA 0x0D
#define ZOPT220x_UVB_DATA 0x10
#define ZOPT220x_UV_COMP_DATA 0x13
#define ZOPT220x_COMP_DATA 0x16
#define ZOPT220x_INT_CFG 0x19
#define ZOPT220x_INT_PST 0x1A
#define ZOPT220x_DEVICE_CONFIG 0x2F
#define ZOPT220x_SPECIAL_MODE_1 0x30
#define ZOPT220x_SPECIAL_MODE_2 0x31

#define SENSORTYPE_ZOPT2201 0xB2
#define SENSORTYPE_ZOPT2202 \
  0xB2  // Really IDT!? You didn't use a different part number for different
        // parts?

class ZOPT220x_Control {
 private:
  byte zoptAddress = 0x53;  // Default I2C address. Can be changed via software.
  // #define ZOPT220X_ADDR 0x53 //7-bit unshifted default I2C Address
  // #define ZOPT220X_ADDR 0x52 //7-bit unshifted programmed NVM I2C Address (in
  //  case you need multiple on same bus)
  //  boolean zopt220xSetup();
  //  boolean zopt220xSetupWithAddress(byte address);
  //  float getUVIndex(void);
  //  float getAdjustedUVIndex(long UVB, byte gain, byte resolution);
  //  long getUVB();
  //  long getALS();
  //  void setMeasurementRate(byte rate);
  //  void setResolution(byte resolution);
  //  void setGain(byte gain);
  //  boolean checkPowerOnStatus();
  //  boolean dataAvailable();
  //  void enableSensor(void);
  //  void disableSensor(void);
  //  void standby();
  //  void enableALSSensing();
  //  void enableUVBSensing();
  //  void enableRawSensing();
  //  void setMode(byte mode);
  //  void setI2CAdr53();
  //  void setI2CAdr52();
  //  void softReset();
  //  long zoptRead3Registers(byte addr);
  //  byte zoptReadRegister(byte addr);
  //  void zoptWriteRegister(byte addr, byte val);
  //  void setMode(byte mode);
 public:
  // Setup the sensor with default settings
  // Assume we are using the default zopt address of 0x53
  boolean zopt220xSetup() {
    boolean result =
        zopt220xSetupWithAddress(zoptAddress);  // Assume the default address
    return (result);
  }

  // Attempts to talk to sensor. Returns false if sensor is not found
  // If found, sets the sensor up with default UVB settings
  // Call this function if you've changed the zopt I2C address
  boolean zopt220xSetupWithAddress(byte address) {
    zoptAddress =
        address;  // Do all the zopt I2C communications at this address

    byte deviceID = zoptReadRegister(ZOPT220x_PART_ID);
    if (deviceID != 0xB2) {
      Serial.print("ID should be 0xB2. We heard 0x");
      Serial.println(deviceID, HEX);
      return (false);
    }

    checkPowerOnStatus();  // Read the POS bit to clear it

    setMeasurementRate(2);  // 100ms default
    setResolution(0);       // 20 bit, 400ms, needed for best UVI calculations
    setGain(4);             // Default for UVB

    return (true);
  }

  // Return the current UV index
  // Assumes UVB sensor has been setup with default settings
  float getUVIndex(void) {
    byte counter = 0;
    while (!dataAvailable()) {
      delay(10);
      if (counter++ > 50)  // We should have a reading in 400ms
      {
        // We've timed out waiting for the data. Return error
        return (-1.0);
      }
    }

    long uvb = getUVB();  // Get new data
    float uvIndex = getAdjustedUVIndex(
        uvb, 4, 0);  // Assumes default for UVI of gain:4, resolution:0
    return (uvIndex);
  }

  // Given UVB reading, and gain and resolution settings, outputs UV index
  // From App note UV Index Calculation from IDT
  float getAdjustedUVIndex(long UVB, byte gain, byte resolution) {
    byte gainMode = 0;  // gain is 0 to 4, gainMode is mapped 1 to 18
    byte resolutionMode =
        0;  // resolution is 0 to 5, resolutionMode is mapped 20bit to 13bit

    // Convert the 5 gain settings to gainMode
    switch (gain) {
      case 0:
        gainMode = 1;
        break;
      case 1:
        gainMode = 3;
        break;
      case 2:
        gainMode = 6;
        break;
      case 3:
        gainMode = 9;
        break;
      case 4:
        gainMode = 18;
        break;
      default:
        gainMode = 18;
        break;  // Unknown state
    }

    // Convert the 6 resolution settings to resolutionMode
    switch (resolution) {
      case 0:
        resolutionMode = 20;
        break;  // 20-bit
      case 1:
        resolutionMode = 19;
        break;
      case 2:
        resolutionMode = 18;
        break;
      case 3:
        resolutionMode = 17;
        break;
      case 4:
        resolutionMode = 16;
        break;
      case 5:
        resolutionMode = 13;
        break;  // 13-bit
      default:
        resolutionMode = 20;
        break;  // Unknown state
    }

    long uvAdjusted = 18 / gainMode * pow(2, 20 - resolutionMode) * UVB;

    // The correction value is default 1. If you want to calculate the actual
    // solar angle you'll need lat/long/day/hh/mm
    float uviCorrection = 1.0;

    // First we need to calculation the elevation angle
    /*float elevationAngle =

    //Let's calculate the correction!
    float c0 = 3.1543;
    float c1 = -0.06204;
    float c2 = 0.0002186;
    float c3 = 0.0000035516;
    uviCorrection = c0 * pow(elevation, 0);
    uviCorrection += c1 * pow(elevation, 1);
    uviCorrection += c2 * pow(elevation, 2);
    uviCorrection += c3 * pow(elevation, 3);*/

    float uvIndex = uvAdjusted / (5500.0 * uviCorrection);

    return (uvIndex);
  }

  // Returns the current UV data with temp compensation applied
  // Assumes LS data bit 3 has been set (MAIN_STATUS)
  // Poll dataAvailable() before calling this function
  long getUVB() { return (zoptRead3Registers(ZOPT220x_UVB_DATA)); }

  // Returns the current Ambient Light data with temp compensation applied
  // Assumes data is available (LS data bit 3 has been set in MAIN_STATUS)
  // Poll dataAvailable() before calling this function
  long getALS() { return (zoptRead3Registers(ZOPT220x_ALS_DATA)); }

  // Set the measurement rate
  // 0 = 25ms per measurement
  // 1 = 50ms
  // 2 = 100ms (default)
  // 3 = 200ms
  // 4 = 500ms
  // 5 = 1000ms
  // 6 = 2000ms
  // 7 = 2000ms
  void setMeasurementRate(byte rate) {
    if (rate > 7) rate = 7;  // Error check

    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_LS_MEAS_RATE);  // Read
    value &= ~(0b00000111);                                // Clear bits 2/1/0
    value |= rate;                                         // Mask in rate
    zoptWriteRegister(ZOPT220x_LS_MEAS_RATE, value);       // Write
  }

  // Set the measurement resolution
  // 0 = 20 bit / 400ms
  // 1 = 19 bit / 200ms
  // 2 = 18 bit / 100ms (default)
  // 3 = 17 bit / 50ms
  // 4 = 16 bit / 25ms
  // 5 = 13 bit / 3.125ms
  void setResolution(byte resolution) {
    if (resolution > 5) resolution = 5;  // Error check

    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_LS_MEAS_RATE);  // Read
    value &= ~(0b01110000);                                // Clear bits 6/5/4
    value |= (resolution << 4);                            // Mask in resolution
    zoptWriteRegister(ZOPT220x_LS_MEAS_RATE, value);       // Write
  }

  // Set the gain for the sensor
  // Different for ALS and UVB. It depends which mode you're in. See datasheet
  // for info. 0 = Gain 1 1 = Gain 3 (default) 2 = Gain 6 3 = Gain 9 4 = Gain 18
  // (recommended for UBV)
  void setGain(byte gain) {
    if (gain > 4) gain = 4;  // Error check

    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_LS_GAIN);  // Read
    value &= ~(0b00000111);                           // Clear bits 2/1/0
    value |= gain;                                    // Mask in gain
    zoptWriteRegister(ZOPT220x_LS_GAIN, value);       // Write
  }

  // Returns true if Bit 5 is set
  boolean checkPowerOnStatus() {
    byte value = zoptReadRegister(ZOPT220x_MAIN_STATUS);  // Read
    if (value & 1 << 5) return (true);
    return (false);
  }

  // Check if new data is available
  // Returns true if Bit 3 is set
  boolean dataAvailable() {
    byte value = zoptReadRegister(ZOPT220x_MAIN_STATUS);  // Read
    if (value & 1 << 3) return (true);
    return (false);
  }

  // Turns on sensor
  // Sets LS_EN bit
  void enableSensor(void) {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_MAIN_CTRL);  // Read
    value |= (1 << 1);                                  // Set LS_EN bit
    zoptWriteRegister(ZOPT220x_MAIN_CTRL, value);       // Write
  }

  // Turns off sensor
  // Clears LS_EN bit
  void disableSensor(void) {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_MAIN_CTRL);  // Read
    value &= ~(1 << 1);                                 // Clear LS_EN bit
    zoptWriteRegister(ZOPT220x_MAIN_CTRL, value);       // Write
  }

  void standby() {
    setMode(1);  // Turn off everything and standby for I2C
  }
  void enableALSSensing() {
    setMode(2);  // ALS + COMP channels activated
  }
  void enableUVBSensing() {
    setMode(3);  // UVB + UV_COMP channels activated
  }
  void enableRawSensing() {
    setMode(4);  // Special mode: ALS+UVB+UV_COMP+COMP channels activated
  }

  // Sets the mode of the sensor
  // Mode 1: Standby (power on default)
  // Mode 2: Ambient light sensing w/ compensation
  // Mode 3: UVB sensing w/ compensation
  // Mode 4: ALS/UVBS_Raw
  void setMode(byte mode) {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_MAIN_CTRL);  // Read

    if (mode == 0)
      value &= ~(1 << 1);  // Not a valid mode. Standby. Clear LS_EN bit
    else if (mode == 1)
      value &= ~(1 << 1);  // Standby. Clear LS_EN bit
    else if (mode == 2)    // ALS
    {
      value |= (1 << 1);   // Set LS_EN bit
      value &= ~(1 << 2);  // Clear RawMode_SEL bit
      value &= ~(1 << 3);  // Clear LS_Mode bit for ALS
    } else if (mode == 3)  // UVB
    {
      value |= (1 << 1);   // Set LS_EN bit
      value &= ~(1 << 2);  // Clear RawMode_SEL bit
      value |= (1 << 3);   // Set LS_Mode bit for UVB
    } else if (mode == 4)  // RawMode
    {
      // Send magic twiddly bytes
      zoptWriteRegister(ZOPT220x_SPECIAL_MODE_2,
                        0xB5);  // Write first special byte
      zoptWriteRegister(ZOPT220x_SPECIAL_MODE_1,
                        0xDF);  // Write second special byte
      zoptWriteRegister(ZOPT220x_SPECIAL_MODE_2,
                        0x04);  // Write third special byte

      value |= (1 << 1);   // Set LS_EN bit
      value |= (1 << 2);   // Set RawMode_SEL bit
      value &= ~(1 << 3);  // Clear LS_Mode bit
    }

    zoptWriteRegister(ZOPT220x_MAIN_CTRL, value);  // Write
  }

  // Sets the I2C address to 0x53 (default)
  void setI2CAdr53() {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_DEVICE_CONFIG);  // Read
    value &= ~(1 << 1);  // Clear I2C address bit = 0x53
    zoptWriteRegister(ZOPT220x_DEVICE_CONFIG, value);  // Write
  }

  // Sets the I2C address to 0x52
  void setI2CAdr52() {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_DEVICE_CONFIG);  // Read
    value |= (1 << 1);  // Set I2C address bit = 0x52
    zoptWriteRegister(ZOPT220x_DEVICE_CONFIG, value);  // Write
  }

  // Does a soft reset
  // Give sensor at least 1000ms to reset
  void softReset() {
    // Read, set, write
    byte value = zoptReadRegister(ZOPT220x_MAIN_CTRL);  // Read
    value |= (1 << 4);                                  // Set RST bit
    zoptWriteRegister(ZOPT220x_MAIN_CTRL, value);       // Write
  }

  // The ALS and UVB data come in three byte pack: low/mid/high
  // Read all three and store in long
  // Returns -1 if times out
  // Max size is 20 bits so signed long is ok
  long zoptRead3Registers(byte addr) {
    Wire.beginTransmission(zoptAddress);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(zoptAddress, 3);

    // Non-blocking wait for 3 bytes
    byte counter = 0;
    while (Wire.available() < 3) {
      delay(1);
      if (counter++ > 25) return (-1);  // Error
    }

    byte lowValue = Wire.read();
    byte midValue = Wire.read();
    byte highValue = Wire.read();

    long value = (long)highValue << 16 | (long)midValue << 8 | lowValue;

    return (value);
  }

  // Reads from a give location from the ZOPT220x
  byte zoptReadRegister(byte addr) {
    Wire.beginTransmission(zoptAddress);
    Wire.write(addr);
    Wire.endTransmission();

    Wire.requestFrom(zoptAddress, 1);
    if (Wire.available()) return (Wire.read());

    Serial.println("I2C Error");
    return (0xFF);  // Error
  }

  // Write a value to a spot in the ZOPT220x
  void zoptWriteRegister(byte addr, byte val) {
    Wire.beginTransmission(zoptAddress);
    Wire.write(addr);
    Wire.write(val);
    Wire.endTransmission();
  }
};

#endif