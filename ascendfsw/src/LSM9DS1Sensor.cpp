#include "LSM9DS1Sensor.h"

/**
 * @brief Construct a new LSM9DS1Sensor object with default minimum_period of 0
 *
 */
LSM9DS1Sensor::LSM9DS1Sensor() : LSM9DS1Sensor(0) {}

/**
 * @brief Construct a new LSM9DS1Sensor object, given mininum_period between
 * sensor reads
 *
 * @param minimum_period Minimum time to wait between readings in ms
 */
LSM9DS1Sensor::LSM9DS1Sensor(unsigned long minimum_period)
    : Sensor("LSM9DS1",
             "LSM9DS1 AccX,LSM9DS1 AccY,LSM9DS1 AccZ,LSM9DS1 GyroX,LSM9DS1 "
             "GyroY,LSM9DS1 GyroZ,LSM9DS1 MagX,LSM9DS1 MagY,LSM9DS1 MagZ,",
             9, minimum_period),
      lsm(LSM9DS1_XGCS_PIN,
          LSM9DS1_MCS_PIN) {  // Initialize lsm with CS pins for SPI
  // calibration offsets to zero
  for (int i = 0; i < 3; i++) {
    accel_offsets[i] = 0.0;
    gyro_offsets[i] = 0.0;
    mag_offsets[i] = 0.0;
  }
}

/**
 * @brief Verifies that the LSM is connected and working
 *
 * @return true if connected and working
 * @return false if not connected and working
 */
bool LSM9DS1Sensor::verify() {
  if (!lsm.begin()) {
    Serial.println("Unable to initialize the LSM9DS1. Check wiring.");
    return false;
  }
  setupSensor();  // sensor ranges and data rates
  return true;
}

/**
 * @brief Helper function to set up sensor ranges and data rates
 */
void LSM9DS1Sensor::setupSensor() {
  // Accelerometer range to 16G and data rate to 952Hz
  lsm.setupAccel(lsm.LSM9DS1_ACCELRANGE_16G, lsm.LSM9DS1_ACCELDATARATE_952HZ);

  // Magnetometer sensitivity to 4 Gauss
  lsm.setupMag(lsm.LSM9DS1_MAGGAIN_4GAUSS);

  // Gyroscope scale to 245 degrees per second
  lsm.setupGyro(lsm.LSM9DS1_GYROSCALE_245DPS);
}

/**
 * @brief Retrieves data from LSM 9-axis IMU
 *
 * @return String A CSV section in format AccX, AccY, AccZ, GyroX, GyroY, GyroZ,
 * MagX, MagY, MagZ,
 */
String LSM9DS1Sensor::readData() {
  lsm.read();

  sensors_event_t aevent, mevent, gevent, temp_event;
  lsm.getEvent(&aevent, &mevent, &gevent, &temp_event);

  // calibration offsets to accelerometer data
  float accX = aevent.acceleration.x - accel_offsets[0];
  float accY = aevent.acceleration.y - accel_offsets[1];
  float accZ = aevent.acceleration.z - accel_offsets[2];

  // calibration offsets to gyroscope data
  float gyroX = gevent.gyro.x - gyro_offsets[0];
  float gyroY = gevent.gyro.y - gyro_offsets[1];
  float gyroZ = gevent.gyro.z - gyro_offsets[2];

  // calibration offsets to magnetometer data
  float magX = mevent.magnetic.x - mag_offsets[0];
  float magY = mevent.magnetic.y - mag_offsets[1];
  float magZ = mevent.magnetic.z - mag_offsets[2];

  return String(accX) + "," + String(accY) + "," + String(accZ) + "," +
         String(gyroX) + "," + String(gyroY) + "," + String(gyroZ) + "," +
         String(magX) + "," + String(magY) + "," + String(magZ) + ",";
}

/**
 * @brief Calibrates the LSM9DS1 sensor by collecting offset data for gyro and
 * accel
 */
