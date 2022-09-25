# Infinitag Target

The Infinitag Target is a hit target for the Infinitag system based on an ESP32 module. 
The target has the following functions:
- 12 individually controllable RGBW LEDs on the front for a status display.
- IR receivers to receive the signals
- I2C input and output 
- One switchable 5V connector
- One switchable 3.3V connector
- Two switches to pass the incoming signal



## Hardware

The Hardware folder contains:
- KiCad6 project
- JLCPCB files



> :warning: **In progress**: Currently all files are still in an alpha version and should not be ordered yet



### KiCad6 project

The project for KiCad6 contains all files for the target board incl. scheme files & Co.



### JLCPCB files

In the JLCPCB folder all files are ready to be uploaded and ordered directly at https://jlcpcb.com/. In addition, the assembly and positioning files are also here, so that the board can be ordered directly assembled, if desired.




## Software

In the code directory is the source code as .ino file for the Arduino IDE to upload to the taget board.



### Preparation

In order for the board to be recorded successfully, you must first add another source in the settings for the board management:

```
https://dl.espressif.com/dl/package_esp32_index.json
```

The board uses the CH340 chipset to program the ESP32.
Please check in your device management if you have installed an appropriate driver on your device.
If not, install the appropriate driver, so that the Arduino IDE recognizes the target board.



### Settings

The following must be selected as the board for the upload:
ESP32 Arduino > ESP32 Dev Module

Additionally the value "16MB (128Mb)" must be selected under "Flash size".
