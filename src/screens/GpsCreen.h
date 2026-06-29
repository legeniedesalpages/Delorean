#pragma once

#include <Arduino.h>

#ifdef SECONDS_PER_DAY
#undef SECONDS_PER_DAY
#endif

#include <NMEAGPS.h>
#include <NeoSWSerial.h>

#include "../display_utils.h"

struct GpsScreenState {
  bool serialStarted;
  bool active;
  bool hasSignal;
  bool hasFix;
  bool hasLocation;
  bool hasSatellites;
  bool hasSpeed;
  float latitude;
  float longitude;
  float speedKmh;
  uint8_t satellites;
  uint16_t isrChars;
  uint16_t lastObservedIsrChars;
  unsigned long lastByteMs;
  unsigned long lastSentenceMs;
};

inline GpsScreenState& gpsScreenState() {
  static GpsScreenState state = {false, false, false, false, false, false, false, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0};
  return state;
}

inline NMEAGPS& gpsParser();

inline volatile uint16_t& gpsIsrCharCount() {
  static volatile uint16_t count = 0;
  return count;
}

inline void handleGpsRxCharForParser(uint8_t c) {
  volatile uint16_t& count = gpsIsrCharCount();
  if (count < 9999) {
    count += 1;
  }
  gpsParser().handle(c);
}

inline NMEAGPS& gpsParser() {
  static NMEAGPS parser;
  return parser;
}

inline NeoSWSerial& gpsSerialPort() {
  // D4 = RX (from GPS TX), D5 = TX (to GPS RX)
  static NeoSWSerial port(4, 5);
  return port;
}

inline void initGpsCreen() {
  GpsScreenState& state = gpsScreenState();
  state = {false, false, false, false, false, false, false, 0.0f, 0.0f, 0.0f, 0, 0, 0, 0, 0};
  gpsIsrCharCount() = 0;
}

inline bool gpsSpeedAvailable() {
  return gpsScreenState().hasSpeed;
}

inline float gpsSpeedKmh() {
  return gpsScreenState().speedKmh;
}

inline void setGpsCreenEnabled(bool enabled) {
  GpsScreenState& state = gpsScreenState();
  NeoSWSerial& gps = gpsSerialPort();

  if (enabled) {
    if (!state.serialStarted) {
      gps.attachInterrupt(handleGpsRxCharForParser);
      gps.begin(9600);
      state.serialStarted = true;
    }
    gps.listen();
    state.active = true;
    return;
  }

  if (state.active) {
    gps.detachInterrupt();
    gps.end();
    state.active = false;
    state.serialStarted = false;
  }
}

inline void updateGpsCreen(unsigned long nowMs) {
  GpsScreenState& state = gpsScreenState();
  NMEAGPS& parser = gpsParser();
  const uint16_t currentIsrChars = gpsIsrCharCount();
  state.isrChars = currentIsrChars;

  if (currentIsrChars != state.lastObservedIsrChars) {
    state.lastByteMs = nowMs;
    state.hasSignal = true;
    state.lastObservedIsrChars = currentIsrChars;
  }

  while (parser.available()) {
    const gps_fix fix = parser.read();
    state.lastSentenceMs = nowMs;
    state.hasLocation = fix.valid.location;
    state.hasFix = fix.valid.location;
    state.hasSatellites = fix.valid.satellites;
    state.hasSpeed = fix.valid.speed;

    if (fix.valid.location) {
      state.latitude = fix.latitude();
      state.longitude = fix.longitude();
    }

    if (fix.valid.speed) {
      state.speedKmh = fix.speed_kph();
    }

    if (fix.valid.satellites) {
      state.satellites = fix.satellites;
    }
  }

  if (state.hasSignal && (nowMs - state.lastByteMs) > 4000UL) {
    state.hasSignal = false;
    state.hasFix = false;
    state.hasSpeed = false;
  }
}

inline void renderGpsCreen(U8G2& u8g2) {
  GpsScreenState& state = gpsScreenState();
  char line[28];
  char latitudeText[12];
  char longitudeText[12];

  u8g2.setFont(u8g2_font_6x12_tr);

  if (!state.hasSignal) {
    u8g2.drawStr(2, toPhysicalY(30), "Statut: PAS DE SIGNAL");
  } else if (state.hasFix) {
    u8g2.drawStr(2, toPhysicalY(30), "Statut: GPS OK");
  } else {
    u8g2.drawStr(2, toPhysicalY(30), "Statut: RECHERCHE...");
  }

  if (state.hasSatellites) {
    char* p = line;
    const char* prefix = "Satellites: ";
    while (*prefix != '\0') {
      *p++ = *prefix++;
    }

    uint8_t sat = state.satellites;
    if (sat >= 100) {
      *p++ = static_cast<char>('0' + (sat / 100));
      sat %= 100;
      *p++ = static_cast<char>('0' + (sat / 10));
      *p++ = static_cast<char>('0' + (sat % 10));
    } else if (sat >= 10) {
      *p++ = static_cast<char>('0' + (sat / 10));
      *p++ = static_cast<char>('0' + (sat % 10));
    } else {
      *p++ = static_cast<char>('0' + sat);
    }
    *p = '\0';
  } else {
    const char* txt = "Satellites: --";
    uint8_t i = 0;
    while (txt[i] != '\0' && i < (sizeof(line) - 1)) {
      line[i] = txt[i];
      ++i;
    }
    line[i] = '\0';
  }
  u8g2.drawStr(2, toPhysicalY(42), line);

  if (state.hasLocation) {
    dtostrf(state.latitude, 0, 2, latitudeText);
    dtostrf(state.longitude, 0, 2, longitudeText);
    char* p = line;
    const char* prefix = "Position: ";
    while (*prefix != '\0' && (p - line) < static_cast<int>(sizeof(line) - 1)) {
      *p++ = *prefix++;
    }
    const char* lat = latitudeText;
    while (*lat != '\0' && (p - line) < static_cast<int>(sizeof(line) - 1)) {
      *p++ = *lat++;
    }
    if ((p - line) < static_cast<int>(sizeof(line) - 1)) {
      *p++ = ',';
    }
    const char* lon = longitudeText;
    while (*lon != '\0' && (p - line) < static_cast<int>(sizeof(line) - 1)) {
      *p++ = *lon++;
    }
    *p = '\0';
  } else {
    const char* txt = "Position: ----,----";
    uint8_t i = 0;
    while (txt[i] != '\0' && i < (sizeof(line) - 1)) {
      line[i] = txt[i];
      ++i;
    }
    line[i] = '\0';
  }
  u8g2.drawStr(2, toPhysicalY(54), line);
}