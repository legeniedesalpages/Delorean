#pragma once

#include <Arduino.h>

#include "TimeScreen.h"

constexpr uint8_t kRtcEepromI2cAddress = 0x57;
constexpr uint32_t kTripPersistMagic = 0x54525031UL; // TRP1
constexpr unsigned long kTripPersistIntervalMs = 60000UL;

struct TripPersistedData {
  uint32_t magic;
  uint32_t cumulativeSeconds;
  uint32_t previousTripSeconds;
  uint32_t activeTripSnapshotSeconds;
};

struct TripTimeScreenState {
  bool initialized;
  bool persistenceAvailable;
  TripPersistedData data;
  unsigned long lastPersistMs;
};

inline TripTimeScreenState& tripTimeScreenState() {
  static TripTimeScreenState state = {false, false, {0, 0, 0, 0}, 0};
  return state;
}

inline bool rtcEepromReadBytes(uint16_t address, uint8_t* out, uint8_t length) {
  Wire.beginTransmission(kRtcEepromI2cAddress);
  Wire.write(static_cast<uint8_t>((address >> 8) & 0xFF));
  Wire.write(static_cast<uint8_t>(address & 0xFF));
  if (Wire.endTransmission() != 0) {
    return false;
  }

  const uint8_t received = Wire.requestFrom(static_cast<int>(kRtcEepromI2cAddress), static_cast<int>(length));
  if (received != length) {
    return false;
  }

  for (uint8_t i = 0; i < length; ++i) {
    out[i] = Wire.read();
  }
  return true;
}

inline bool rtcEepromWriteBytes(uint16_t address, const uint8_t* data, uint8_t length) {
  for (uint8_t i = 0; i < length; ++i) {
    Wire.beginTransmission(kRtcEepromI2cAddress);
    Wire.write(static_cast<uint8_t>(((address + i) >> 8) & 0xFF));
    Wire.write(static_cast<uint8_t>((address + i) & 0xFF));
    Wire.write(data[i]);
    if (Wire.endTransmission() != 0) {
      return false;
    }
    delay(5);
  }
  return true;
}

inline bool loadTripPersistedData(TripPersistedData& out) {
  return rtcEepromReadBytes(0, reinterpret_cast<uint8_t*>(&out), sizeof(TripPersistedData));
}

inline bool saveTripPersistedData(const TripPersistedData& data) {
  return rtcEepromWriteBytes(0, reinterpret_cast<const uint8_t*>(&data), sizeof(TripPersistedData));
}

inline void initTripTimeScreen() {
  TripTimeScreenState& state = tripTimeScreenState();
  if (state.initialized) {
    return;
  }

  state.initialized = true;
  state.persistenceAvailable = timeScreenRtcAvailable();
  state.data = {kTripPersistMagic, 0, 0, 0};

  if (!state.persistenceAvailable) {
    return;
  }

  TripPersistedData loaded = {0, 0, 0, 0};
  if (!loadTripPersistedData(loaded) || loaded.magic != kTripPersistMagic) {
    saveTripPersistedData(state.data);
    return;
  }

  state.data = loaded;

  if (state.data.activeTripSnapshotSeconds >= 60UL) {
    state.data.previousTripSeconds = state.data.activeTripSnapshotSeconds;
    state.data.cumulativeSeconds += state.data.activeTripSnapshotSeconds;
  }

  state.data.activeTripSnapshotSeconds = 0;
  saveTripPersistedData(state.data);
}

inline void updateTripTimeScreen(unsigned long nowMs) {
  TripTimeScreenState& state = tripTimeScreenState();
  const uint32_t currentTripSeconds = nowMs / 1000UL;

  if (!state.persistenceAvailable) {
    return;
  }

  if ((nowMs - state.lastPersistMs) < kTripPersistIntervalMs) {
    return;
  }

  state.lastPersistMs = nowMs;

  if (state.data.activeTripSnapshotSeconds == currentTripSeconds) {
    return;
  }

  state.data.activeTripSnapshotSeconds = currentTripSeconds;
  saveTripPersistedData(state.data);
}

inline void formatHms(uint32_t seconds, char* out, size_t outLen) {
  const uint32_t hh = seconds / 3600UL;
  const uint32_t mm = (seconds % 3600UL) / 60UL;
  const uint32_t ss = seconds % 60UL;
  snprintf(out, outLen, "%02lu:%02lu:%02lu", static_cast<unsigned long>(hh), static_cast<unsigned long>(mm), static_cast<unsigned long>(ss));
}

inline void formatDhm(uint32_t seconds, char* out, size_t outLen) {
  const uint32_t days = seconds / 86400UL;
  const uint32_t hours = (seconds % 86400UL) / 3600UL;
  const uint32_t minutes = (seconds % 3600UL) / 60UL;
  snprintf(out, outLen, "%lud %02luh %02lum", static_cast<unsigned long>(days), static_cast<unsigned long>(hours), static_cast<unsigned long>(minutes));
}

inline void renderTripTimeScreen(U8G2& u8g2) {
  const unsigned long nowMs = millis();
  updateTripTimeScreen(nowMs);

  TripTimeScreenState& state = tripTimeScreenState();
  const uint32_t currentTripSeconds = nowMs / 1000UL;

  char line[30];
  char value[20];

  u8g2.setFont(u8g2_font_6x12_tr);

  formatHms(currentTripSeconds, value, sizeof(value));
  snprintf(line, sizeof(line), "Actuel: %s", value);
  u8g2.drawStr(2, toPhysicalY(18), line);

  formatHms(state.data.previousTripSeconds, value, sizeof(value));
  snprintf(line, sizeof(line), "Precedent: %s", value);
  u8g2.drawStr(2, toPhysicalY(34), line);

  formatDhm(state.data.cumulativeSeconds, value, sizeof(value));
  snprintf(line, sizeof(line), "Cumule: %s", value);
  u8g2.drawStr(2, toPhysicalY(50), line);
}