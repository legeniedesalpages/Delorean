#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

constexpr int16_t kHiddenTopRows = 8;
constexpr int16_t kLogicalWidth = 128;
constexpr int16_t kLogicalHeight = 64 - kHiddenTopRows;

inline int16_t toPhysicalY(int16_t logicalY) {
  return logicalY + kHiddenTopRows;
}

inline void drawCenteredNumber(U8G2& u8g2, uint8_t screenNumber, int16_t logicalBaseline = 42) {
  char text[2] = {static_cast<char>('0' + screenNumber), '\0'};
  const int16_t textWidth = u8g2.getStrWidth(text);
  const int16_t x = (kLogicalWidth - textWidth) / 2;
  u8g2.drawStr(x, toPhysicalY(logicalBaseline), text);
}