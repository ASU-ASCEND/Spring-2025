# Spring-2025
Flight software for the ASU Spring 2025 ASCEND Payload

## GitHub Subfolders

[DataProcessing](https://github.com/ASU-ASCEND/Spring-2025/tree/main/DataProcessing): Scripts and programs for processing and visualizing flight data.

[Testing](https://github.com/ASU-ASCEND/Spring-2025/tree/main/Testing): Test and calibration information for sensor and storage modules.

[Watchdog](https://github.com/ASU-ASCEND/Spring-2025/tree/main/Watchdog): Software for watchdog Attiny85 chip.

[ascendfsw](https://github.com/ASU-ASCEND/Spring-2025/tree/main/ascendfsw): Payload flight software.

[docs](https://github.com/ASU-ASCEND/Spring-2025/tree/main/docs): Doxygen auto-generated documentation site for GitHub pages. 

[scripts](https://github.com/ASU-ASCEND/Spring-2025/tree/main/scripts): Various organizational and automation scripts. 


## Meeting Schedule

# Potential Milestones 

## FSW Versions 

### FSWv3 (and earlier)
[Fall 2024 Github](https://github.com/ASU-ASCEND/Fall-2024)

### FSWv4
* Packeting (un-packeting)
  * Data to uint8_t array over String
* Device Management System
  * Trigger sensor reconnection attempts
* Prefixing Serial Messages ([Core 1], etc)
* Flash System Recovery System

## Data Processing Tools

### Visualization 
#### Cesium.js 
* 3D Visualization 
#### D3.js 
* 2D Visualization 

### Groundstation GUI
#### Backend 
* Serial Connection 
* Hardline Sorter 
* Packet Parser 
* Recovery Iterface
* File log dumps 

#### Frontend 
* Command line tool (settings/config file)
* Server/Database output 
* GUI 
