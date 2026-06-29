#include <Arduino.h>
#include <U8g2lib.h>

#include "display_utils.h"
#include "screens/ArkanoidScreen.h"
#include "screens/TimeScreen.h"
#include "screens/SpeedScreen.h"
#include "screens/Screen3.h"
#include "screens/Screen4.h"
#include "screens/GpsCreen.h"

// SH1106 128x64 display over I2C on Arduino Uno.
U8G2_SH1106_128X64_NONAME_1_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

constexpr uint8_t kNextButtonPin = 8;
constexpr uint8_t kPreviousButtonPin = 7;
constexpr uint8_t kScreenCount = 5;
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

static void goToNextScreen() {
  currentScreen = (currentScreen + 1) % kScreenCount;
}

static void goToPreviousScreen() {
  currentScreen = (currentScreen + kScreenCount - 1) % kScreenCount;
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
      renderScreen3(u8g2);
      break;
    case 3:
      renderScreen4(u8g2);
      break;
    case 4:
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
  initTimeScreen();
  initGpsCreen();
  resetArkanoidGame(arkanoidGame);
}

void loop() {
  const unsigned long nowMs = millis();
  const bool gpsScreenActive = (currentMode == MODE_SCREENS && currentScreen == 4);

  setGpsCreenEnabled(gpsScreenActive);

  if (gpsScreenActive) {
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

  if (!suppressReleaseActions && nextButton.releasedEvent) {
    goToNextScreen();
  }

  if (!suppressReleaseActions && previousButton.releasedEvent) {
    goToPreviousScreen();
  }

  u8g2.firstPage();
  do {
    renderActiveScreen();
  } while (u8g2.nextPage());
}