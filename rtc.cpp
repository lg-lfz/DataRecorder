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

void setDateTimeFromISO8601(const char* iso8601DateTime, RTC_DS3231 &rtc) {
    int year, month, day, hour, minute, second;
    sscanf(iso8601DateTime, "%d-%d-%dT%d:%d:%d", &year, &month, &day, &hour, &minute, &second);
    rtc.adjust(DateTime(year, month, day, hour, minute, second));
    Serial.printf("Date Time set to: %d-%d-%dT%d:%d:%d\n", year, month, day, hour, minute, second);
}

