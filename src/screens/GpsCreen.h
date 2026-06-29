#pragma once

#include <Arduino.h>
#include <NeoSWSerial.h>

#include "../display_utils.h"

inline NeoSWSerial& gpsSerialPort() {
  // D4 = RX (from GPS TX), D5 = TX (to GPS RX)
  static NeoSWSerial port(4, 5);
  return port;
}

inline bool& gpsSerialBridgeStarted() {
  static bool started = false;
  return started;
}

inline bool& gpsSerialBridgeListening() {
  static bool listening = false;
  return listening;
}

inline void initGpsSerialMonitorBridge() {
  Serial.begin(115200);
  Serial.println(F("GPS NMEA bridge active"));
}

inline void setGpsSerialMonitorBridgeEnabled(bool enabled) {
  NeoSWSerial& gps = gpsSerialPort();

  if (enabled) {
    if (!gpsSerialBridgeStarted()) {
      gps.begin(9600);
      gpsSerialBridgeStarted() = true;
    }
    if (!gpsSerialBridgeListening()) {
      gps.listen();
      gpsSerialBridgeListening() = true;
    }
    return;
  }

  if (gpsSerialBridgeListening()) {
    gps.end();
    gpsSerialBridgeListening() = false;
    gpsSerialBridgeStarted() = false;
  }
}

inline void pumpGpsNmeaToSerialMonitor() {
  NeoSWSerial& gps = gpsSerialPort();
  while (gps.available() > 0) {
    Serial.write(gps.read());
  }
}

inline void renderGpsCreen(U8G2& u8g2) {
  u8g2.setFont(u8g2_font_logisoso42_tn);
  drawCenteredNumber(u8g2, 5);
}