# ASU StratoDevils ASCEND Spring 2025
[Website](https://asuascend.weebly.com/) | [YouTube](https://www.youtube.com/@ASUStratoDevilsASCEND)

### Subsystems 
* [Software](#software)
* [Electrical](#electrical)
* [Mechanical](#mechanical)

# Software
Flight software for the ASU Spring 2025 ASCEND Payload

## GitHub Subfolders

[DataProcessing](https://github.com/ASU-ASCEND/Spring-2025/tree/main/DataProcessing): Scripts and programs for processing and visualizing flight data.

[Testing](https://github.com/ASU-ASCEND/Spring-2025/tree/main/Testing): Test and calibration information for sensor and storage modules.

[Watchdog](https://github.com/ASU-ASCEND/Spring-2025/tree/main/Watchdog): Software for watchdog Attiny85 chip.

[ascendfsw](https://github.com/ASU-ASCEND/Spring-2025/tree/main/ascendfsw): Payload flight software.

[scripts](https://github.com/ASU-ASCEND/Spring-2025/tree/main/scripts): Various organizational and automation scripts. 

A Doxygen auto-generated documentation site for GitHub pages is deployed from the [docs branch](https://github.com/ASU-ASCEND/Spring-2025/tree/docs). 


# Milestones 

## FSW Versions 

### FSWv3 (and earlier)
[Fall 2024 Github](https://github.com/ASU-ASCEND/Fall-2024)

### FSWv4
* Packeting (un-packeting)
  * Data to uint8_t array over String
* Device Recovery System
  * Trigger sensor reconnection attempts
* Prefixing Serial Messages ([Core 1], etc) for sorting
* Flash System Recovery System

## Data Processing Tools

### Visualization 
* [Cesium.js](https://cesium.com/platform/cesiumjs/) 
  * 3D Visualization of the flight ([/DataProcessing/3D](/DataProcessing/3D/))
  * Visualization from Spring 2025: https://www.youtube.com/watch?v=eEC_ttqKnY8
* [D3.js](https://d3js.org/) 
  * 2D Visualization of sensor reading ([/DataProcessing/2D](/DataProcessing/2D/))
* [Jupyter Notebooks](https://jupyter.org/)
  * Data cleaning and [Skew-T Log-P Diagram](https://en.wikipedia.org/wiki/Skew-T_log-P_diagram) using [MetPy](https://unidata.github.io/MetPy/latest/)

### Groundstation GUI
[/GroundStation/GroundStation](/GroundStation/GroundStation/)
#### Backend
* Serial Connection 
* Serial Sorter 
* Packet Parser 
* Serial Sender
* File log dumps
* Server Interface/Process

#### Frontend 
* GUI for displaying 
  * Decoded Packet 
  * Sorted Core Debug Prints 
* Flash Data Recovery Interface 

# Electrical
Links to the PCBs design files  

### StratoCore PCB 
https://drive.google.com/file/d/1e-bXs0K2BFJtf7aeRV-rmkgyk9yUyAgQ/view?usp=sharing 
### StratoSense PCB
https://drive.google.com/file/d/113i7Ffd7eHaCxem99AW3rWAT8VqHN-kn/view?usp=sharing 

# Mechanical 
### CAD Models 