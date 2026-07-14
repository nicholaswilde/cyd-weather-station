#include "button_manager.h"
#include "Arduino.h"

ButtonManager::ButtonManager(uint8_t pin, uint32_t debounceDelayMs, uint32_t longPressDelayMs)
    : _pin(pin), _debounceDelay(debounceDelayMs), _longPressDelay(longPressDelayMs),
      _lastPinState(HIGH), _stableState(HIGH), _lastTransitionTime(0),
      _pressStartTime(0), _longPressTriggered(false) {}

void ButtonManager::begin() {
    pinMode(_pin, INPUT);
}

ButtonAction ButtonManager::update(unsigned long currentMillis) {
    // Skeleton implementation
    return ButtonAction::NONE;
}
