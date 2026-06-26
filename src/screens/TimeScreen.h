#pragma once

#include <RTClib.h>
#include <Wire.h>

#include "../display_utils.h"

inline RTC_DS3231& timeScreenRtc() {
  static RTC_DS3231 rtc;
  return rtc;
}

inline bool& timeScreenRtcAvailable() {
  static bool available = false;
  return available;
}

inline void initTimeScreen() {
  Wire.begin();

  RTC_DS3231& rtc = timeScreenRtc();
  bool& available = timeScreenRtcAvailable();
  available = rtc.begin();

  if (available && rtc.lostPower()) {
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
}

inline void drawTimeSegA(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x + 2, y, x + 14, y);
  u8g2.drawLine(x + 0, y + 1, x + 14, y + 1);
  u8g2.drawLine(x + 0, y + 2, x + 13, y + 2);
}

inline void drawTimeSegB(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x + 16, y + 2, x + 14, y + 15);
  u8g2.drawLine(x + 17, y + 1, x + 15, y + 16);
  u8g2.drawLine(x + 18, y + 2, x + 16, y + 15);
}

inline void drawTimeSegC(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x + 13, y + 22, x + 11, y + 33);
  u8g2.drawLine(x + 14, y + 21, x + 12, y + 34);
  u8g2.drawLine(x + 15, y + 22, x + 13, y + 35);
}

inline void drawTimeSegD(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x - 4, y + 33, x - 3, y + 33);
  u8g2.drawLine(x - 5, y + 34, x - 3, y + 34);
  u8g2.drawLine(x - 5, y + 35, x + 9, y + 35);
  u8g2.drawLine(x - 5, y + 36, x + 10, y + 36);
  u8g2.drawLine(x - 4, y + 37, x + 11, y + 37);
}

inline void drawTimeSegE(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x - 3, y + 22, x - 5, y + 31);
  u8g2.drawLine(x - 2, y + 21, x - 4, y + 30);
  u8g2.drawLine(x - 1, y + 22, x - 3, y + 29);
}

inline void drawTimeSegF(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x + 1, y + 5, x - 1, y + 15);
  u8g2.drawLine(x + 0, y + 5, x - 2, y + 16);
  u8g2.drawLine(x - 1, y + 5, x - 3, y + 15);
}

inline void drawTimeSegG(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x - 0, y + 17, x + 12, y + 17);
  u8g2.drawLine(x - 1, y + 18, x + 13, y + 18);
  u8g2.drawLine(x - 0, y + 19, x + 12, y + 19);
}

inline void drawTimeDigit(U8G2& u8g2, int x, int y, int value) {
  bool segments[7] = {false, false, false, false, false, false, false};

  switch (value) {
    case 0: segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[5] = true; break;
    case 1: segments[1] = true; segments[2] = true; break;
    case 2: segments[0] = true; segments[1] = true; segments[3] = true; segments[4] = true; segments[6] = true; break;
    case 3: segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true; segments[6] = true; break;
    case 4: segments[1] = true; segments[2] = true; segments[5] = true; segments[6] = true; break;
    case 5: segments[0] = true; segments[2] = true; segments[3] = true; segments[5] = true; segments[6] = true; break;
    case 6: segments[0] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[5] = true; segments[6] = true; break;
    case 7: segments[0] = true; segments[1] = true; segments[2] = true; break;
    case 8: segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true; segments[4] = true; segments[5] = true; segments[6] = true; break;
    case 9: segments[0] = true; segments[1] = true; segments[2] = true; segments[3] = true; segments[5] = true; segments[6] = true; break;
    default: break;
  }

  if (segments[0]) drawTimeSegA(u8g2, x, y);
  if (segments[1]) drawTimeSegB(u8g2, x, y);
  if (segments[2]) drawTimeSegC(u8g2, x, y);
  if (segments[3]) drawTimeSegD(u8g2, x, y);
  if (segments[4]) drawTimeSegE(u8g2, x, y);
  if (segments[5]) drawTimeSegF(u8g2, x, y);
  if (segments[6]) drawTimeSegG(u8g2, x, y);
}

inline void drawTimeColon(U8G2& u8g2, int x, int y) {
  u8g2.drawDisc(x, y - 10, 2, U8G2_DRAW_ALL);
  u8g2.drawDisc(x, y + 10, 2, U8G2_DRAW_ALL);
}

inline void renderTimeScreen(U8G2& u8g2) {
  constexpr int kDigitW = 14;
  constexpr int kGap = 12;

  if (!timeScreenRtcAvailable()) {
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(18, toPhysicalY(24), "ECRAN DES HEURES");
    u8g2.drawStr(34, toPhysicalY(38), "RTC ABSENT");
    return;
  }

  const DateTime now = timeScreenRtc().now();
  const int hours = now.hour();
  const int minutes = now.minute();

  const int step = kDigitW + kGap;
  const int totalWidth = step * 4;
  const int startX = ((kLogicalWidth - totalWidth) / 2) + 2;
  const int y = toPhysicalY(18);

  drawTimeDigit(u8g2, startX, y, hours / 10);
  drawTimeDigit(u8g2, startX + step, y, hours % 10);

  if ((now.second() % 2) == 0) {
    drawTimeColon(u8g2, startX + step * 2, toPhysicalY(39));
  }

  drawTimeDigit(u8g2, startX + step * 2 + kGap, y, minutes / 10);
  drawTimeDigit(u8g2, startX + step * 3 + kGap, y, minutes % 10);
}