void LSM9DS1Sensor::calibrate() {
  Serial.println("Calibrating sensor... Please keep the sensor stationary.");
  delay(3000);  // time to place the sensor still

  // Variables to accumulate readings for calibration
  const int num_samples = 100;
  float acc_offset[3] = {0, 0, 0};
  float gyro_offset[3] = {0, 0, 0};
  float mag_offset[3] = {0, 0, 0};

  for (int i = 0; i < num_samples; i++) {
    lsm.read();
    sensors_event_t aevent, mevent, gevent, temp_event;
    lsm.getEvent(&aevent, &mevent, &gevent, &temp_event);

    acc_offset[0] += aevent.acceleration.x;
    acc_offset[1] += aevent.acceleration.y;
    acc_offset[2] += aevent.acceleration.z;

    gyro_offset[0] += gevent.gyro.x;
    gyro_offset[1] += gevent.gyro.y;
    gyro_offset[2] += gevent.gyro.z;

    mag_offset[0] += mevent.magnetic.x;
    mag_offset[1] += mevent.magnetic.y;
    mag_offset[2] += mevent.magnetic.z;

    delay(20);
  }

  // Average offsets
  for (int i = 0; i < 3; i++) {
    accel_offsets[i] = acc_offset[i] / num_samples;
    gyro_offsets[i] = gyro_offset[i] / num_samples;
    mag_offsets[i] = mag_offset[i] / num_samples;
  }

  Serial.println("Calibration complete.");
}

/**
 * @brief Appends the LSM9DS1 sensor data to the packet buffer as raw bytes.
 *
 * The following data are appended in order:
 *   - Date: day (uint8_t), month (uint8_t), year (uint16_t)
 *   - Latitude (float)
 *   - Longitude (float)
 *   - Speed (float)
 *   - Angle (float)
 *   - Altitude (float)
 *   - Satellites (uint8_t)
 *
 * If no valid fix is available, default value (0) is appended for all fields.
 *
 * @param packet Pointer to the packet byte array. This pointer is incremented
 * as each value is copied.
 */
void LSM9DS1Sensor::readDataPacket(uint8_t*& packet) {
  lsm.read();
  sensors_event_t aevent, mevent, gevent, temp_event;
  lsm.getEvent(&aevent, &mevent, &gevent, &temp_event);

  // calibration offsets to accelerometer data
  float accX = aevent.acceleration.x - accel_offsets[0];
  float accY = aevent.acceleration.y - accel_offsets[1];
  float accZ = aevent.acceleration.z - accel_offsets[2];

  // calibration offsets to gyroscope data
  float gyroX = gevent.gyro.x - gyro_offsets[0];
  float gyroY = gevent.gyro.y - gyro_offsets[1];
  float gyroZ = gevent.gyro.z - gyro_offsets[2];

  // calibration offsets to magnetometer data
  float magX = mevent.magnetic.x - mag_offsets[0];
  float magY = mevent.magnetic.y - mag_offsets[1];
  float magZ = mevent.magnetic.z - mag_offsets[2];

  float data[9] = {accX, accY, accZ, gyroX, gyroY, gyroZ, magX, magY, magZ};

  for (int i = 0; i < 9; i++) {
    memcpy(packet, &data[i], sizeof(float));
    packet += sizeof(float);
  }
}

/**
 * @brief Decodes the LSM9DS1 sensor data from the packet buffer into a CSV
 * string.
 *
 * The data are read in the same order they were written and  reconstructed as a
 * string "day/month/year" and the remaining fields are appended as CSV values.
 *
 * @param packet Pointer to the packet byte array and this packet pointer is
 * incremented.
 * @return String The decoded sensor data in CSV format.
 */
String LSM9DS1Sensor::decodeToCSV(uint8_t*& packet) {
  float data[9];
  for (int i = 0; i < 9; i++) {
    memcpy(&data[i], packet, sizeof(float));
    packet += sizeof(float);
  }

  return String(data[0]) + "," + String(data[1]) + "," + String(data[2]) + "," +
         String(data[3]) + "," + String(data[4]) + "," + String(data[5]) + "," +
         String(data[6]) + "," + String(data[7]) + "," + String(data[8]) + "," +
         String(data[8]) + ",";
}