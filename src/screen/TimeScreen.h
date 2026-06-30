#pragma once

#include <Arduino.h>
#include <U8g2lib.h>

#include "service/ButtonService.h"

inline void renderTimeScreen(U8G2 &u8g2, ButtonPressed btn) {
  u8g2.drawStr(20, 35, "Time");
}