# ESP32 Garage Thermostat

## Goals
* A Simple Local Interface, with a small display and three buttons
* A web based interface for remote control
* Automatic shutoff based on timer
* Capable of sensing temperature
* Capable of switching a heater based on temp

## Hardware
* [ESP32 dev board](https://www.amazon.com/dp/B0718T232Z)
* [DMT22 temp Sensor](https://www.amazon.com/dp/B0CPHQC9SF)
* [Relay module](https://www.amazon.com/dp/B09G6H7JDT)
* [128x32 i2c bitmaped display module](https://www.amazon.com/dp/B09YD8284T)

## Build Process
### Arduino IDE (Legacy)
* Create new Project
* Rename all *.cpp files to *.ino and import.
* Import all *.h files
* Update secrets.h
* Generate WebPage.html.h 
  This must be repeated everytime the html file is updated
  ```
  xxd -i WebPage.html > WebPage.html.h 
  ```
* Build and Upload

### CLion
* Install [CLion](https://www.jetbrains.com/clion/download/?section=linux) for your platform. It requires registration but is free for individual use.
* Install [PlatformIO Core](https://platformio.org/install/cli)
* Install [PlatformIO for CLion](https://plugins.jetbrains.com/plugin/13922-platformio-for-clion) plugin 
* Open the root as a project
* Update secrets.h
* TODO auto import/generate WebPage.html file
