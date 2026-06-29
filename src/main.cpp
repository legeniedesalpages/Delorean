#include <Arduino.h>
#include <U8g2lib.h>

#include "display_utils.h"
#include "screens/ArkanoidScreen.h"
#include "screens/TimeScreen.h"
#include "screens/SpeedScreen.h"
#include "screens/TemperatureScreen.h"
#include "screens/TripTimeScreen.h"
#include "screens/DateScreen.h"
#include "screens/GpsCreen.h"

// SH1106 128x64 display over I2C on Arduino Uno.
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

constexpr uint8_t kNextButtonPin = 8;
constexpr uint8_t kPreviousButtonPin = 7;
constexpr uint16_t kDebounceMs = 30;

enum AppMode : uint8_t {
  MODE_SCREENS = 0,
  MODE_GAME
};

struct ButtonState {
  uint8_t pin;
  bool stableLevel;
  bool lastRawLevel;
  unsigned long lastChangeMs;
  bool pressedEvent;
  bool releasedEvent;
};

ButtonState nextButton = {kNextButtonPin, HIGH, HIGH, 0, false, false};
ButtonState previousButton = {kPreviousButtonPin, HIGH, HIGH, 0, false, false};
uint8_t currentScreen = 0;
AppMode currentMode = MODE_SCREENS;
bool comboLatched = false;
bool suppressReleaseActions = false;
ArkanoidGameState arkanoidGame;

static void drawStartupFrameSimple(int leftX, int rightX, int underlineX, uint8_t underlineWidth, bool showDmc, int dmcX) {
  const int titleY = toPhysicalY(31);
  const int underlineY = toPhysicalY(38);
  const int dmcY = toPhysicalY(52);

  u8g2.setFont(u8g2_font_8x13B_tr);
  // Faux bold + slightly bigger look for title.
  u8g2.drawStr(leftX, titleY, "D E L O ");
  u8g2.drawStr(leftX + 1, titleY, "D E L O ");
  u8g2.drawStr(rightX, titleY, "R E A N");
  u8g2.drawStr(rightX + 1, titleY, "R E A N");

  if (underlineWidth > 0) {
    u8g2.drawHLine(underlineX, underlineY, underlineWidth);
  }

  if (showDmc) {
    // Slightly lighter than title with a pseudo-italic offset.
    u8g2.setFont(u8g2_font_6x12_tr);
    u8g2.drawStr(dmcX, dmcY, "DMC-12");
  }
}

static void playStartupAnimation() {
  u8g2.setFont(u8g2_font_8x13B_tr);
  const int leftWordWidth = u8g2.getStrWidth("D E L O ");
  const int titleWidth = u8g2.getStrWidth("D E L O R E A N");
  const int finalLeftX = (kLogicalWidth - titleWidth) / 2;
  const int finalRightX = finalLeftX + leftWordWidth;

  u8g2.setFont(u8g2_font_6x12_tr);
  const int dmcX = (kLogicalWidth - u8g2.getStrWidth("DMC-12")) / 2;

  const int leftStartX = -30;
  const int rightStartX = 128;
  const uint8_t mergeFrames = 14;

  // DELO from left, REAN from right.
  for (uint8_t frame = 0; frame < mergeFrames; ++frame) {
    const int leftX = leftStartX + ((finalLeftX - leftStartX) * frame) / (mergeFrames - 1);
    const int rightX = rightStartX + ((finalRightX - rightStartX) * frame) / (mergeFrames - 1);
    u8g2.firstPage();
    do {
      drawStartupFrameSimple(leftX, rightX, finalLeftX, 0, false, dmcX);
    } while (u8g2.nextPage());
    delay(20);
  }

  // Underline appears progressively under DELOREAN.
  for (uint8_t width = 0; width <= titleWidth; width += 9) {
    u8g2.firstPage();
    do {
      drawStartupFrameSimple(finalLeftX, finalRightX, finalLeftX, width, false, dmcX);
    } while (u8g2.nextPage());
    delay(10);
  }

  // Final title card with DMC-12.
  u8g2.firstPage();
  do {
    drawStartupFrameSimple(finalLeftX, finalRightX, finalLeftX, titleWidth, true, dmcX);
  } while (u8g2.nextPage());
  delay(3320);
}

