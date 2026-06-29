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
  float latitude;
  float longitude;
  uint8_t satellites;
  uint32_t baudRate;
  uint8_t baudIndex;
  uint16_t rawEvents;
  uint16_t isrChars;
  uint16_t parsedSentences;
  unsigned long lastBaudSwitchMs;
  unsigned long lastByteMs;
  unsigned long lastSentenceMs;
};

inline GpsScreenState& gpsScreenState() {
  static GpsScreenState state = {false, false, false, false, false, false, 0.0f, 0.0f, 0, 9600UL, 0, 0, 0, 0, 0, 0, 0};
  return state;
}

inline NMEAGPS& gpsParser();

inline volatile uint16_t& gpsIsrCharCount() {
  static volatile uint16_t count = 0;
  return count;
}

inline void handleGpsRxChar(uint8_t) {
  volatile uint16_t& count = gpsIsrCharCount();
  if (count < 9999) {
    count += 1;
  }
}

inline void handleGpsRxCharForParser(uint8_t c) {
  volatile uint16_t& count = gpsIsrCharCount();
  if (count < 9999) {
    count += 1;
  }
  gpsParser().handle(c);
}

inline const uint32_t* gpsBaudRates() {
  static const uint32_t rates[] = {9600UL, 115200UL, 38400UL, 19200UL, 4800UL};
  return rates;
}

inline uint8_t gpsBaudRateCount() {
  return 4;
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
  state = {false, false, false, false, false, false, 0.0f, 0.0f, 0, 9600UL, 0, 0, 0, 0, 0, 0, 0};
  gpsIsrCharCount() = 0;
}

inline void restartGpsSerialAtCurrentBaud() {
  GpsScreenState& state = gpsScreenState();
  NeoSWSerial& gps = gpsSerialPort();

  gps.end();
  gps.attachInterrupt(handleGpsRxCharForParser);
  gps.begin(state.baudRate);
  gps.listen();
  state.serialStarted = true;
}

inline void setGpsCreenEnabled(bool enabled) {
  GpsScreenState& state = gpsScreenState();
  NeoSWSerial& gps = gpsSerialPort();

  if (enabled) {
    if (!state.serialStarted) {
      gps.attachInterrupt(handleGpsRxCharForParser);
      gps.begin(state.baudRate);
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
  state.isrChars = gpsIsrCharCount();

  if (!state.hasSignal && state.parsedSentences == 0 && (nowMs - state.lastBaudSwitchMs) > 3000UL) {
    state.baudIndex = (state.baudIndex + 1) % gpsBaudRateCount();
    state.baudRate = gpsBaudRates()[state.baudIndex];
    state.lastBaudSwitchMs = nowMs;
    if (state.active) {
      restartGpsSerialAtCurrentBaud();
    }
  }

  if (state.isrChars > 0) {
    state.lastByteMs = nowMs;
    state.hasSignal = true;
    state.rawEvents = state.isrChars;
  }

  while (parser.available()) {
    const gps_fix fix = parser.read();
    state.lastSentenceMs = nowMs;
    if (state.parsedSentences < 9999) {
      state.parsedSentences += 1;
    }
    state.hasLocation = fix.valid.location;
    state.hasFix = fix.valid.location;
    state.hasSatellites = fix.valid.satellites;

    if (fix.valid.location) {
      state.latitude = fix.latitude();
      state.longitude = fix.longitude();
    }

    if (fix.valid.satellites) {
      state.satellites = fix.satellites;
    }
  }

  if (state.hasSignal && (nowMs - state.lastByteMs) > 4000UL) {
    state.hasSignal = false;
    state.hasFix = false;
  }
}

inline void renderGpsCreen(U8G2& u8g2) {
  GpsScreenState& state = gpsScreenState();
  char line[28];

  u8g2.setFont(u8g2_font_6x12_tr);

  if (!state.hasSignal) {
    u8g2.drawStr(2, toPhysicalY(18), "STATUT: PAS DE SIG");
  } else if (state.parsedSentences == 0) {
    u8g2.drawStr(2, toPhysicalY(18), "STATUT: RX BRUT");
  } else if (state.hasFix) {
    u8g2.drawStr(2, toPhysicalY(18), "STATUT: FIX OK");
  } else {
    u8g2.drawStr(2, toPhysicalY(18), "STATUT: RECHERCHE");
  }


  if (state.hasSatellites) {
    snprintf(line, sizeof(line), "SAT:%u NMEA:%u", state.satellites, state.parsedSentences);
  } else {
    snprintf(line, sizeof(line), "SAT:-- NMEA:%u", state.parsedSentences);
  }
  u8g2.drawStr(2, toPhysicalY(34), line);

  snprintf(line, sizeof(line), "RX:%u ISR:%u", state.rawEvents, state.isrChars);
  u8g2.drawStr(2, toPhysicalY(50), line);

  snprintf(line, sizeof(line), "B:%lu", static_cast<unsigned long>(state.baudRate));
  u8g2.drawStr(2, toPhysicalY(62), line);
}