# ESP32 MiniSD Project

This repository includes examples of two exciting projects currently in progress:

1. **70mai Dashcam File Copier** - Located in the folder `./70mai`, this project enables seamless file transfer from your 70mai dashcam.
2. **Simple FTP Server** - Located in the folder `./ftp-server`, this project transforms your device into a lightweight FTP server.

Welcome to the ESP32 MiniSD project! This is a flexible and compact design built for ESP32-C6 and ESP32-C3 microcontrollers, allowing you to easily integrate SD card storage, LED indicators, and charging capabilities. Perfect for data logging, IoT projects, or prototyping smart devices.

---

## Features

- **Supports both ESP32-C6 and ESP32-C3 microcontrollers**
- **MicroSD Card Support** with SPI interface
- **Meter Integration** for real-time monitoring
- **Onboard Charging Circuit** for convenience
- **LED Indicator** for visual feedback

---

## Pin Configuration

### ESP32-C6

| Pin | GPIO   | Function  |
| --- | ------ | --------- |
| D0  | GPIO0  | METER     |
| D1  | GPIO1  | METER\_EN |
| D2  | GPIO2  | SD\_CS    |
| D3  | GPIO21 | SD\_MOSI  |
| D4  | GPIO22 | SD\_CLK   |
| D5  | GPIO23 | SD\_MISO  |
| D6  | GPIO16 | SD\_EN    |
| D10 | GPIO18 | CHARGE    |
| LED | GPIO15 | LED       |

### ESP32-C3

| Pin | GPIO   | Function  |
| --- | ------ | --------- |
| D0  | GPIO2  | METER     |
| D1  | GPIO3  | METER\_EN |
| D2  | GPIO4  | SD\_CS    |
| D3  | GPIO5  | SD\_MOSI  |
| D4  | GPIO6  | SD\_CLK   |
| D5  | GPIO7  | SD\_MISO  |
| D6  | GPIO21 | SD\_EN    |
| D10 | GPIO10 | CHARGE    |

---

## Quick Start Guide

1. **Hardware Setup**

   - Connect the appropriate ESP32 microcontroller (C6 or C3) to your MiniSD module.
   - Ensure your SD card is inserted and properly formatted.

2. **Software Installation**

   - Clone this repository to your local machine.
   - Flash your ESP32 with the provided firmware or your custom application.

3. **Power and Test**

   - Power the device via USB or external battery.
   - Use the onboard LED and charging circuit to monitor functionality.

---

## Full Kit Available

Get the **ESP32 MiniSD Full Kit** preassembled for ESP32-C3 at [Silocity Labs](https://shop.silocitylabs.com/products/esp32minisd). For ESP32-C6 compatibility, purchase the **PCB Only** from our [Tindie Store](https://www.tindie.com/products/plantcare/esp32minisd/).

---

## License

This project is licensed under the MIT License. See the LICENSE file for details.

---

## Contribution

We also encourage you to share your own examples or projects using this hardware! Submit your examples through pull requests to help others explore the full potential of the ESP32 MiniSD module.

We welcome contributions! Feel free to submit issues or pull requests to improve this project.

---

## Contact

For support or inquiries, please visit our [shop page](https://shop.silocitylabs.com/products/esp32minisd) or contact us through the provided channels.

