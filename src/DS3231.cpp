#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;

void InitRTC() {
    Wire.begin();
    rtc.begin();
    if (rtc.lostPower()) {
        rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
    }
}

DateTime getTime() {
    return rtc.now();
}
