#pragma once

#include <Arduino.h>
#include <NMEAGPS.h>
#include <NeoSWSerial.h>

inline int16_t& gpsSpeedKph() { static int16_t speedKph = -1; return speedKph; }
inline uint32_t& lastValidSpeedTime() { static uint32_t t = 0; return t; }

inline NeoSWSerial& gpsSerial() { static NeoSWSerial gps(4, 5); return gps; }
inline NMEAGPS& gpsParser() { static NMEAGPS parser; return parser; }

inline void handleGpsRxCharForParser(uint8_t c) { gpsParser().handle(c); }

inline void initGps() {
  Serial.println("Init GPS Service");
  NeoSWSerial& gps = gpsSerial();
  gps.begin(9600);
  gps.attachInterrupt(handleGpsRxCharForParser);
  gps.listen();
  Serial.println("Listening GPS Service");
}

inline void updateSpeed() {
    NMEAGPS& parser = gpsParser();

    if (parser.available()) {
        gps_fix fix = parser.read();
        if (fix.valid.speed) {
            int16_t& speedKph = gpsSpeedKph();
            speedKph = fix.speed_kph();
            lastValidSpeedTime() = millis();
            return;
        }
    }

    // If no valid speed was received, check if the last valid speed is too old
    if (gpsSpeedKph() != -1 && (millis() - lastValidSpeedTime() > 5000)) { // 5 seconds threshold
        gpsSpeedKph() = -1;
    }
}

