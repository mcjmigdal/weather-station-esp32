# e-paper wheatherstation mini-project

## About

This project is a small weather station for temperature, humidity and
pressure monitoring. The project is build around 
(BME280)[https://www.bosch-sensortec.com/products/environmental-sensors/humidity-sensors-bme280/] sensor,
using (ESP32)[https://www.espressif.com/en/products/socs/esp32] and 
(E-Ink 2.9" display)[https://www.waveshare.com/wiki/E-Paper_ESP32_Driver_Board].
The weather station is assembled without soldiering, instead I used
small breadboard and jumper wires. The code controling E-Ink display
is based on the code provided by (waveshare electronics)[https://files.waveshare.com/upload/5/50/E-Paper_ESP32_Driver_Board_Code.7z]
and BME280 part based on (Adafruit's)[https://github.com/adafruit/Adafruit_BME280_Library].

<img 
    alt="e-ink paper in photography frame displaying environmental readings"
    src="./station.png"
    style="height: 1200px; width: 850px"
/>


## Quick start

### Wiring

                         +-----------------------+
                         | O      | USB |      O |
                         |        -------        |
   BME280 VCC ------ 3V3 | [X]               [X] | VIN ------ EPD VCC
   BME280 GND ------ GND | [X]               [X] | GND ------ EPD GND
                  GPIO15 | [ ]               [X] | GPIO13 --- EPD CLK
                   GPIO2 | [ ]               [X] | GPIO12 --- EPD CS
                   GPIO4 | [ ]               [X] | GPIO14 --- EPD DIN
                  GPIO16 | [ ]               [X] | GPIO27 --- EPD DC
                  GPIO17 | [ ]               [X] | GPIO26 --- EPD RST
                   GPIO5 | [ ]  ___________  [X] | GPIO25 --- EPD BUSY
                  GPIO18 | [ ] |           | [ ] | GPIO33
                  GPIO19 | [ ] |           | [ ] | GPIO32
   BME280 SDA --- GPIO21 | [X] |           | [ ] | GPIO35
                   GPIO3 | [ ] |           | [ ] | GPIO34
                   GPIO1 | [ ] |           | [ ] | GPIO39
   BME280 SCL --- GPIO22 | [X] |           | [ ] | GPIO36
                  GPIO23 | [ ] |___________| [ ] | EN 
                         |                       |
                         |  |  |  ____  ____  |  |
                         |  |  |  |  |  |  |  |  |
                         |  |__|__|  |__|  |__|  |
                         | O                   O |
                         +-----------------------+

### Code upload

Open the project in Arduino IDE. Connect ESP32 board. Compile and upload code.

## Displaying pictures

In addition to environment readings station displays a static picture as well as measurments icons.
These can be modified by adding new images to `ImageData.h`. 
To converted the images to the required format use (png_to_header.py from lmarzen's repository)[https://github.com/lmarzen/esp32-weather-epd/blob/main/icons/png_to_header.py].
The commands could look like:

```
inkscape -w 128 -h 128 flower.svg -o flower.png --export-background="#ffffff"
python3 png_to_header.py -i flower.png -o flower.h 
```

Fair choice of images can be found at [icons-for-free.com](icons-for-free.com).

## Notes

ESP32 pinout ASCII Art borrowed from (iamamused)[https://gist.github.com/iamamused/42cad8a59547ed5580e78d5274220813].
