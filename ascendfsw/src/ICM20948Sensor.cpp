#include "ICM20948Sensor.h"

/**
 * @brief Construct a new ICM20948Sensor object with default minimum_period of 0
 *
 */
ICM20948Sensor::ICM20948Sensor() : ICM20948Sensor(0) {}

/**
 * @brief Construct a new ICM20948Sensor object, given mininum_period between
 * sensor reads
 *
 * @param minimum_period Minimum time to wait between readings in ms
 */
ICM20948Sensor::ICM20948Sensor(unsigned long minimum_period)
    : Sensor("ICM20948",
             "ICM AccX,ICM AccY,ICM AccZ,ICM GyroX,ICM "
             "GyroY,ICM GyroZ,ICM MagX,ICM MagY,ICM MagZ, ICM TempC,",
             10, minimum_period) {}

/**
 * @brief Verifies that the ICM is connected and working
 *
 * @return true if connected and working
 * @return false if not connected and working
 */
bool ICM20948Sensor::verify() {
#if ICM_I2C_MODE
  if (!this->icm.begin_I2C()) {
    return false;
  }
#else
  if (!this->icm.begin_SPI(ICM_CS_PIN)) {
    return false;
  }
#endif

  icm.setAccelRange(icm20948_accel_range_t::ICM20948_ACCEL_RANGE_16_G);
  icm.setGyroRange(icm20948_gyro_range_t::ICM20948_GYRO_RANGE_1000_DPS);
  icm.setMagDataRate(ak09916_data_rate_t::AK09916_MAG_DATARATE_50_HZ);

  this->icm_accel = icm.getAccelerometerSensor();
  this->icm_gyro = icm.getGyroSensor();
  this->icm_mag = icm.getMagnetometerSensor();
  this->icm_temp = icm.getTemperatureSensor();

  return true;
}

/**
 * @brief Retrieves data from ICM 9-axis IMU
 *
 * @return String A CSV section in format AccX, AccY, AccZ, GyroX, GyroY, GyroZ,
 * MagX, MagY, MagZ, TempC
 */
String ICM20948Sensor::readData() {
  // get new sensor events
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t temp;
  sensors_event_t mag;
  this->icm_temp->getEvent(&temp);
  this->icm_accel->getEvent(&accel);
  this->icm_gyro->getEvent(&gyro);
  this->icm_mag->getEvent(&mag);

  return String(accel.acceleration.x) + "," + String(accel.acceleration.y) +
         "," + String(accel.acceleration.z) + "," + String(gyro.gyro.x) + "," +
         String(gyro.gyro.y) + "," + String(gyro.gyro.z) + "," +
         String(mag.magnetic.x) + "," + String(mag.magnetic.y) + "," +
         String(mag.magnetic.z) + "," + String(temp.temperature) + ",";
}