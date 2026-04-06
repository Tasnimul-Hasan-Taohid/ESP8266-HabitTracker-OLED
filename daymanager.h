#pragma once
#include <Arduino.h>
#include "config.h"

// Since there's no RTC, we simulate a "day" using a configurable
// real-time period. Default is 24 hours. For testing you can set
// it much shorter via DAY_DURATION_MS in config.
//
// On each boot, we compare stored millis epoch against current.
// Because millis() resets on power-off, we treat each fresh boot
// as a potential new day — tracked via EEPROM day index.

class DayManager {
public:
  DayManager();
  void begin(uint8_t lastDayIndex);

  // Call every loop — returns true once when a new day is detected
  bool tick();

  // Force a new day (for testing)
  void forceNewDay();

private:
  unsigned long _dayStartMs;
  uint8_t       _lastDayIndex;
  bool          _newDayThisBoot;
};
