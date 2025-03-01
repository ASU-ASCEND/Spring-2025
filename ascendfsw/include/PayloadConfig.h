/**
 * @file PayloadConfig.h
 * @brief Central location for pinout and config defines
 *
 * All pin definitions must include PIN in their name to allow the conflict
 * checker to catch pin conflits ex: SD_CS_PIN instead of just SD_CS
 *
 */

#ifndef PAYLOAD_CONFIG_H
#define PAYLOAD_CONFIG_H

/** @brief Error display bit 2 */
#define ERROR_PIN_2 22
/** @brief Error display bit 1 */
#define ERROR_PIN_1 15
/** @brief Error display bit 0 */
#define ERROR_PIN_0 14

// sensors
/** @brief BME680 CS Pin */
#define BME680_SPI_CS_PIN 3
/** @brief BME680 SPI Mode Toggle Pin */
#define BME680_SPI_MODE 0

/** @brief LSM9DS1 IMU Accelerometer/Gyroscope Pin */
#define LSM9DS1_XGCS_PIN 27
/** @brief LSM9DS1 IMU Magnetometer Pin */
#define LSM9DS1_MCS_PIN 28

/** @brief MTK3339 GPS CS Pin*/
// #define MTK3339_CS_PIN 20
#define SERIAL2_RX_PIN 8
#define SERIAL2_TX_PIN 9

/** @brief UV sensor I2C Addresses */
// [1, 1, 1, 0, 1, A1, A0]
#define UV_I2C_ADDR_1 0b1110110
#define UV_I2C_ADDR_2 0b1110100

// storages
// for radio
/** @brief UART0 RX Pin */
#define SERIAL1_RX_PIN 1
/** @brief UART0 RX Pin */
#define SERIAL1_TX_PIN 0

/** @brief SD Card SPI Toggle */
#define SD_SPI1 1
/** @brief SD Card SPI CS Pin */
#define SD_CS_PIN 7

/** @brief Flash memory SPI Toggle */
#define FLASH_SPI1 0
/** @brief Flash chip SPI CS Pin */
#define FLASH_CS_PIN 17

// spi1
/** @brief SPI1 MISO Pin */
#define SPI1_MISO_PIN 12
/** @brief SPI1 SCK Pin */
#define SPI1_SCK_PIN 10
/** @brief SPI1 MOSI Pin */
#define SPI1_MOSI_PIN 11

// main pin definitions
/** @brief Built-in LED Pin */
#define ON_BOARD_LED_PIN 25
/** @brief Core 0 Heartbeat Pin */
#define HEARTBEAT_PIN_0 21
/** @brief Core 1 Heartbeat Pin */
#define HEARTBEAT_PIN_1 20
/** @brief Flash Data Interface Enable Pin  */
#define DATA_INTERFACE_PIN 26

// multicore transfer queue
#define QT_ENTRY_SIZE 500
#define QT_MAX_SIZE 10

// packet properties
const uint8_t SYNC_BYTES[] = {'A', 'S', 'U', '!'};

// temporary toggle macros for testing
#define PACKET_SYSTEM_TESTING 1
#define RECOVERY_SYSTEM 1

#endif