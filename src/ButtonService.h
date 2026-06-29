#pragma once

#include <Arduino.h>

constexpr uint8_t leftButtonPin = 7;
constexpr uint8_t rightButtonPin = 8;

enum ButtonPressed : uint8_t {
  NONE = 0,
  RIGHT = 1,
  LEFT = 2,
  BOTH =3,
  RIGHT_LONG = 4,
  LEFT_LONG = 5,
};

struct ButtonState {
  uint8_t pin;
  bool pressed;
  bool ignoreNextRelease;
  bool ignoreNextLong;
  uint32_t lastPressTime;
};

ButtonState leftButton = {leftButtonPin, false, false, false, 0};
ButtonState rightButton = {rightButtonPin, false, false, false, 0};

void buttonSetup() {
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
}

ButtonPressed buttonStatus() {

  ButtonPressed status = ButtonPressed::NONE;

  if (leftButton.pressed) {

    if (!leftButton.ignoreNextLong && (millis() - leftButton.lastPressTime > 2500)) { // 2.5 seconds threshold for long press
      leftButton.pressed = false;
      leftButton.ignoreNextRelease = true; 
      leftButton.ignoreNextLong = true;
      return ButtonPressed::LEFT_LONG;
    } 

    if (digitalRead(leftButton.pin) == HIGH) { // relachement du bouton 
      leftButton.pressed = false;
      leftButton.ignoreNextLong = false;
      if (!leftButton.ignoreNextRelease) {
        status = ButtonPressed::LEFT;
      } else {
        leftButton.ignoreNextRelease = false;
      }
    }
  } else {

    if (digitalRead(leftButton.pin) == LOW) {
      leftButton.pressed = true;
      leftButton.lastPressTime = millis();
      if (rightButton.pressed) {
        leftButton.ignoreNextRelease = true;
        rightButton.ignoreNextRelease = true;
        return ButtonPressed::BOTH;
      }
    }
  }

  if (rightButton.pressed) {

    if (!rightButton.ignoreNextLong && (millis() - rightButton.lastPressTime > 2500)) { // 2.5 seconds threshold for long press
      rightButton.pressed = false;
      rightButton.ignoreNextRelease = true; 
      rightButton.ignoreNextLong = true;
      return ButtonPressed::RIGHT_LONG;
    }

    if (digitalRead(rightButton.pin) == HIGH) { // relachement du bouton 
      rightButton.pressed = false;
      rightButton.ignoreNextLong = false;
      if (!rightButton.ignoreNextRelease) {
        status = ButtonPressed::RIGHT;
      } else {
        rightButton.ignoreNextRelease = false;
      }
    }
  } else {
    if (digitalRead(rightButton.pin) == LOW) {
      rightButton.pressed = true;
      rightButton.lastPressTime = millis();
      if (leftButton.pressed) {
        leftButton.ignoreNextRelease = true;
        rightButton.ignoreNextRelease = true;
        return ButtonPressed::BOTH;
      }
    }
  }

  return status;
}