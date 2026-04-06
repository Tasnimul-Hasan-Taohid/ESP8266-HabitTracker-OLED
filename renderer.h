#pragma once

#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "config.h"
#include "habits.h"

class Renderer {
public:
  Renderer();
  bool begin();

  // Screens
  void drawSummary(HabitStore& store, int selectedIdx);
  void drawDetail(HabitStore& store, int idx);
  void drawHoldProgress(float pct);   // Overlay: hold bar while user holds
  void drawDoneAnim(const char* name);
  void drawUnmarkConfirm(const char* name);
  void drawNewDay();
  void drawBoot();

private:
  Adafruit_SSD1306 _display;

  void _drawCalendarGrid(Habit& h, int x, int y);
  void _drawStreakBar(int streak, int best, int x, int y, int w);
  void _drawCentered(const String& text, int y, uint8_t size = 1);
  void _drawCheckmark(int x, int y);
  void _drawCross(int x, int y);
};
