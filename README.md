# <img src="./docs/logo/logo.svg" alt="NodeMCU WiFi Alarm Logo" width="50px"> NodeMCU WiFi Alarm
A simple firmware that makes NodeMCU a Wireless alarm center.

![](https://img.shields.io/github/license/Ax3lFernus/NodeMCU_WiFi_Alarm?style=for-the-badge) ![](https://img.shields.io/github/last-commit/Ax3lFernus/NodeMCU_WiFi_Alarm?style=for-the-badge) ![](https://img.shields.io/github/release-date/Ax3lFernus/NodeMCU_WiFi_Alarm?style=for-the-badge)

## Table of Contents
- [Features](#features)
  - [RC522 Support](#rc522-support)
  - [Wireless Connectivity](#wireless-connectivity)
  - [APIs](#apis)
  - [Web Panel](#web-panel)
- [Installation](#installation)
  - [Wiring Diagram](#wiring-diagram)
  - [Flash firmware](#flash-firmware)
  - [Install Web Panel](#install-web-panel)
- [The Author](#the-author)
- [License](#license)

## Features
### RC522 Support
### Wireless Connectivity
### APIs
### Web Panel

## Installation
### Wiring Diagram
<img src="./docs/WiringDiagram.png" alt="Wiring Diagram" width="60%">

The role of each pin used in the board is described below:
|Pin|Description|
| ------------ | ------------ |
|D0|It gives the signal to the siren to make it sound|
|D1|Detects the status of the door contact|
|D2|Detects the status of the tamper contact|
|D3|Reserved for the RST pin of the RFID board|
|D4|Reserved for the SDA pin of the RFID board|
|D5|Reserved for the SCK pin of the RFID board|
|D6|Reserved for the MISO pin of the RFID board|
|D7|Reserved for the MOSI pin of the RFID board|
|D8|Indicates the alarm status (enabled/disabled)|

### Flash firmware
You can flash the firmware on the NodeMCU via the [Arduino IDE](https://www.arduino.cc/en/Main/Software).
If you install the id for the first time you will need:
1. Go to File -> Preferences -> Add the link http://arduino.esp8266.com/stable/package_esp8266com_index.json in "Additional Boards Manager URLs" -> Click "OK"
2. Go to Tools -> Board -> click on "Boards Manager" -> search "esp8266" -> select leatest version -> click "Install" -> when finished click "Close"
3. Go to Tools and check this configurations:

  | Name | Value |
  |------|-------|
  | Board| NodeMCU 1.0 (ESP-12E Module)|
  |Upload Speed| 115200|
  |Port|Select the COM port where the board is connected|  

4. Go to File -> Open -> select 'firmware.ino' file from the repository directory on your pc -> when the code opens go to "Network SSID" and "Network PASSWORD" in the code and insert your network credentials between the quotes
5. In "UID Card Code" enter the code of your card for enable/disable the alarm.
6. Click on the arrow pointing to the right to flash the card.
7. That's all!

**:information_source: Note:** if you don't know the uid of your card you will need to: flash the card -> open the serial monitor (select the value 115200 as baud rate) -> scan the RFID card -> copy the UID that comes out in the monitor -> go to step 5.
### Install Web Panel

## The Author
This software was developed by Alessandro Annese.<br>
You can follow me on:<br>
[Linkedin](https://www.linkedin.com/in/alessandro-annese-79683913b/)<br>
[GitHub](https://github.com/Ax3lFernus)

## License
[BSD 3-Clause License](https://github.com/Ax3lFernus/NodeMCU_WiFi_Alarm/blob/master/LICENSE)

Copyright &copy; 2020, Alessandro Annese<br>
All rights reserved.
