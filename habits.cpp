#include "habits.h"

// Default habit names shown on first boot
static const char* DEFAULT_NAMES[MAX_HABITS] = {
  "Exercise",
  "Read",
  "Meditate",
  "No Junk Food",
  "Sleep Early"
};

HabitStore::HabitStore() : _count(MAX_HABITS), _dayIndex(0) {
  memset(_habits, 0, sizeof(_habits));
}

// ─── Begin ─────────────────────────────────────────────────────
void HabitStore::begin() {
  EEPROM.begin(EEPROM_SIZE);

  uint8_t magic = EEPROM.read(EEPROM_MAGIC_ADDR);
  if (magic != EEPROM_MAGIC) {
    Serial.println("[HABITS] First boot — writing defaults");
    _loadDefaults();
    save();
  } else {
    Serial.println("[HABITS] Loading from EEPROM");
    _count    = EEPROM.read(EEPROM_HABIT_COUNT);
    _dayIndex = EEPROM.read(EEPROM_DAY_ADDR);
    if (_count < 1 || _count > MAX_HABITS) _count = MAX_HABITS;
    for (int i = 0; i < _count; i++) _readHabit(i);
    Serial.print("[HABITS] Loaded ");
    Serial.print(_count);
    Serial.print(" habits, day=");
    Serial.println(_dayIndex);
  }
}

// ─── Save ──────────────────────────────────────────────────────
void HabitStore::save() {
  EEPROM.write(EEPROM_MAGIC_ADDR,  EEPROM_MAGIC);
  EEPROM.write(EEPROM_HABIT_COUNT, (uint8_t)_count);
  EEPROM.write(EEPROM_DAY_ADDR,    _dayIndex);
  for (int i = 0; i < _count; i++) _writeHabit(i);
  EEPROM.commit();
  Serial.println("[HABITS] Saved to EEPROM");
}

// ─── Advance Day ───────────────────────────────────────────────
void HabitStore::advanceDay() {
  Serial.println("[HABITS] Advancing day");
  _dayIndex++;

  for (int i = 0; i < _count; i++) {
    Habit& h = _habits[i];

    // Shift history: today's result becomes history[1], etc.
    for (int d = HISTORY_DAYS - 1; d > 0; d--) {
      h.history[d] = h.history[d - 1];
    }

    // Yesterday (now history[1]) was today's done status
    h.history[1] = h.doneToday;

    // Reset today
    h.history[0] = 0;
    h.doneToday  = 0;

    // Recalculate streak based on new history
    _recalcStreak(i);
  }
  save();
}

// ─── Day Index ─────────────────────────────────────────────────
uint8_t HabitStore::getDayIndex() { return _dayIndex; }

// ─── Habit Access ──────────────────────────────────────────────
int     HabitStore::getHabitCount() { return _count; }
Habit&  HabitStore::getHabit(int i) { return _habits[i]; }

// ─── Mark Done ─────────────────────────────────────────────────
void HabitStore::markDone(int i) {
  if (i < 0 || i >= _count) return;
  _habits[i].doneToday  = 1;
  _habits[i].history[0] = 1;
  _recalcStreak(i);
  _writeHabit(i);
  EEPROM.commit();
  Serial.print("[HABITS] Marked done: ");
  Serial.println(_habits[i].name);
}

void HabitStore::unmarkDone(int i) {
  if (i < 0 || i >= _count) return;
  _habits[i].doneToday  = 0;
  _habits[i].history[0] = 0;
  _recalcStreak(i);
  _writeHabit(i);
  EEPROM.commit();
  Serial.print("[HABITS] Unmarked: ");
  Serial.println(_habits[i].name);
}

bool HabitStore::isDoneToday(int i) {
  if (i < 0 || i >= _count) return false;
  return _habits[i].doneToday == 1;
}

// ─── Stats ─────────────────────────────────────────────────────
int HabitStore::getLongestStreak(int i) {
  if (i < 0 || i >= _count) return 0;
  return _habits[i].bestStreak;
}

float HabitStore::getWeekScore(int i) {
  if (i < 0 || i >= _count) return 0.0f;
  int done = 0;
  for (int d = 0; d < HISTORY_DAYS; d++) {
    if (_habits[i].history[d]) done++;
  }
  return (float)done / (float)HISTORY_DAYS;
}

// ─── Private: Defaults ─────────────────────────────────────────
void HabitStore::_loadDefaults() {
  _count    = MAX_HABITS;
  _dayIndex = 0;
  for (int i = 0; i < _count; i++) {
    Habit& h = _habits[i];
    memset(&h, 0, sizeof(Habit));
    strncpy(h.name, DEFAULT_NAMES[i], HABIT_NAME_LEN - 1);
    h.name[HABIT_NAME_LEN - 1] = '\0';
    h.streak     = 0;
    h.bestStreak = 0;
    h.doneToday  = 0;
    memset(h.history, 0, sizeof(h.history));
  }
}

// ─── Private: Streak Calculation ───────────────────────────────
void HabitStore::_recalcStreak(int i) {
  Habit& h = _habits[i];
  int streak = 0;
  // Count consecutive days from today backwards
  for (int d = 0; d < HISTORY_DAYS; d++) {
    if (h.history[d] == 1) streak++;
    else break;
  }
  h.streak = (uint8_t)streak;
  if (h.streak > h.bestStreak) h.bestStreak = h.streak;
}

// ─── Private: EEPROM address ───────────────────────────────────
int HabitStore::_eepromBase(int i) {
  return EEPROM_HABITS_START + (i * EEPROM_HABIT_STRIDE);
}

// ─── Private: Read one habit from EEPROM ───────────────────────
void HabitStore::_readHabit(int i) {
  int base = _eepromBase(i);
  Habit& h = _habits[i];

  for (int c = 0; c < HABIT_NAME_LEN; c++)
    h.name[c] = (char)EEPROM.read(base + c);
  h.name[HABIT_NAME_LEN - 1] = '\0';

  h.streak     = EEPROM.read(base + 12);
  h.bestStreak = EEPROM.read(base + 13);

  for (int d = 0; d < HISTORY_DAYS; d++)
    h.history[d] = EEPROM.read(base + 14 + d);

  h.doneToday = EEPROM.read(base + 21);
}

// ─── Private: Write one habit to EEPROM ────────────────────────
void HabitStore::_writeHabit(int i) {
  int base = _eepromBase(i);
  Habit& h = _habits[i];

  for (int c = 0; c < HABIT_NAME_LEN; c++)
    EEPROM.write(base + c, (uint8_t)h.name[c]);

  EEPROM.write(base + 12, h.streak);
  EEPROM.write(base + 13, h.bestStreak);

  for (int d = 0; d < HISTORY_DAYS; d++)
    EEPROM.write(base + 14 + d, h.history[d]);

  EEPROM.write(base + 21, h.doneToday);
}
