#include "daymanager.h"

// A "day" = DAY_DURATION_MS milliseconds of uptime
// Default: 86400000 = 24 hours
// For testing, set to something small like 60000 (1 minute)
#define DAY_DURATION_MS  86400000UL

DayManager::DayManager()
  : _dayStartMs(0), _lastDayIndex(0), _newDayThisBoot(false) {}

void DayManager::begin(uint8_t lastDayIndex) {
  _dayStartMs     = millis();
  _lastDayIndex   = lastDayIndex;
  // On each fresh power-on, we consider it a new day
  // This is the simplest approach without an RTC
  _newDayThisBoot = true;
  Serial.print("[DAY] Started. Last day index=");
  Serial.println(lastDayIndex);
}

bool DayManager::tick() {
  // Fire once per boot (simulating new day on power-on)
  if (_newDayThisBoot) {
    _newDayThisBoot = false;
    Serial.println("[DAY] New day triggered (boot)");
    return true;
  }

  // Also fire if device stays on long enough (24h continuous use)
  if (millis() - _dayStartMs >= DAY_DURATION_MS) {
    _dayStartMs = millis();
    Serial.println("[DAY] New day triggered (24h elapsed)");
    return true;
  }

  return false;
}

void DayManager::forceNewDay() {
  _newDayThisBoot = true;
}
