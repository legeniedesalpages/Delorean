#pragma once

#include "TimeScreen.h"

inline void drawDateSlash(U8G2& u8g2, int x, int y) {
  u8g2.drawLine(x - 2, y + 30, x + 6, y + 8);
  u8g2.drawLine(x - 1, y + 30, x + 7, y + 8);
}

inline void renderDateScreen(U8G2& u8g2) {
  constexpr int kDigitW = 14;
  constexpr int kGap = 12;

  u8g2.setFont(u8g2_font_6x12_tr);

  if (!timeScreenRtcAvailable()) {
    u8g2.drawStr(20, toPhysicalY(24), "ECRAN DATE");
    u8g2.drawStr(34, toPhysicalY(38), "RTC ABSENT");
    return;
  }

  const DateTime now = timeScreenRtc().now();
  const int day = now.day();
  const int month = now.month();
  const int year = now.year();

  char yearText[5];
  const int yearMod = year % 10000;
  yearText[0] = static_cast<char>('0' + ((yearMod / 1000) % 10));
  yearText[1] = static_cast<char>('0' + ((yearMod / 100) % 10));
  yearText[2] = static_cast<char>('0' + ((yearMod / 10) % 10));
  yearText[3] = static_cast<char>('0' + (yearMod % 10));
  yearText[4] = '\0';
  const int yearX = (kLogicalWidth - u8g2.getStrWidth(yearText)) / 2;
  u8g2.drawStr(yearX, toPhysicalY(12), yearText);

  const int step = kDigitW + kGap;
  const int totalWidth = step * 4;
  const int startX = ((kLogicalWidth - totalWidth) / 2) + 2;
  const int y = toPhysicalY(18);

  drawTimeDigit(u8g2, startX, y, day / 10);
  drawTimeDigit(u8g2, startX + step, y, day % 10);

  drawDateSlash(u8g2, startX + step * 2 - 3, y);

  drawTimeDigit(u8g2, startX + step * 2 + kGap, y, month / 10);
  drawTimeDigit(u8g2, startX + step * 3 + kGap, y, month % 10);
}