# Infinitag Target
[![CC BY-NC-SA 4.0][cc-by-nc-sa-shield]][cc-by-nc-sa] [![version](https://img.shields.io/badge/version-3.1.0-lightgray.svg)](https://github.com/Infinitag/Target)

The Infinitag Target is a hit target for the Infinitag system based on an ESP32 S3 module. 
The target has the following functions:

- 12 individually controllable RGBW LEDs on the front for a status display.
- IR receivers to receive the signals
- One switchable 5V connector
- One switchable 3.3V connector
- One switches to pass the incoming signal
- Power supply port
- Extension port for additional external LEDs



## Software

In the "ArduinoCode" directory is the source code as .ino file for the Arduino IDE to upload to the taget board.



### Preparation

In order for the board to be recorded successfully, you must first add another source in the settings for the board management:

```
https://dl.espressif.com/dl/package_esp32_index.json
```

The board uses the CP2102N chipset to program the ESP32.
Please check in your device management if you have installed an appropriate driver on your device.
If not, install the appropriate driver, so that the Arduino IDE recognizes the target board.



### Settings

The following must be selected as the board for the upload:
ESP32 Arduino > ESP32 S3 Dev Module



## Hardware

### PCB

We have published all files for the PCB incl. schematic, PCB design and BOM here:

https://oshwlab.com/crusher/infinitag-target

### 3D files

The files for a 3D enclosure can be found in the folder "Hardware/3DFiles".

Additionally you can find the latest version under the following link:

https://a360.co/3NOv182



## License
This work is licensed under a
[Creative Commons Attribution-NonCommercial-ShareAlike 4.0 International License][cc-by-nc-sa].

[![CC BY-NC-SA 4.0][cc-by-nc-sa-image]][cc-by-nc-sa]

[cc-by-nc-sa]: http://creativecommons.org/licenses/by-nc-sa/4.0/
[cc-by-nc-sa-image]: https://licensebuttons.net/l/by-nc-sa/4.0/88x31.png
[cc-by-nc-sa-shield]: https://img.shields.io/badge/License-CC%20BY--NC--SA%204.0-lightgrey.svg

## Additional

For more information about the Infinitag System please visit [infinitag.io](http://www.infinitag.io).
