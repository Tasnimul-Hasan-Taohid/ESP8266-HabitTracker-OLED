#pragma once
#include <Arduino.h>
#include <EEPROM.h>
#include "config.h"

struct Habit {
  char    name[HABIT_NAME_LEN];  // e.g. "Exercise"
  uint8_t streak;                // Current streak in days
  uint8_t bestStreak;            // All-time best streak
  uint8_t history[HISTORY_DAYS]; // 1=done, 0=missed. [0]=today,[1]=yesterday...
  uint8_t doneToday;             // 1 if marked done today
};

class HabitStore {
public:
  HabitStore();

  void    begin();               // Load from EEPROM or init defaults
  void    save();                // Write everything to EEPROM

  // Day management — call advanceDay() when a new day starts
  void    advanceDay();
  uint8_t getDayIndex();         // 0-255 rolling day counter

  // Habit access
  int     getHabitCount();
  Habit&  getHabit(int i);

  // Marking
  void    markDone(int i);       // Mark habit i as done today
  void    unmarkDone(int i);     // Undo (for accidental holds)
  bool    isDoneToday(int i);

  // Streak helpers
  int     getLongestStreak(int i);
  float   getWeekScore(int i);   // 0.0–1.0 fraction of last 7 days done

private:
  Habit   _habits[MAX_HABITS];
  int     _count;
  uint8_t _dayIndex;

  void    _loadDefaults();
  void    _recalcStreak(int i);
  int     _eepromBase(int i);
  void    _readHabit(int i);
  void    _writeHabit(int i);
};
