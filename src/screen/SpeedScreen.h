#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "service/DigitService.h"

const int speedStartX = 10;

inline void renderSpeedScreen(U8G2 &u8g2, ButtonPressed btn) {

  int16_t speed = gpsSpeedKph();

  if (speed < 0) {
    drawNegativeSymbol(u8g2, speedStartX, Y_POSITION);
    drawNegativeSymbol(u8g2, speedStartX + step, Y_POSITION);
    drawNegativeSymbol(u8g2, speedStartX + step * 2, Y_POSITION);

  } else {
    if (speed >= 100) {
      drawTimeDigit(u8g2, speedStartX, Y_POSITION, speed / 100);
    }
    if (speed >= 10) {
      drawTimeDigit(u8g2, speedStartX + step, Y_POSITION, (speed / 10) % 10);
    }
    drawTimeDigit(u8g2, speedStartX + step * 2, Y_POSITION, speed % 10);
  }

  u8g2.drawStr(85, 63, "Km/h");
}