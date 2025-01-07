# ESP32 MiniSD FTP Server Example

This is an example of how to use SimpleFTPServer with the ESP32MiniSD Hat by SiloCityLabs. This code base is a work in progress and may not be complete yet.

# Guide

 - Compile/flash the firmware using arduino ide.
 - Format a microsd card no bigger than 32GB to fat32.
 - Copy the `config.txt.example` to `/config.txt` on the microsd card.

# Libraries

This uses a modified SimpleFTPServer 2.1.7 located in libraries folder of this repository. You can copy the contents of this folder to your `~/Arduino/libraries/` folder

`FtpServerKey.h` was modified to setup `STORAGE_SD` as the default

```
	#define DEFAULT_FTP_SERVER_NETWORK_TYPE_ESP32 		NETWORK_ESP32
	#define DEFAULT_STORAGE_TYPE_ESP32 					STORAGE_SD
```