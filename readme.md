# NodeMCU Amica Module ESP8266MOD

In this project we use the NodeMCU Lua Amica Module V2 ESP8266 from AZ-Delivery:
https://www.az-delivery.de/en/products/nodemcu

# ESP8266 Pinout

![Pinout](./assets/ESP8266-Pinout-NodeMCU.webp)

# ESP8266 Wireing

ESP8266 D1 -> BME280 and DS3231 SDA
ESP8266 D2 -> BME280 and DS3231 SCL
ESP8266 3.3V -> BME280 and DS3231 VIN
ESP8266 GND -> BME280 and DS3231 GND

ESP8266 A0 -> Analoge current measurement (Analog-to-Digital Converter)

# BME280 Sensor

![BME280](./assets/bme280.png)

# DS3231 Sensor

![DS3231](./assets/ds3231.jpg)

# A0 for measurement

A0 Pin Functionality:
Analog Input Capability: A0 on the ESP8266 allows you to read analog voltages between 0V to 1.0V when using the default voltage reference. This pin can be used with analog sensors that output varying voltages corresponding to physical measurements such as light intensity, temperature, humidity, etc.

Resolution: The ADC (Analog-to-Digital Converter) on the ESP8266 has 10-bit resolution, meaning it can map analog voltages to digital values ranging from 0 to 1023.

Voltage Reference: By default, the ADC uses an internal reference voltage of 1.0V. If needed, you can also set an external reference voltage.

Programming: In Arduino or other compatible development environments, you can read the analog voltage on A0 using functions like analogRead(A0), which returns an integer value representing the voltage level between 0 and 1023.

# Libs

ESPAsyncTCP.h
ESPAsyncWebServer.h
Adafruit_BME280.h
RTClib.h
ArduinoJson.h

# Setup arduino enviroment for VS Code with arduino-cli

Commands to use:

```
arduino-cli core update-index
arduino-cli core upgrade
arduino-cli core install esp8266:esp8266

arduino-cli lib install ESPAsyncTCP
arduino-cli lib install ESPAsyncWebServer
arduino-cli lib install "Adafruit BME280 Library"
arduino-cli lib install RTClib
arduino-cli lib install ArduinoJson

arduino-cli lib update-index
arduino-cli lib list

arduino-cli lib upgrade <library-name>
```

# Filesystem on ESP 8266

This ESP has a flash disk on board. With the LittleFS lib, it is possible to store data on that.
During the boot with this code, we get some info about the capacity and the used storage.

Init LittleFS Filesystem...
LittleFS mounted successfully
Listing directory: /
Filename: data.csv, Size: 319 bytes
Total space:      2072576 bytes
Used space:       24576 bytes
Available space:  2048000 bytes

That's all about around 2 MB of diskspace.

If we assume to have a line of data in our csv-file with about 150 characters, each character typically takes up 1 byte in a standard ASCII encoding. Therefore, each line would take approximately 150 bytes of space. We can then calculate the number of lines that can be stored as follows:

2048000 bytes / 150 bytes/line = appr. 13500 lines

depending on how frequent we store data, the memory will last for x hours of storing data.
Example:

Every 5 sec, that means 60sec/5sec=12 lines a minute we store data.
One hour has 60min, that means 60min*12 lines = 720 lines per hour.

13500 lines / 720 lines per hour = 18,75 hours of storing data.

# HTML 

In order to use Syntax highlighting and IntelliSense from VS Code, we have a sperate [html.html](html.html) in our repo, so we can edit things there and afterwards copy it over to the [html.h](html.h) file. The content of this html file will then be accessible with the variable name *htmlContent* if you include the html.h file in your sources...

# Releases

Find some releases here: https://github.com/lg-lfz/DataRecorder/releases/
They can be flashed with the online flasher https://web.esphome.io/
