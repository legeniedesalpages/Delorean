#pragma once

#include <Arduino.h>
#include <U8g2lib.h>
#include <DHT.h>

const int temperatureStartX = 20;
constexpr uint8_t dhtPin = 2;
constexpr uint8_t dhtType = AM2301;
inline DHT& temperatureScreenDht() { static DHT dht(dhtPin, dhtType); return dht; }

inline void initTemperatureScreen() {
  temperatureScreenDht().begin();
}

inline void renderTemperatureScreen(U8G2 &u8g2, ButtonPressed btn) {
    
    const int16_t temperature = temperatureScreenDht().readTemperature();

    if (temperature <= -10) {
        drawNegativeSymbol(u8g2, temperatureStartX, Y_POSITION);
    } else if (temperature < 0) {
        drawNegativeSymbol(u8g2, temperatureStartX + step, Y_POSITION);
    }

    const int16_t absTemperature = abs(temperature);

    if (absTemperature >= 10) {
        drawTimeDigit(u8g2, temperatureStartX + step, Y_POSITION, (absTemperature / 10) % 10);
    }
    drawTimeDigit(u8g2, temperatureStartX + step * 2, Y_POSITION, absTemperature % 10);

    u8g2.drawCircle(temperatureStartX + 78, 30, 2, U8G2_DRAW_ALL);
    u8g2.drawStr(temperatureStartX + 82, 37, "C");
}