#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <RTClib.h>

#include "service/DigitService.h"
#include "service/ButtonService.h"

const int kDigitW = 14;
const int kGap = 12;
const int startX = 10;
const int y = 26;
const int step = kDigitW + kGap;

inline RTC_DS3231& rtc() { static RTC_DS3231 rtc; return rtc; }
inline int& hour() { static int hour; return hour; }
inline int& minute() { static int minute; return minute; }

inline bool& edititionMode() { static bool edititionMode; return edititionMode; }
inline bool& editHour() { static bool editHour; return editHour; }
inline bool& ignoreNextButton() { static bool ignoreNextButton; return ignoreNextButton; }

inline void initTimeScreen() {
  rtc().begin();
  edititionMode() = false;
}

void drawTimeColon(U8G2& u8g2, int x, int y) {
  u8g2.drawDisc(x, y - 10, 2, U8G2_DRAW_ALL);
  u8g2.drawDisc(x, y + 10, 2, U8G2_DRAW_ALL);
}

inline bool renderTimeScreen(U8G2 &u8g2, ButtonPressed btn) {

  bool blink;
  if (btn == ButtonPressed::LEFT_LONG) {
    edititionMode() = true;
    editHour() = true;
    ignoreNextButton() = false;
  }

  if (btn == ButtonPressed::NONE) {
    ignoreNextButton() = false;
  }

  if (edititionMode()) {

    if (millis() % 1000 < 200) {
      blink = true;
    } else {
      blink = false;
    }

    if (btn == ButtonPressed::RIGHT && !ignoreNextButton()) {

      if (editHour()) {
        editHour() = false;
      } else {
        DateTime now = rtc().now();
        rtc().adjust(DateTime(now.year(), now.month(), now.day(), hour(), minute(), 0));
        edititionMode() = false;
      }

      ignoreNextButton() = true;

    } else if (btn == ButtonPressed::LEFT && !ignoreNextButton()) {
      if (editHour()) {
        hour()++;
        if (hour() > 23) hour() = 0;
      } else {
        minute()++;
        if (minute() > 59) minute() = 0;
      }

      ignoreNextButton() = true;
    } 

  } else {
    const DateTime now = rtc().now();
    hour() = now.hour();
    minute() = now.minute();

    if ((rtc().now().second() % 2) == 0) {
      drawTimeColon(u8g2, startX + step * 2, 45);
    }
  }

  if (!(edititionMode() && editHour() && blink)) {
    drawTimeDigit(u8g2, startX, y, hour() / 10);
    drawTimeDigit(u8g2, startX + step, y, hour() % 10);
  }

  if (!(edititionMode() && !editHour() && blink)) {
    drawTimeDigit(u8g2, startX + step * 2 + kGap, y, minute() / 10);
    drawTimeDigit(u8g2, startX + step * 3 + kGap, y, minute() % 10);
  }
  
  return edititionMode();
}

