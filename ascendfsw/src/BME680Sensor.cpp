#include "BME680Sensor.h"

/**
 * @brief Default constructor for the BME680Sensor class.
 *
 * Initializes the sensor object with a default minimum period of 0
 * milliseconds.
 */
BME680Sensor::BME680Sensor() : BME680Sensor(0) {}

/**
 * @brief Parameterized constructor for the BME680Sensor class.
 *
 * This constructor initializes the BME680Sensor with a specified minimum period
 * between sensor readings. It passes sensor-specific information like the name,
 * CSV header, number of fields, and the minimum period between reads to the
 * base Sensor class constructor.
 *
 * @param minimum_period The minimum time (in milliseconds) between consecutive
 * sensor reads.
 */
BME680Sensor::BME680Sensor(unsigned long minimum_period)
    : Sensor("BME680",
             "BMETemp(C),BMEPress(hPa),BMEHum(%),BMEGas(KOhms),BMEAlt(m),", 5,
             minimum_period) {
#if BME680_SPI_MODE
  bme = Adafruit_BME680(BME680_SPI_CS_PIN);
#else
  bme = Adafruit_BME680();
#endif
}

/**
 * @brief Verifies the connection and readiness of the BME680 sensor.
 *
 * This function initializes the sensor and configures it by setting
 * temperature, humidity, and pressure oversampling, along with the gas heater
 * and IIR filter. It checks if the sensor is properly connected and ready for
 * reading data.
 *
 * @return true - If the sensor is detected and successfully initialized.
 * @return false - If the sensor is not detected or fails to initialize.
 */
bool BME680Sensor::verify() {
  if (!bme.begin()) {
    return false;
  }

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);  // 320°C for 150 ms

  return true;
}

/**
 * @brief Reads sensor data and returns it in CSV format.
 *
 * Performs a reading from the BME680 sensor, which includes temperature,
 * pressure, humidity, gas resistance, and an approximate altitude based on
 * sea-level pressure. If the sensor fails to perform a reading, a placeholder
 * string ("-, -, -, -, -, ") is returned.
 *
 * @return String - A string containing the sensor readings formatted as:
 * "Temperature (Celsius), Pressure (hPa), Humidity (%), Gas Resistance (KOhms),
 * Altitude (meters)".
 */
String BME680Sensor::readData() {
  if (!bme.performReading()) {
    return "-,-,-,-,-,";
  }

  return String(bme.temperature) + "," + String(bme.pressure / 100.0) + "," +
         String(bme.humidity) + "," + String(bme.gas_resistance) + "," +
         String(bme.readAltitude(SEALEVELPRESSURE_HPA)) + ",";
}

/**
 * @brief Reads sensor data and appends it to the packet byte array.
 *
 * Reads data from the BME680 sensor and appends it to the passed uint8_t array
 * pointer, incrementing it while doing so. The data includes temperature,
 * pressure, humidity, gas resistance, and an approximate altitude based on
 * sea-level pressure.
 *
 * @param packet - Pointer to the packet byte array.
 */
void BME680Sensor::readDataPacket(uint8_t*& packet) {
  if (!bme.performReading()) {
    return;
  }

  float data[5] = {bme.temperature, (float)(bme.pressure / 100.0), bme.humidity,
                   (float)(bme.gas_resistance), bme.readAltitude(SEALEVELPRESSURE_HPA)};

  for (int i = 0; i < 5; i++) {
    memcpy(packet, &data[i], sizeof(float));
    packet += sizeof(float);
  }
}

/**
 * @brief Decodes the packet data and returns it in CSV format.
 *
 * Decodes the packet data from the BME680 sensor and returns it in CSV format.
 * The data includes temperature, pressure, humidity, gas resistance, and an
 * approximate altitude based on sea-level pressure.
 *
 * @param packet - Packet to decode.
 * @return String - A string containing the sensor readings
 */
String BME680Sensor::decodeToCSV(uint8_t* packet) {
  float data[5];
  for (int i = 0; i < 5; i++) {
    memcpy(&data[i], packet, sizeof(float));
    packet += sizeof(float);
  }

  return String(data[0]) + "," + String(data[1]) + "," + String(data[2]) + "," +
         String(data[3]) + "," + String(data[4]) + ",";
}
