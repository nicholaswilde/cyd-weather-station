#include "screensaver_manager.h"

#ifndef NATIVE_TEST
#include <Arduino.h>
#include "ui.h"
#endif

ScreenSaverManager::ScreenSaverManager(BacklightManager& backlight, uint32_t timeoutMs)
    : _backlight(backlight), _timeoutMs(timeoutMs), _lastActivityTime(0), _active(false) {}

void ScreenSaverManager::begin() {
#ifndef NATIVE_TEST
    _lastActivityTime = millis();
#else
    _lastActivityTime = 0;
#endif
    _active = false;
}

void ScreenSaverManager::feedActivity() {
#ifndef NATIVE_TEST
    _lastActivityTime = millis();
#else
    _lastActivityTime = 0;
#endif
}

void ScreenSaverManager::update(uint32_t currentMillis) {
    if (_active) return;

    if (currentMillis - _lastActivityTime >= _timeoutMs) {
        sleep();
    }
}

void ScreenSaverManager::sleep() {
    if (_active) return;
    _active = true;
    
    // Dim backlight to 5%
    _backlight.fadeTo(5, 500);

#ifndef NATIVE_TEST
    showScreenSaver();
#endif
}

void ScreenSaverManager::wake(uint8_t targetBrightness) {
    if (!_active) return;
    _active = false;
    feedActivity();
    
    // Restore backlight to target
    _backlight.fadeTo(targetBrightness, 200);

#ifndef NATIVE_TEST
    hideScreenSaver();
#endif
}
