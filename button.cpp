#include "button.h"

Button::Button()
  : _last(HIGH), _current(HIGH),
    _pressFlag(false), _holdFlag(false), _holdFired(false),
    _debounceTime(0), _pressTime(0) {}

void Button::begin() {
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  Serial.println("[BTN] Ready");
}

void Button::update() {
  bool raw = digitalRead(BUTTON_PIN);
  unsigned long now = millis();

  if (raw != _last) _debounceTime = now;

  if ((now - _debounceTime) > DEBOUNCE_MS) {
    // Falling edge — button just pressed
    if (_current == HIGH && raw == LOW) {
      _pressFlag  = true;
      _holdFired  = false;
      _pressTime  = now;
    }
    // Rising edge — button released
    if (_current == LOW && raw == HIGH) {
      _holdFired = false;
    }
    // Check for hold while button is down
    if (raw == LOW && !_holdFired && (now - _pressTime) >= HOLD_MS) {
      _holdFlag  = true;
      _holdFired = true;
    }
    _current = raw;
  }
  _last = raw;
}

bool Button::wasPressed() {
  if (_pressFlag) { _pressFlag = false; return true; }
  return false;
}

bool Button::wasHeld() {
  if (_holdFlag) { _holdFlag = false; return true; }
  return false;
}

bool Button::isCurrentlyHeld() {
  return (_current == LOW && (millis() - _pressTime) >= HOLD_MS);
}
