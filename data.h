#pragma once

#include <Arduino.h>

struct SensorData
{
    float pressure;
    float altitude;
    float temperature;
    float measurement_value;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    uint8_t day;
    uint8_t month;
    uint16_t year;
};

struct TimeDateData
{
    uint16_t Year;
    uint8_t Day;
    uint8_t Month;
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
};

String formatISO8601(const uint16_t year, const uint8_t month, const uint8_t day, const uint8_t hour, const uint8_t minute, const uint8_t second);
String formatISO8601(const TimeDateData & time_date_data);
String getJSON(const SensorData & data);