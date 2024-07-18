#include "bme280.h"

int initBME280(uint8_t addr, Adafruit_BME280 & bme)
{
    Serial.printf("Checking for BME280 at addr: 0x%x...\n", addr);
    bool status = bme.begin(addr, &Wire);
    if (!status)
    {
        Serial.println("No BME280 found! Check wiring!");
        Serial.flush();
        return -1;
    }
    Serial.printf("BME280 found, act. Temperature %0.2f Grad Celsius\n", bme.readTemperature());
    Serial.flush();
    return 0;
}