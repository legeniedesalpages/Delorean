#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <RTClib.h>

#include "service/DigitService.h"
#include "service/ButtonService.h"
#include "service/TwoDigitScreenService.h"

inline RTC_DS3231& rtc() { static RTC_DS3231 rtc; return rtc; }

inline void initTimeScreen() {
  rtc().begin();
}

void drawTimeColon(U8G2& u8g2, int x, int y) {
  u8g2.drawDisc(x, y, 2, U8G2_DRAW_ALL);
  u8g2.drawDisc(x, y + 20, 2, U8G2_DRAW_ALL);
}

void setRtcTime(uint8_t hour, uint8_t minute) {
    const DateTime now = rtc().now();
    rtc().adjust(DateTime(now.year(), now.month(), now.day(), hour, minute, 0));
}

inline bool renderTimeScreen(U8G2 &u8g2, ButtonPressed button) {

  const DateTime now = rtc().now();

  if ((now.second() % 2) == 0) {
    drawTimeColon(u8g2, digitStartX + step * 2, 35);
  }

  return renderTwoDigitScreen(u8g2, button, now.hour(), now.minute(), 23, 59, setRtcTime);
}