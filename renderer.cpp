#include "renderer.h"

Renderer::Renderer()
  : _display(OLED_WIDTH, OLED_HEIGHT, &Wire, OLED_RESET) {}

bool Renderer::begin() {
  Wire.begin(OLED_SDA, OLED_SCL);
  if (!_display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    Serial.println("[RENDERER] Failed");
    return false;
  }
  _display.clearDisplay();
  _display.setTextColor(SSD1306_WHITE);
  _display.display();
  Serial.println("[RENDERER] OK");
  return true;
}

// ─── Boot Screen ───────────────────────────────────────────────
void Renderer::drawBoot() {
  _display.clearDisplay();
  _drawCentered("HabitTracker", 12, 1);
  _drawCentered("v1.0", 26);
  _drawCentered("Loading...", 40);
  _display.display();
}

// ─── New Day Screen ────────────────────────────────────────────
void Renderer::drawNewDay() {
  _display.clearDisplay();
  _drawCentered("New Day!", 10, 2);
  _drawCentered("Keep going!", 36);
  _drawCentered("Streaks updated", 50);
  _display.display();
}

// ─── Summary Screen ────────────────────────────────────────────
// Shows all habits in a compact list, selected one is highlighted
void Renderer::drawSummary(HabitStore& store, int selectedIdx) {
  _display.clearDisplay();

  // Header bar
  _display.fillRect(0, 0, OLED_WIDTH, 10, SSD1306_WHITE);
  _display.setTextColor(SSD1306_BLACK);
  _display.setTextSize(1);
  _display.setCursor(2, 1);
  _display.print("Habits");

  // Today's date label (day counter)
  String dayStr = "Day " + String(store.getDayIndex() + 1);
  int dayX = OLED_WIDTH - dayStr.length() * 6 - 2;
  _display.setCursor(dayX, 1);
  _display.print(dayStr);
  _display.setTextColor(SSD1306_WHITE);

  int count = store.getHabitCount();
  int rowH  = (OLED_HEIGHT - 11) / count;  // height per row

  for (int i = 0; i < count; i++) {
    Habit& h    = store.getHabit(i);
    int    rowY = 11 + i * rowH;
    bool   sel  = (i == selectedIdx);
    bool   done = (h.doneToday == 1);

    // Selection highlight
    if (sel) {
      _display.fillRect(0, rowY, OLED_WIDTH, rowH, SSD1306_WHITE);
      _display.setTextColor(SSD1306_BLACK);
    } else {
      _display.setTextColor(SSD1306_WHITE);
    }

    // Habit name (truncated to fit)
    _display.setTextSize(1);
    _display.setCursor(3, rowY + 2);
    char shortName[10];
    strncpy(shortName, h.name, 9);
    shortName[9] = '\0';
    _display.print(shortName);

    // Streak
    String stk = String(h.streak) + "d";
    _display.setCursor(72, rowY + 2);
    _display.print(stk);

    // Done / not done indicator (right side)
    if (done) {
      _drawCheckmark(112, rowY + 2);
    } else {
      _drawCross(112, rowY + 2);
    }

    // Tiny 7-day dots (7 pixels wide)
    for (int d = 6; d >= 0; d--) {
      int dotX = 90 + (6 - d) * 3;
      int dotY = rowY + 2;
      if (h.history[d]) {
        _display.fillRect(dotX, dotY + 2, 2, 2, sel ? SSD1306_BLACK : SSD1306_WHITE);
      } else {
        _display.drawRect(dotX, dotY + 2, 2, 2, sel ? SSD1306_BLACK : SSD1306_WHITE);
      }
    }

    _display.setTextColor(SSD1306_WHITE);
  }

  // Footer hint
  _display.setTextSize(1);
  _display.setTextColor(SSD1306_WHITE);
  _display.setCursor(0, 57);
  _display.print("Press=next  Hold=detail");

  _display.display();
}

// ─── Detail Screen ─────────────────────────────────────────────
// Full screen for one habit: name, streak, best, 7-day grid
void Renderer::drawDetail(HabitStore& store, int idx) {
  _display.clearDisplay();
  Habit& h   = store.getHabit(idx);
  bool   done = (h.doneToday == 1);

  // ── Header ──
  _display.fillRect(0, 0, OLED_WIDTH, 11, SSD1306_WHITE);
  _display.setTextColor(SSD1306_BLACK);
  _display.setTextSize(1);
  _display.setCursor(2, 2);
  _display.print(h.name);

  // Done badge on header right
  if (done) {
    _display.setCursor(104, 2);
    _display.print("DONE");
  }
  _display.setTextColor(SSD1306_WHITE);

  // ── Streak row ──
  _display.setTextSize(1);
  _display.setCursor(2, 14);
  _display.print("Streak:");

  // Large streak number
  _display.setTextSize(2);
  String stk = String(h.streak);
  _display.setCursor(50, 11);
  _display.print(stk);

  _display.setTextSize(1);
  _display.setCursor(50 + stk.length() * 12, 18);
  _display.print("days");

  // Best streak
  _display.setCursor(2, 26);
  _display.print("Best: ");
  _display.print(h.bestStreak);
  _display.print("d");

  // ── 7-day calendar grid ──
  // Label row: Mo Tu We Th Fr Sa Su (generic — we show last 7 days oldest→newest)
  _display.setCursor(2, 36);
  _display.print("Last 7 days:");

  _drawCalendarGrid(h, 2, 46);

  // ── Footer hint ──
  _display.setCursor(0, 57);
  if (!done) {
    _display.print("Hold=mark done  Press=back");
  } else {
    _display.print("Hold=unmark  Press=back");
  }

  _display.display();
}

