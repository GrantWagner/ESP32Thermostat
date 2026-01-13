# ESP32 Garage Thermostat

## Goals
* A Simple Local Interface, with a small display and three buttons
* A web based interface for remote control
* Automatic shutoff based on timer
* Capable of sensing tempature
* Capable of switching a heater based on temp

## Hardware
* [ESP32 dev board](https://www.amazon.com/dp/B0718T232Z)
* [DMT22 temp Sensor](https://www.amazon.com/dp/B0CPHQC9SF)
* [Relay module](https://www.amazon.com/dp/B09G6H7JDT)
* [128x32 i2c bitmaped display module](https://www.amazon.com/dp/B09YD8284T)

## Build Process
### Arduino IDE
* Create new Project
* Import all *.ino files
* Import all *.h files
* Update secrets.h
* Generate WebPage.html.h 
  This must be repeated everytime the html file is updated
  ```
  xxd -i WebPage.html > WebPage.html.h 
  ```
* Build and Upload

### CLion
TODO
