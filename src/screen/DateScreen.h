#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <RTClib.h>

#include "service/DigitService.h"
#include "service/ButtonService.h"
#include "service/TwoDigitScreenService.h"


void setRtcDate(uint8_t day, uint8_t month) {
    const DateTime now = rtc().now();
    rtc().adjust(DateTime(now.year(), month, day, now.hour(), now.minute(), now.second()));
}

inline void drawDateSlash(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x - 2, y + 24, x + 6, y - 4);
  u8g2.drawLine(x - 1, y + 25, x + 7, y - 5);
  u8g2.drawLine(x - 0, y + 24, x + 8, y - 4);
}

inline bool renderDateScreen(U8G2 &u8g2, ButtonPressed button) {
  const DateTime now = rtc().now();
  drawDateSlash(u8g2, digitStartX + step * 2 -3, 35);
  return renderTwoDigitScreen(u8g2, button, now.day(), now.month(), 31, 12, setRtcDate);
}