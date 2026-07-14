#include "button_manager.h"
#include "Arduino.h"

ButtonManager::ButtonManager(uint8_t pin, uint32_t debounceDelayMs, uint32_t longPressDelayMs)
    : _pin(pin), _debounceDelay(debounceDelayMs), _longPressDelay(longPressDelayMs),
      _lastPinState(HIGH), _stableState(HIGH), _lastTransitionTime(0),
      _pressStartTime(0), _longPressTriggered(false) {}

void ButtonManager::begin() {
    pinMode(_pin, INPUT);
    // Initialize state
    uint8_t initVal = digitalRead(_pin);
    _lastPinState = initVal;
    _stableState = initVal;
}

ButtonAction ButtonManager::update(unsigned long currentMillis) {
    uint8_t reading = digitalRead(_pin);
    ButtonAction action = ButtonAction::NONE;

    if (reading != _lastPinState) {
        _lastTransitionTime = currentMillis;
        _lastPinState = reading;
    }

    if ((currentMillis - _lastTransitionTime) >= _debounceDelay) {
        if (reading != _stableState) {
            _stableState = reading;
            if (_stableState == LOW) {
                _pressStartTime = _lastTransitionTime;
                _longPressTriggered = false;
            } else { // HIGH (released)
                if (!_longPressTriggered) {
                    if (_pressStartTime > 0 && (currentMillis - _pressStartTime) < _longPressDelay) {
                        action = ButtonAction::SINGLE_PRESS;
                    }
                }
                _pressStartTime = 0;
            }
        }
    }

    if (_stableState == LOW && !_longPressTriggered) {
        if ((currentMillis - _pressStartTime) >= _longPressDelay) {
            _longPressTriggered = true;
            action = ButtonAction::LONG_PRESS;
        }
    }

    return action;
}
