#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <RTClib.h>

#include "service/DigitService.h"
#include "service/ButtonService.h"

const int digitStartX = 10;

inline uint8_t& first() { static uint8_t first; return first; }
inline uint8_t& second() { static uint8_t second; return second; }

inline bool& edititionMode() { static bool edititionMode; return edititionMode; }
inline bool& editFirst() { static bool editFirst; return editFirst; }
inline bool& ignoreNextButton() { static bool ignoreNextButton; return ignoreNextButton; }

inline bool renderTwoDigitScreen(U8G2 &u8g2, ButtonPressed btn, uint8_t firstValue, uint8_t secondValue,uint8_t maxFirst, uint8_t maxSecond, void (*onValidate)(uint8_t, uint8_t)) {

  bool blink = (millis() % 1000) < 200;
  if (btn == ButtonPressed::LEFT_LONG) {
    edititionMode() = true;
    editFirst() = true;
    ignoreNextButton() = false;
  }

  if (btn == ButtonPressed::NONE) {
    ignoreNextButton() = false;
  }

  if (edititionMode()) {

    if (btn == ButtonPressed::RIGHT && !ignoreNextButton()) {

      if (editFirst()) {
        editFirst() = false;
      } else {
        onValidate(first(), second());
        edititionMode() = false;
      }

      ignoreNextButton() = true;

    } else if (btn == ButtonPressed::LEFT && !ignoreNextButton()) {
      if (editFirst()) {
        first()++;
        if (first() > maxFirst) first() = 0;
      } else {
        second()++;
        if (second() > maxSecond) second() = 0;
      }

      ignoreNextButton() = true;
    } 

  } else {

    first() = firstValue;
    second() = secondValue;
    
  }

  if (!(edititionMode() && editFirst() && blink)) {
    drawTimeDigit(u8g2, digitStartX, Y_POSITION, first() / 10);
    drawTimeDigit(u8g2, digitStartX + step, Y_POSITION, first() % 10);
  }

  if (!(edititionMode() && !editFirst() && blink)) {
    drawTimeDigit(u8g2, digitStartX + step * 2 + gap, Y_POSITION, second() / 10);
    drawTimeDigit(u8g2, digitStartX + step * 3 + gap, Y_POSITION, second() % 10);
  }
  
  return edititionMode();
}

