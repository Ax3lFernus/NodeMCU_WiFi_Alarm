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
  - [Change the timers](#change-the-timers)
  - [Install Web Panel](#install-web-panel)
- [The Author](#the-author)
- [License](#license)

## Features
### RC522 Support
With the RFID-RC522 card it is possible to activate and deactivate the alarm via an RFID tag or card.
<img src="./docs/images/rfid_card.jpg" alt="RFID Card" width=40%>&nbsp;<img src="./docs/images/rfid_tag.jpg" alt="RFID Tag" width="53.5%">
### Wireless Connectivity
The NodeMCU card was born to have an integrated WiFi NIC. This functionality is used to check the card to read its status through the [APIs](#apis).
### APIs
The APIs provide commands to check and modify the alarm status. 
It is necessary to know the local IP of the card in order to interact with it through HTTP GET requests, the answers are in json format.
Here is the list of possible requests:
1. To know the status of the card just send the request `http://<CARD_IP>/status`
2. To activate the alarm just send the request `http://<CARD_IP>/1/on`
3. To deactivate the alarm just send the request `http://<CARD_IP>/1/off`
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

4. Go to File -> Open -> select 'firmware.ino' file from the repository directory on your pc -> when the code opens go to "Network SSID" and "Network PASSWORD" in the code and insert your network credentials between the quotes<br>
`// Network and UID Credentials`<br>
`const char *ssid = "<YOUR WIFI NAME HERE>";          //Network SSID`<br>
`const char *password = "<YOUR WIFI PASSWORD HERE>"; //Network PASSWORD`<br>
`const char *UID = "<UID CARD HERE>";                //UID Card Code`<br>
5. In "UID Card Code" enter the code of your card for enable/disable the alarm.
6. Click on the arrow pointing to the right to flash the card.
7. That's all!

**:information_source: Note:** if you don't know the uid of your card you will need to: flash the card -> open the serial monitor (select the value 115200 as baud rate) -> scan the RFID card -> copy the UID that comes out in the monitor -> go to step 5.
### Change the timers
### Install Web Panel
The web panel doesn't require installation so you can open `./webServer/index.html` with a browser without using additional programs. The only step is to open the `index.html` with a text editor (es. Notepad++) and insert the ip of the card in the local network in the field `nodemcu_ip`:<br>
`let nodemcu_ip = "<NODEMCU IP HERE>" //Set your NodeMCU IP ex: 192.168.1.1`<br>
However, you can upload the `index.html` to a web server to be able to visit it from other devices on the same local network.<br>
<br>
**:warning: Warning:** the panel must be started on a device connected in the same local network as the NodeMCU board.
## The Author
This software was developed by Alessandro Annese.<br>
You can follow me on:<br>
[Linkedin](https://www.linkedin.com/in/alessandro-annese-79683913b/)<br>
[GitHub](https://github.com/Ax3lFernus)

## License
[BSD 3-Clause License](https://github.com/Ax3lFernus/NodeMCU_WiFi_Alarm/blob/master/LICENSE)

Copyright &copy; 2020, Alessandro Annese<br>
All rights reserved.
