#pragma once

#include <RTClib.h>
#include "data.h"

int initRTC3231(RTC_DS3231 & rtc);
TimeDateData read_time(RTC_DS3231 &rtc);
void setDateTimeFromISO8601(const char* iso8601DateTime, RTC_DS3231 &rtc);

