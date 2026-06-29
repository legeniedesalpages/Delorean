#include <Arduino.h>
#include <U8g2lib.h>
#include "GpsService.h"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);
  u8g2.begin();
  u8g2.setFont(u8g2_font_10x20_tr);
  initGps();
}

void loop() {

  updateSpeed();

  u8g2.firstPage();
  do {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "Vitesse: %d", gpsSpeedKph());
    u8g2.drawStr(20, 40, buffer);
  } while (u8g2.nextPage());
}