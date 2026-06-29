#include <Arduino.h>
#include <U8g2lib.h>
#include "GpsService.h"
#include "ButtonService.h"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

void setup() {
  Serial.begin(115200);

  u8g2.begin();
  u8g2.setFont(u8g2_font_10x20_tr);

  initGps();

  buttonSetup();
}

void loop() {

  updateSpeed();
  ButtonPressed btn = buttonStatus();
  if (btn == ButtonPressed::LEFT) {
      Serial.println("LEFT");
    } else if (btn == ButtonPressed::RIGHT) {
      Serial.println("RIGHT");
    } else if (btn == ButtonPressed::BOTH) {
      Serial.println("BOTH");
    } else if (btn == ButtonPressed::LEFT_LONG) {
      Serial.println("LEFT LONG");
    } else if (btn == ButtonPressed::RIGHT_LONG) {
      Serial.println("RIGHT LONG");
    }

  u8g2.firstPage();
  do {
    char buffer[20];
    snprintf(buffer, sizeof(buffer), "Vitesse: %d", gpsSpeedKph());
    u8g2.drawStr(20, 35, buffer);

    
  } while (u8g2.nextPage());
}