# 📅 Habit Tracker — ESP8266 + OLED

> Track up to 5 daily habits on an ESP8266 with a 0.96" OLED display. One button cycles through habits, hold to mark done. Streaks, best streaks, and a 7-day calendar grid — all saved to EEPROM and surviving power cycles.

No WiFi. No app. No subscription. Just consistency.

---

## Features

| Feature | Details |
|---|---|
| 📋 5 Habits | Pre-loaded defaults, all customisable in code |
| ✅ Mark Done | Hold button to mark a habit complete for the day |
| 🔥 Streak Counter | Counts consecutive days completed |
| 🏆 Best Streak | All-time best per habit, saved to EEPROM |
| 📆 7-Day Grid | Visual calendar of the last 7 days |
| 💾 EEPROM Persistence | All data survives power-off |
| 📊 Summary View | All habits at a glance with done/not done |
| 🔄 Detail View | Zoom in on a single habit for full stats |
| ⏱️ Hold Progress Bar | Live fill bar while holding the button |
| 🌅 New Day | Automatically advances on each power-on |

---

## Default Habits

Loaded on first boot. Change them in `habits.cpp`:

1. Exercise
2. Read
3. Meditate
4. No Junk Food
5. Sleep Early

---

## Hardware

### Components

| Part | Details |
|---|---|
| Board | ESP8266 — NodeMCU or Wemos D1 Mini |
| Display | 0.96" OLED, SSD1306, 128×64 px, I2C |
| Button | Momentary push button (normally open) |
| Power | Micro-USB 5V |

### Wiring

| ESP8266 Pin | Connected To | Notes |
|---|---|---|
| D1 (GPIO5) | OLED SCL | I2C Clock |
| D2 (GPIO4) | OLED SDA | I2C Data |
| 3V3 | OLED VCC | 3.3V only — not 5V |
| GND | OLED GND + Button | Ground |
| D3 (GPIO0) | Button → GND | Active LOW, internal pull-up |

> **No external button needed to test!** GPIO0 is the built-in FLASH button on all NodeMCU and D1 Mini boards.

---

## Project Structure

```
ESP8266-HabitTracker-OLED/
├── main.ino            ← App loop, screen management, button logic
├── config.h            ← All constants, EEPROM layout, screen enums
├── button.h / .cpp     ← Debounced press + hold detection
├── habits.h / .cpp     ← Habit data, EEPROM read/write, streak logic
├── renderer.h / .cpp   ← All OLED screens and drawing
├── daymanager.h / .cpp ← Day-change detection
└── README.md
```

---

## Library Installation

**Sketch → Include Library → Manage Libraries**, install:

| Library | Author |
|---|---|
| Adafruit GFX | Adafruit |
| Adafruit SSD1306 | Adafruit |

No other external libraries needed.

---

## Flash & Run

1. Open `main.ino` in Arduino IDE
2. **Tools → Board** → `NodeMCU 1.0` or `LOLIN(Wemos) D1 Mini`
3. **Tools → Port** → select your COM port
4. Click **Upload**
5. On first boot, defaults are written to EEPROM automatically

---

## How to Use

### Summary Screen (default view)
Shows all 5 habits with:
- Habit name
- Current streak (days)
- Tiny 7-dot history (filled = done)
- ✓ or ✗ for today

```
Press  →  cycle to next habit
Hold   →  open detail view for selected habit
```

### Detail Screen
Full view of one habit:
- Large streak number
- Best streak
- 7-day calendar grid (labeled 6→T, oldest to today)

```
Press  →  back to summary
Hold   →  mark done (or unmark if already done)
```

### Hold Progress Bar
When you hold the button, a progress bar fills at the bottom of the screen. Release early = cancelled. Fill completely = action confirmed.

### Done Animation
After marking a habit done, a checkmark animation plays for 1.8 seconds, then returns to the detail view.

### New Day
Every time the device powers on, it advances the day counter. This shifts history, resets today's done flags, and updates streaks. A "New Day!" screen is shown briefly on boot.

---

## OLED Screens

