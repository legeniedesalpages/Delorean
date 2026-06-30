#pragma once

#include <Arduino.h>

constexpr uint8_t leftButtonPin = 7;
constexpr uint8_t rightButtonPin = 8;
constexpr uint32_t LONG_PRESS_MS = 2500;

enum ButtonPressed : uint8_t
{
  NONE = 0,
  RIGHT,
  LEFT,
  BOTH,
  RIGHT_LONG,
  LEFT_LONG
};

struct ButtonState
{
  uint8_t pin;
  bool pressed;
  bool ignoreNextRelease;
  bool ignoreNextLong;
  uint32_t lastPressTime;
};

ButtonState leftButton = {leftButtonPin, false, false, false, 0};
ButtonState rightButton = {rightButtonPin, false, false, false, 0};

inline void buttonSetup() {
  pinMode(leftButtonPin, INPUT_PULLUP);
  pinMode(rightButtonPin, INPUT_PULLUP);
}

inline ButtonPressed processButton(ButtonState &button, ButtonState &other, ButtonPressed shortEvent, ButtonPressed longEvent) {
  if (button.pressed) {

    if (!button.ignoreNextLong && millis() - button.lastPressTime >= LONG_PRESS_MS) {
      button.pressed = false;
      button.ignoreNextRelease = true;
      button.ignoreNextLong = true;
      return longEvent;
    }

    if (digitalRead(button.pin) == HIGH) {

      button.pressed = false;
      button.ignoreNextLong = false;

      if (!button.ignoreNextRelease) {
        return shortEvent;
      }

      button.ignoreNextRelease = false;
    }
  } else {

    if (digitalRead(button.pin) == LOW){

      button.pressed = true;
      button.lastPressTime = millis();

      if (other.pressed){
        button.ignoreNextRelease = true;
        other.ignoreNextRelease = true;
        return BOTH;
      }
    }
  }

  return NONE;
}

inline ButtonPressed buttonStatus(){
  ButtonPressed event = processButton(leftButton, rightButton, LEFT, LEFT_LONG);
  if (event != NONE) {
    return event;
  }
  return processButton(rightButton, leftButton, RIGHT, RIGHT_LONG);
}