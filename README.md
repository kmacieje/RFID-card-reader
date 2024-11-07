# RFID card reader
### Purpose
The main goal of the project was to create a scanning card system for prisons and dive into embedded topics based on ESP8266 microcontroller.
### Description
The ESP8266 microcontroller utilizes an external RFID module to read appropriate tags from cards and decides on the action to take. If the ID is not found in the database, the card is classified as unknown, and after three unsuccessful authentication attempts, an alarm is triggered. The system also features an Admin mode, protection against unauthorized case opening, and battery backup power. The Wi-Fi module connects to the network and transmits data in JSON format to a Google Sheets database via HTTPS protocol. Additionally, a PCB board has been designed to provide the most practical solution for the device.
### Software
- Visual Studio Code with PlatformIO
- Arduino IDE
### Technologies
- C++
- JavaScript
### Hardware components
- ESP-12E chip
- MF RC522 RFID Module
- OLED Display 128x32
- battery backup power including: LDO, TP-4056
- Li-Pol battery
- LED diodes
- reed switch
- switch buttons
- resistors, capacitors, wires, fuse

