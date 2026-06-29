#pragma once

#include <Arduino.h>

#include "TimeScreen.h"
#include "GpsCreen.h"

inline float& speedScreenKmhValue() {
  static float kmh = 0.0f;
  return kmh;
}

inline void setSpeedScreenKmh(float kmh) {
  if (kmh < 0.0f) {
    kmh = 0.0f;
  }
  if (kmh > 999.0f) {
    kmh = 999.0f;
  }
  speedScreenKmhValue() = kmh;
}

inline void renderSpeedScreen(U8G2& u8g2) {
  constexpr int kDigitW = 14;
  constexpr int kGap = 12;
  constexpr int kUnitGap = 4;
  const char* kUnitText = "KM/H";
  const bool hasGpsSpeed = gpsSpeedAvailable();

  if (hasGpsSpeed) {
    setSpeedScreenKmh(gpsSpeedKmh());
  }

  const int speedInt = static_cast<int>(speedScreenKmhValue() + 0.5f);
  const int hundreds = speedInt / 100;
  const int tens = (speedInt / 10) % 10;
  const int ones = speedInt % 10;

  const int digitCount = hasGpsSpeed ? ((speedInt >= 100) ? 3 : ((speedInt >= 10) ? 2 : 1)) : 1;
  const int digits[3] = {hundreds, tens, ones};
  const int firstDigitIndex = 3 - digitCount;

  const int step = kDigitW + kGap;
  const int numberWidth = ((digitCount - 1) * step) + kDigitW;

  u8g2.setFont(u8g2_font_6x12_tr);
  const int unitWidth = u8g2.getStrWidth(kUnitText);
  const int totalWidth = numberWidth + kUnitGap + unitWidth;
  const int startX = ((kLogicalWidth - totalWidth) / 2) + 2;
  const int y = toPhysicalY(18);

  if (hasGpsSpeed) {
    for (int i = 0; i < digitCount; ++i) {
      drawTimeDigit(u8g2, startX + (i * step), y, digits[firstDigitIndex + i]);
    }
  } else {
    drawTimeSegG(u8g2, startX, y);
  }

  const int unitX = startX + numberWidth + kUnitGap;
  u8g2.drawStr(unitX, toPhysicalY(52), kUnitText);
}