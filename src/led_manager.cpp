#include "led_manager.h"
#ifndef NATIVE_TEST
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

LedManager::LedManager(int redPin, int greenPin, int bluePin)
    : _redPin(redPin), _greenPin(greenPin), _bluePin(bluePin),
      _state(STATE_OFF), _enabled(true), _brightness(255),
      _lastToggleTime(0), _blinkState(false), _stateStartTime(0),
      _stateInitialized(false) {}

void LedManager::begin() {
#ifndef NATIVE_TEST
    // --- Set up three LEDC PWM channels for dimming the RGB LED ---
    ledcSetup(5, 5000, 8); // channel 5 = red,   5 kHz, 8-bit
    ledcSetup(6, 5000, 8); // channel 6 = green, 5 kHz, 8-bit
    ledcSetup(7, 5000, 8); // channel 7 = blue,  5 kHz, 8-bit
    ledcAttachPin(_redPin,   5);
    ledcAttachPin(_greenPin, 6);
    ledcAttachPin(_bluePin,  7);
#else
    pinMode(_redPin, OUTPUT);
    pinMode(_greenPin, OUTPUT);
    pinMode(_bluePin, OUTPUT);
#endif
    turnOffAll();
    _stateInitialized = false;
}

void LedManager::writePins(bool r, bool g, bool b) {
    if (!_enabled) {
        turnOffAll();
        return;
    }
#ifndef NATIVE_TEST
    // --- Pins are active-LOW; duty 255 = OFF, 0 = full-ON ---
    // --- Scale active duty by _brightness (0-255) ---
    int rDuty = r ? (255 - _brightness) : 255;
    int gDuty = g ? (255 - _brightness) : 255;
    int bDuty = b ? (255 - _brightness) : 255;
    ledcWrite(5, rDuty);
    ledcWrite(6, gDuty);
    ledcWrite(7, bDuty);
#else
    digitalWrite(_redPin, r ? LOW : HIGH);
    digitalWrite(_greenPin, g ? LOW : HIGH);
    digitalWrite(_bluePin, b ? LOW : HIGH);
#endif
}

void LedManager::turnOffAll() {
#ifndef NATIVE_TEST
    ledcWrite(5, 255); // active-LOW: duty 255 = OFF
    ledcWrite(6, 255);
    ledcWrite(7, 255);
#else
    digitalWrite(_redPin, HIGH);
    digitalWrite(_greenPin, HIGH);
    digitalWrite(_bluePin, HIGH);
#endif
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

        case STATE_AP_MODE:
            if (currentMillis - _lastToggleTime >= 1000) {
                _blinkState = !_blinkState;
                _lastToggleTime = currentMillis;
            }
            writePins(_blinkState, false, _blinkState); // purple slow blink
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

/**
 * @brief Sets the LED brightness (0-255) applied when a pin is active.
 */
void LedManager::setBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;
    _brightness = brightness;
}

/**
 * @brief Returns the current LED brightness value (0-255).
 */
int LedManager::getBrightness() const {
    return _brightness;
}