static void updateButton(ButtonState& button, unsigned long nowMs) {
  const bool rawLevel = digitalRead(button.pin);
  button.pressedEvent = false;
  button.releasedEvent = false;

  if (rawLevel != button.lastRawLevel) {
    button.lastRawLevel = rawLevel;
    button.lastChangeMs = nowMs;
  }

  if ((nowMs - button.lastChangeMs) < kDebounceMs) {
    return;
  }

  if (rawLevel != button.stableLevel) {
    const bool previousStableLevel = button.stableLevel;
    button.stableLevel = rawLevel;

    if (previousStableLevel == HIGH && button.stableLevel == LOW) {
      button.pressedEvent = true;
    }

    if (previousStableLevel == LOW && button.stableLevel == HIGH) {
      button.releasedEvent = true;
    }
  }
}

static bool isPressed(const ButtonState& button) {
  return button.stableLevel == LOW;
}

static void toggleTimeAndSpeedScreens() {
  if (currentScreen == 0) {
    currentScreen = 1;
  } else {
    currentScreen = 0;
  }
}

static void cycleScreensThreeToFive() {
  if (currentScreen < 2 || currentScreen > 5) {
    currentScreen = 2;
    return;
  }

  currentScreen = (currentScreen == 5) ? 2 : (currentScreen + 1);
}

static void enterGame() {
  currentMode = MODE_GAME;
  comboLatched = true;
  suppressReleaseActions = true;
  resetArkanoidGame(arkanoidGame);
}

static void leaveGame() {
  currentMode = MODE_SCREENS;
  comboLatched = true;
  suppressReleaseActions = true;
}

static void handleTwoButtonCombo() {
  if (isPressed(nextButton) && isPressed(previousButton)) {
    if (!comboLatched) {
      if (currentMode == MODE_SCREENS) {
        enterGame();
      } else {
        leaveGame();
      }
    }
    return;
  }

  comboLatched = false;
  if (!isPressed(nextButton) && !isPressed(previousButton)) {
    suppressReleaseActions = false;
  }
}

static void renderActiveScreen() {
  switch (currentScreen) {
    case 0:
      renderTimeScreen(u8g2);
      break;
    case 1:
      renderSpeedScreen(u8g2);
      break;
    case 2:
      renderTemperatureScreen(u8g2);
      break;
    case 3:
      renderTripTimeScreen(u8g2);
      break;
    case 4:
      renderDateScreen(u8g2);
      break;
    case 5:
      renderGpsCreen(u8g2);
      break;
    default:
      renderTimeScreen(u8g2);
      break;
  }
}

void setup() {
  pinMode(kNextButtonPin, INPUT_PULLUP);
  pinMode(kPreviousButtonPin, INPUT_PULLUP);
  u8g2.begin();
  playStartupAnimation();
  initTimeScreen();
  initTemperatureScreen();
  initTripTimeScreen();
  initGpsCreen();
  resetArkanoidGame(arkanoidGame);
}

void loop() {
  const unsigned long nowMs = millis();
  const bool gpsDataScreenActive = (currentMode == MODE_SCREENS && (currentScreen == 1 || currentScreen == 5));

  setGpsCreenEnabled(gpsDataScreenActive);

  if (gpsDataScreenActive) {
    updateGpsCreen(nowMs);
  }

  updateButton(nextButton, nowMs);
  updateButton(previousButton, nowMs);
  handleTwoButtonCombo();

  if (currentMode == MODE_GAME) {
    updateArkanoidGame(
      arkanoidGame,
      isPressed(previousButton) && !isPressed(nextButton),
      isPressed(nextButton) && !isPressed(previousButton),
      nowMs
    );
    u8g2.firstPage();
    do {
      renderArkanoidGame(u8g2, arkanoidGame);
    } while (u8g2.nextPage());
    return;
  }

  if (!suppressReleaseActions && previousButton.releasedEvent) {
    toggleTimeAndSpeedScreens();
  }

  if (!suppressReleaseActions && nextButton.releasedEvent) {
    cycleScreensThreeToFive();
  }

  u8g2.firstPage();
  do {
    renderActiveScreen();
  } while (u8g2.nextPage());
}