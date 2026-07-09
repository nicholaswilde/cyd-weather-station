#include "led_manager.h"
#ifndef NATIVE_TEST
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

LedManager::LedManager(int redPin, int greenPin, int bluePin)
    : _redPin(redPin), _greenPin(greenPin), _bluePin(bluePin),
      _state(STATE_OFF), _enabled(true),
      _lastToggleTime(0), _blinkState(false), _stateStartTime(0),
      _stateInitialized(false) {}

void LedManager::begin() {
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
    turnOffAll();
    _stateInitialized = false;
}

void LedManager::writePins(bool r, bool g, bool b) {
    if (!_enabled) {
        turnOffAll();
        return;
    }
    digitalWrite(_redPin, r ? LOW : HIGH);
    digitalWrite(_greenPin, g ? LOW : HIGH);
    digitalWrite(_bluePin, b ? LOW : HIGH);
}

void LedManager::turnOffAll() {
    digitalWrite(_redPin, HIGH);
    digitalWrite(_greenPin, HIGH);
    digitalWrite(_bluePin, HIGH);
}

void LedManager::update(unsigned long currentMillis) {
    if (!_enabled) {
        turnOffAll();
        return;
    }

    if (!_stateInitialized) {
        _stateStartTime = currentMillis;
        _lastToggleTime = currentMillis;
        _blinkState = true;
        _stateInitialized = true;
    }

    switch (_state) {
        case STATE_OFF:
            writePins(false, false, false);
            break;

        case STATE_CONNECTING:
            if (currentMillis - _lastToggleTime >= 500) {
                _blinkState = !_blinkState;
                _lastToggleTime = currentMillis;
            }
            writePins(false, false, _blinkState);
            break;

        case STATE_CONNECTED:
            if (currentMillis - _stateStartTime >= 3000) {
                _state = STATE_OFF;
                _stateStartTime = 0;
                writePins(false, false, false);
            } else {
                writePins(false, true, false);
            }
            break;

        case STATE_DISCONNECTED:
            if (currentMillis - _lastToggleTime >= 200) {
                _blinkState = !_blinkState;
                _lastToggleTime = currentMillis;
            }
            writePins(_blinkState, false, false);
            break;

        case STATE_PULSE_YELLOW:
            if (currentMillis - _stateStartTime >= 1000) {
                _state = STATE_OFF;
                _stateStartTime = 0;
                writePins(false, false, false);
            } else {
                writePins(true, true, false);
            }
            break;

        case STATE_PULSE_BLUE:
            if (currentMillis - _stateStartTime >= 1000) {
                _state = STATE_OFF;
                _stateStartTime = 0;
                writePins(false, false, false);
            } else {
                writePins(false, false, true);
            }
            break;

        case STATE_PULSE_WHITE:
            if (currentMillis - _stateStartTime >= 1000) {
                _state = STATE_OFF;
                _stateStartTime = 0;
                writePins(false, false, false);
            } else {
                writePins(true, true, true);
            }
            break;

        case STATE_ALERT_RED:
            if (currentMillis - _lastToggleTime >= 500) {
                _blinkState = !_blinkState;
                _lastToggleTime = currentMillis;
            }
            writePins(_blinkState, false, false);
            break;
    }
}

void LedManager::setState(State state) {
    if (_state != state) {
        _state = state;
        _stateStartTime = 0;
        _lastToggleTime = 0;
        _blinkState = true;
        _stateInitialized = false;
    }
}

LedManager::State LedManager::getState() const {
    return _state;
}

void LedManager::setEnabled(bool enabled) {
    _enabled = enabled;
    if (!_enabled) {
        turnOffAll();
    }
}

bool LedManager::isEnabled() const {
    return _enabled;
}
