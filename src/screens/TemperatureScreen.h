#pragma once

#include <Arduino.h>
#include <DHT.h>

#include "TimeScreen.h"

constexpr uint8_t kDhtPin = 2;
constexpr uint8_t kDhtType = DHT22;
constexpr unsigned long kDhtRefreshMs = 2000UL;

struct TemperatureScreenState {
  bool initialized;
  bool hasTemperature;
  bool hasHumidity;
  float temperatureC;
  float humidityPct;
  unsigned long lastReadMs;
};

inline DHT& temperatureScreenDht() {
  static DHT dht(kDhtPin, kDhtType);
  return dht;
}

inline TemperatureScreenState& temperatureScreenState() {
  static TemperatureScreenState state = {false, false, false, 0.0f, 0.0f, 0};
  return state;
}

inline void initTemperatureScreen() {
  TemperatureScreenState& state = temperatureScreenState();
  if (state.initialized) {
    return;
  }

  temperatureScreenDht().begin();
  state.initialized = true;
}

inline void updateTemperatureScreen(unsigned long nowMs) {
  TemperatureScreenState& state = temperatureScreenState();
  if ((nowMs - state.lastReadMs) < kDhtRefreshMs) {
    return;
  }

  state.lastReadMs = nowMs;
  const float temperature = temperatureScreenDht().readTemperature();
  const float humidity = temperatureScreenDht().readHumidity();

  state.hasTemperature = !isnan(temperature);
  state.hasHumidity = !isnan(humidity);

  if (state.hasTemperature) {
    state.temperatureC = temperature;
  }
  if (state.hasHumidity) {
    state.humidityPct = humidity;
  }
}

inline void drawTemperatureUnitTopAligned(U8G2& u8g2, int x, int digitTopPhysicalY) {
  const int degreeCenterX = x + 2;
  const int degreeCenterY = digitTopPhysicalY + 4;
  u8g2.drawCircle(degreeCenterX, degreeCenterY, 2, U8G2_DRAW_ALL);
  u8g2.drawStr(x + 8, digitTopPhysicalY + 12, "C");
}

inline void renderTemperatureScreen(U8G2& u8g2) {
  updateTemperatureScreen(millis());

  constexpr int kDigitW = 14;
  constexpr int kGap = 12;
  constexpr int kUnitGap = 4;
  constexpr int kTempLogicalY = 18;

  TemperatureScreenState& state = temperatureScreenState();

  char humidityText[20];
  u8g2.setFont(u8g2_font_6x12_tr);
  if (state.hasHumidity) {
    int humidityInt = static_cast<int>(state.humidityPct + 0.5f);
    if (humidityInt < 0) {
      humidityInt = 0;
    }
    if (humidityInt > 100) {
      humidityInt = 100;
    }

    char* p = humidityText;
    *p++ = 'h';
    *p++ = ':';
    if (humidityInt >= 100) {
      *p++ = '1';
      *p++ = '0';
      *p++ = '0';
    } else if (humidityInt >= 10) {
      *p++ = static_cast<char>('0' + (humidityInt / 10));
      *p++ = static_cast<char>('0' + (humidityInt % 10));
    } else {
      *p++ = static_cast<char>('0' + humidityInt);
    }
    *p++ = '%';
    *p = '\0';
  } else {
    humidityText[0] = 'h';
    humidityText[1] = ':';
    humidityText[2] = '-';
    humidityText[3] = '-';
    humidityText[4] = '%';
    humidityText[5] = '\0';
  }
  const int humidityX = kLogicalWidth - u8g2.getStrWidth(humidityText) - 2;
  u8g2.drawStr(humidityX, toPhysicalY(12), humidityText);

  const int step = kDigitW + kGap;
  const int y = toPhysicalY(kTempLogicalY);

  if (!state.hasTemperature) {
    const int unitWidth = u8g2.getStrWidth("C") + 10;
    const int totalWidth = kDigitW + kUnitGap + unitWidth;
    const int startX = ((kLogicalWidth - totalWidth) / 2) + 2;
    drawTimeSegG(u8g2, startX, y);
    drawTemperatureUnitTopAligned(u8g2, startX + kDigitW + kUnitGap, y);
    return;
  }

  float tempValue = state.temperatureC;
  if (tempValue > 99.0f) {
    tempValue = 99.0f;
  }
  if (tempValue < -99.0f) {
    tempValue = -99.0f;
  }

  const int tempInt = static_cast<int>(tempValue + ((tempValue >= 0.0f) ? 0.5f : -0.5f));

  const bool negative = tempInt < 0;
  const int absTemp = negative ? -tempInt : tempInt;
  const int tens = (absTemp / 10) % 10;
  const int ones = absTemp % 10;
  const bool hasTens = absTemp >= 10;

  int glyphCount = 1;
  if (hasTens) {
    glyphCount += 1;
  }
  if (negative) {
    glyphCount += 1;
  }

  const int numberWidth = ((glyphCount - 1) * step) + kDigitW;
  const int unitWidth = u8g2.getStrWidth("C") + 10;
  const int totalWidth = numberWidth + kUnitGap + unitWidth;
  const int startX = ((kLogicalWidth - totalWidth) / 2) + 2;

  int glyphIndex = 0;
  if (negative) {
    drawTimeSegG(u8g2, startX + (glyphIndex * step), y);
    glyphIndex += 1;
  }

  if (hasTens) {
    drawTimeDigit(u8g2, startX + (glyphIndex * step), y, tens);
    glyphIndex += 1;
  }

  drawTimeDigit(u8g2, startX + (glyphIndex * step), y, ones);

  drawTemperatureUnitTopAligned(u8g2, startX + numberWidth + kUnitGap + 5, y);
}