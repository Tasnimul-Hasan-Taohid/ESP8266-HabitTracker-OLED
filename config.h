#pragma once

// ─── OLED ───────────────────────────────────────────────────────
#define OLED_SDA          4        // D2
#define OLED_SCL          5        // D1
#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_RESET        -1
#define OLED_I2C_ADDR     0x3C

// ─── Button ─────────────────────────────────────────────────────
#define BUTTON_PIN        0        // D3 / GPIO0  active LOW
#define DEBOUNCE_MS       50
#define HOLD_MS           800      // Hold duration to mark done / confirm action

// ─── Habits ─────────────────────────────────────────────────────
#define MAX_HABITS        5
#define HABIT_NAME_LEN    12       // Max chars in a habit name (incl null)
#define HISTORY_DAYS      7        // Days of history shown in calendar grid

// ─── Timing ─────────────────────────────────────────────────────
#define IDLE_TIMEOUT_MS   6000     // Return to summary after idle (ms)
#define SAVE_DELAY_MS     500      // Small delay before EEPROM write

// ─── EEPROM Layout ──────────────────────────────────────────────
//
//  Byte  0        : Magic byte (0xAB) — detect first boot
//  Byte  1        : Number of active habits (0–5)
//  Byte  2        : Day index (0–364, rolls over) — increments each new day
//  Bytes 3–6      : Timestamp placeholder (millis-based day counter)
//  Bytes 7–9      : Reserved
//
//  Per habit (10 bytes each, habits 0–4):
//    Offset 10 + (habit * 30)
//      [0..11]  : name (12 bytes, null terminated)
//      [12]     : streak (uint8, days)
//      [13]     : best streak (uint8)
//      [14..20] : history bits — 7 bytes, each byte = 1 day (1=done, 0=missed)
//                 history[0] = today, history[1] = yesterday, ...
//      [21]     : done today flag (0 or 1)
//      [22..29] : reserved
//
//  Total EEPROM used: 10 + (5 * 30) = 160 bytes
//
#define EEPROM_SIZE         256
#define EEPROM_MAGIC        0xAB
#define EEPROM_MAGIC_ADDR   0
#define EEPROM_HABIT_COUNT  1
#define EEPROM_DAY_ADDR     2      // 1 byte day counter (0-255, wraps)
#define EEPROM_HABITS_START 10
#define EEPROM_HABIT_STRIDE 30

// ─── App Screens ────────────────────────────────────────────────
enum AppScreen {
  SCREEN_SUMMARY,    // Overview: all habits + done/not done today
  SCREEN_DETAIL,     // Single habit: name, streak, 7-day grid
  SCREEN_CONFIRM,    // "Hold to mark done" or "Hold to reset"
  SCREEN_DONE_ANIM,  // Brief celebration when marked done
  SCREEN_SETUP,      // First-boot habit name entry (cycles through presets)
};

// ─── Debug ──────────────────────────────────────────────────────
#define DEBUG_BAUD  115200
