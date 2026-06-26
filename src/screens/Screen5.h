#pragma once

#include "../display_utils.h"

inline void renderScreen5(U8G2& u8g2) {
  u8g2.setFont(u8g2_font_logisoso42_tn);
  drawCenteredNumber(u8g2, 5);
}