### Summary View
```
┌──────────────────────────┐
│ Habits          Day 12   │  ← header bar (inverted)
├──────────────────────────┤
│ Exercise   3d  ░▓▓░▓▓▓  ✓│  ← selected (inverted)
│ Read       0d  ░░░░░░░  ✗│
│ Meditate   1d  ░░░░░░▓  ✓│
│ No Junk    5d  ▓▓▓▓▓▓▓  ✓│
│ Sleep Ear  2d  ░░░░░▓▓  ✓│
│ Press=next  Hold=detail  │
└──────────────────────────┘
```

### Detail View
```
┌──────────────────────────┐
│ Exercise              DONE│  ← header (inverted)
│ Streak:    3             │
│            days          │
│ Best: 7d                 │
│ Last 7 days:             │
│  6  5  4  3  2  1  T     │
│ [░][░][▓][░][▓][▓][▓]   │
│ Hold=mark done  Press=bk │
└──────────────────────────┘
```

### Done Animation
```
┌──────────────────────────┐
│          Done!           │
│   ╭────╮    Exercise     │
│   │ ✓  │                 │
│   ╰────╯                 │
│       Keep it up!        │
└──────────────────────────┘
```

---

## Customising Habits

Open `habits.cpp` and edit `DEFAULT_NAMES[]`:

```cpp
static const char* DEFAULT_NAMES[MAX_HABITS] = {
  "Exercise",      // Max 11 characters
  "Read",
  "Meditate",
  "No Junk Food",
  "Sleep Early"
};
```

> **Important:** Changing names after first boot requires a factory reset (wipe EEPROM). See below.

---

## Factory Reset (wipe EEPROM)

To reset all habits and streaks:

1. Open `main.ino`
2. In `setup()`, temporarily add this line before `store.begin()`:

```cpp
EEPROM.begin(256);
for (int i = 0; i < 256; i++) EEPROM.write(i, 0);
EEPROM.commit();
```

3. Upload, let it run once, then remove the lines and upload again

---

## EEPROM Layout

```
Byte 0        : Magic (0xAB) — first boot detection
Byte 1        : Habit count
Byte 2        : Day index (0–255, rolling)
Bytes 3–9     : Reserved

Per habit (30 bytes each, starting at byte 10):
  [0..11]     : Name (12 bytes, null terminated)
  [12]        : Current streak
  [13]        : Best streak
  [14..20]    : 7-day history (1=done, 0=missed)
  [21]        : Done today flag
  [22..29]    : Reserved

Total used: 10 + (5 × 30) = 160 bytes
```

---

## Day Tracking Note

This project has no RTC (real-time clock). Instead:
- Each **power-on** counts as a new day
- If kept powered on continuously, a new day triggers every **24 hours**

For accurate daily tracking, power the device on once per day (e.g. each morning). For a proper clock-based version, add a DS3231 RTC module.

---

## Troubleshooting

**Display blank**
- Check SDA → D2 (GPIO4), SCL → D1 (GPIO5)
- Use 3.3V power only — 5V will damage the OLED
- Confirm I2C address is `0x3C` in `config.h`

**Button not responding**
- GPIO0 is active LOW with internal pull-up
- Connect one button leg to GPIO0, other to GND
- Open Serial Monitor at 115200 baud — look for `[BTN]` logs

**Data lost on power-off**
- This shouldn't happen — all data is EEPROM backed
- If it does, check Serial Monitor for `[HABITS] Saved` messages
- Try increasing `SAVE_DELAY_MS` in `config.h`

**Streaks not counting correctly**
- Streak counts consecutive days from today backwards
- One missed day resets the streak to 0
- Best streak is saved separately and never resets

**All habits reset to defaults**
- This means the EEPROM magic byte was lost
- Can happen after reflashing — data is preserved across soft resets only

---

## Configuration

Edit `config.h`:

```cpp
// Hold duration to mark done (default 800ms)
#define HOLD_MS           800

// Idle time before returning to summary (default 6s)
#define IDLE_TIMEOUT_MS   6000

// Max habits tracked
#define MAX_HABITS        5

// Days of history shown
#define HISTORY_DAYS      7
```

---

## License

MIT — free to use, modify, and build on.

---

## Credits

Built with ESP8266 Arduino core + Adafruit SSD1306.  
Inspired by the Streaks app and James Clear's *Atomic Habits*.
