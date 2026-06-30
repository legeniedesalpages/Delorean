#include <Arduino.h>
#include <U8g2lib.h>

#include "service/GpsService.h"
#include "service/ButtonService.h"

#include "screen/TimeScreen.h"
#include "screen/SpeedScreen.h"
#include "screen/TemperatureScreen.h"
#include "screen/DateScreen.h"
#include "screen/TripScreen.h"
#include "screen/GpsScreen.h"
#include "screen/GameScreen.h"

U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

enum Screen : uint8_t
{
  TIME,
  SPEED,
  TEMPERATURE,
  DATE,
  TRIP,
  GPS,
  GAME
};

static Screen currentScreen = Screen::TIME;
static bool overrideButton = false;

void setup() {
  Serial.begin(115200);

  buttonSetup();

  u8g2.begin();
  u8g2.setFont(u8g2_font_8x13_tr);

  initGps();
  initTimeScreen();
}

void pageChange(ButtonPressed btn) {
  
  if (btn == ButtonPressed::LEFT) {

    if (currentScreen == Screen::TIME) {
      currentScreen = Screen::SPEED; 
    } else {
      currentScreen = Screen::TIME;
    }

  } else if (btn == ButtonPressed::RIGHT) {

    if (currentScreen == Screen::TEMPERATURE) {
      currentScreen = Screen::DATE; 
    } else if (currentScreen == Screen::DATE) {
      currentScreen = Screen::TRIP;
    } else if (currentScreen == Screen::TRIP) {
      currentScreen = Screen::GPS;
    } else {
      currentScreen = Screen::TEMPERATURE;
    }

  } else if (btn == ButtonPressed::BOTH) {
    currentScreen = Screen::GAME;
  }
}

void pageRender(Screen screen, ButtonPressed btn) {
  switch (screen) {
    case Screen::TIME:
      overrideButton = renderTimeScreen(u8g2, btn);
      break;
    case Screen::SPEED:
      renderSpeedScreen(u8g2, btn);
      break;
    case Screen::TEMPERATURE:
      u8g2.drawStr(20, 35, "Temperature");
      break;
    case Screen::DATE:
      u8g2.drawStr(20, 35, "Date");
      break;
    case Screen::TRIP:
      u8g2.drawStr(20, 35, "Trip");
      break;
    case Screen::GPS:
      u8g2.drawStr(20, 35, "GPS");
      break;
    case Screen::GAME:
      u8g2.drawStr(20, 35, "Game");
      break;
  }
}

void loop() {

  updateSpeed();

  ButtonPressed btn = buttonStatus();
  if (!overrideButton) {
   pageChange(btn);
  }

  u8g2.firstPage();
  do {
    pageRender(currentScreen, btn);
  } while (u8g2.nextPage());
}