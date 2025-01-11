# Hardware

## Microcontroller 
[Raspberry Pi Pico](https://www.raspberrypi.com/documentation/microcontrollers/pico-series.html)

## Watchdog Chip
[Attiny85](https://www.microchip.com/en-us/product/ATtiny85)

## Sensors 

#### [MD0555](https://moderndevice.com/products/wind-sensor-rev-p): Wind Sensor
Protocol: 
<br/>Calibration: 

#### [PT1000](https://microtherm.de/files/microtherm/downloads/mirotherm_PT1000_en.pdf): Temperature
Protocol: 
<br/>Calibration:  

#### [BME680](https://www.adafruit.com/product/3660): Temperature, Pressure, Humidity, Gas, Altitude

Protocol: 
<br/>Calibration:  
=======
**Protocol**: **I2C**, **SPI 4-Wire**, **SPI 3-Wire**
- **I2C Mode**
  - **SCK**: Serial clock (SCL)
  - **SDI**: Data (SDA)
  - **SDO**: Slave Address LSB (GND = `0` (`0x76` address), V<sub>DDIO</sub> = `1` (`0x77` address))
- **SPI Mode**
  - **CSB**: Chip Select (CSB)
  - **SDI**: Input Data (SPI 4W = SDI, SPI 3W = SDI/SDO)
  - **SCK**: Clock (SCK)
  - **SDO**: Output Data (SPI 4W = SDO, SPI 3W = DNC (Do not connect))

**Calibration**: No calibration required. The BME680 uses built-in algorithms for accurate readings.

#### [Mighty Ohm Geiger Counter](https://mightyohm.com/blog/products/geiger-counter/): CPS 
Protocol: 
<br/>Calibration:  

#### [INA260](https://www.ti.com/product/INA260): current, voltage, power
Protocol: **I<sup>2</sup>C**
- **I<sup>2</sup>C Mode**
  - **SCL**: Serial Clock Line (no. 9)
  - **SDA**: Serial Data Line (no. 8)

Calibration: No calibration required as the INA260 is factory-calibrated.

#### [LSM9DS1](https://www.st.com/en/mems-and-sensors/lsm9ds1.html): 9-axis IMU
Protocol: I2C: Easy to use but slower data rates.
SPI: Preferred for faster sampling rates; recommended for high-speed applications.

<br/>Calibration: Necessary for accurate measurements. Perform offset calibration for accelerometer and gyroscope. Magnetometer calibration requires hard iron offset correction.

Libraries: SparkFun LSM9DS1 Library: Supports both I²C and SPI.
GitHub Repository: SparkFun_LSM9DS1_Arduino_Library

#### [MTK3339](https://www.adafruit.com/product/746): GPS
Protocol: 
<br/>Calibration:  

#### [SGP30](https://www.mouser.com/pdfdocs/Sensirion_Gas_Sensors_SGP30_Datasheet_EN-1148053.pdf): Gas Sensor
Protocol: I^2C
<br/>Callibration:
Sensor raw signals are used as inputs for the on-chip calibration and baseline compensation algorithms.
The TVOC and CO2 equivalent are calculated from the sensor signals.
Sensor is 2 data bytes (MSB first) and 1 CRC byte in intervals of 1s for each of two air quality signals.
First 15 seconds is intitialization phase and returns fixed values of TVOC and CO2.

#### [SHT31](https://sensirion.com/media/documents/213E6A3B/63A5A569/Datasheet_SHT3x_DIS.pdf): Humidity, Temperature
Protocol: I2C Interface with communication speeds up to 1 MHz and two user selectable addresses

<br/>Calibration: The sensor reloads calibration data prior to every measurement by default, self calibrated in calibration memory.

#### [ZOPT220](https://www.sparkfun.com/products/retired/14264): UVB Sensor
--Not being used this semester--

#### [AS7331](https://www.sparkfun.com/products/23517): UVA/UVB/UVC Sensor
Protocol:
<br/>Calibration:

#### [Real Time Clock](https://www.adafruit.com/product/3295): 
Protocol:
<br/>Calibration: 

#### On Board Temperature (connected to on-board ADC)
Protocol: ADC (analog read)
<br/>Calibration: Maybe, probably N/A

## Storages
#### [SD Card](https://www.adafruit.com/product/4682)
Protocol: SPI

#### [Flash](https://www.adafruit.com/product/5643)
Protocol: SPI

#### [Radio](https://rfdesign.com.au/modems/)
Protocol: UART
