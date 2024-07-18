#include "rtc.h"

int initRTC3231(RTC_DS3231 &rtc)
{
    Serial.printf("Checking for RTC DS3231...\n");
    if (!rtc.begin())
    {
        Serial.println("Couldn't find RTC DS3231");
        Serial.flush();
        return -1;
    }
    TimeDateData now = read_time(rtc);
    Serial.printf("RTC DS3231 found, act. Date Time: %s\n", formatISO8601(now.Year, now.Month, now.Day, now.Hour, now.Minute, now.Second).c_str());
    Serial.flush();
    return 0;
}

TimeDateData read_time(RTC_DS3231 &rtc)
{
  DateTime now = rtc.now();
  TimeDateData tdd;
  tdd.Year = now.year();
  tdd.Day = now.day();
  tdd.Month = now.month();
  tdd.Minute = now.minute();
  tdd.Hour = now.hour();
  tdd.Second = now.second();
  return tdd;
}

