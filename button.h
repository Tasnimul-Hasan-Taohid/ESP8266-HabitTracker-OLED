#pragma once
#include <Arduino.h>
#include "config.h"

class Button {
public:
  Button();
  void begin();
  void update();

  bool wasPressed();       // Single press, fires once
  bool wasHeld();          // Hold >= HOLD_MS, fires once per hold
  bool isCurrentlyHeld();  // True while physically held >= HOLD_MS

private:
  bool          _last;
  bool          _current;
  bool          _pressFlag;
  bool          _holdFlag;
  bool          _holdFired;
  unsigned long _debounceTime;
  unsigned long _pressTime;
};