// ─── Hold Progress Overlay ─────────────────────────────────────
// Draw a progress bar at the bottom OVER the current screen
void Renderer::drawHoldProgress(float pct) {
  // Clear footer area only
  _display.fillRect(0, 56, OLED_WIDTH, 8, SSD1306_BLACK);

  // Bar background
  _display.drawRect(0, 57, OLED_WIDTH, 6, SSD1306_WHITE);

  // Fill
  int fill = (int)(pct * (OLED_WIDTH - 2));
  if (fill > 0) {
    _display.fillRect(1, 58, fill, 4, SSD1306_WHITE);
  }

  _display.display();
}

// ─── Done Animation ────────────────────────────────────────────
void Renderer::drawDoneAnim(const char* name) {
  _display.clearDisplay();

  // Big checkmark
  // Outer circle
  _display.drawCircle(24, 32, 18, SSD1306_WHITE);
  _display.drawCircle(24, 32, 17, SSD1306_WHITE);
  // Checkmark lines
  _display.drawLine(14, 32, 21, 39, SSD1306_WHITE);
  _display.drawLine(15, 32, 22, 39, SSD1306_WHITE);
  _display.drawLine(21, 39, 34, 24, SSD1306_WHITE);
  _display.drawLine(22, 39, 35, 24, SSD1306_WHITE);

  _display.setTextSize(1);
  _drawCentered("Done!", 10);

  // Name
  _display.setCursor(50, 24);
  _display.setTextSize(1);
  char truncName[10];
  strncpy(truncName, name, 9);
  truncName[9] = '\0';
  _display.print(truncName);

  _drawCentered("Keep it up!", 48);

  _display.display();
}

// ─── Unmark Confirm ────────────────────────────────────────────
void Renderer::drawUnmarkConfirm(const char* name) {
  _display.clearDisplay();
  _drawCentered("Unmark?", 10, 1);

  char truncName[10];
  strncpy(truncName, name, 9);
  truncName[9] = '\0';
  _drawCentered(truncName, 24);
  _drawCentered("Hold to confirm", 40);
  _drawCentered("Press to cancel", 52);

  _display.display();
}

// ─── Private: 7-day calendar grid ─────────────────────────────
// Draws 7 squares (8x8 each) with filled=done, outline=missed
void Renderer::_drawCalendarGrid(Habit& h, int x, int y) {
  int cellW = 14;  // spacing between cells
  int cellS = 10;  // cell size

  // Day labels above (oldest left, today right)
  const char* labels[] = {"6", "5", "4", "3", "2", "1", "T"};
  _display.setTextSize(1);

  for (int d = 6; d >= 0; d--) {
    int col  = 6 - d;
    int cellX = x + col * cellW;

    // Label
    _display.setCursor(cellX + 2, y - 1);
    _display.print(labels[col]);

    // Cell
    if (h.history[d]) {
      // Done — filled square
      _display.fillRect(cellX, y + 8, cellS, cellS, SSD1306_WHITE);
      // Small X inside to distinguish
      _display.drawLine(cellX + 2, y + 10, cellX + cellS - 3, y + cellS + 6, SSD1306_BLACK);
      _display.drawLine(cellX + cellS - 3, y + 10, cellX + 2, y + cellS + 6, SSD1306_BLACK);
    } else {
      // Missed — outline only
      _display.drawRect(cellX, y + 8, cellS, cellS, SSD1306_WHITE);
    }
  }
}

// ─── Private: Checkmark ────────────────────────────────────────
void Renderer::_drawCheckmark(int x, int y) {
  _display.drawLine(x,     y + 3, x + 2, y + 5, SSD1306_WHITE);
  _display.drawLine(x + 2, y + 5, x + 6, y + 1, SSD1306_WHITE);
}

// ─── Private: Cross ────────────────────────────────────────────
void Renderer::_drawCross(int x, int y) {
  _display.drawLine(x,     y,     x + 5, y + 5, SSD1306_WHITE);
  _display.drawLine(x + 5, y,     x,     y + 5, SSD1306_WHITE);
}

// ─── Private: Centered text ────────────────────────────────────
void Renderer::_drawCentered(const String& text, int y, uint8_t size) {
  _display.setTextSize(size);
  _display.setTextColor(SSD1306_WHITE);
  int16_t x1, y1; uint16_t w, h;
  _display.getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  int x = max(0, (int)((OLED_WIDTH - (int)w) / 2));
  _display.setCursor(x, y);
  _display.print(text);
}
