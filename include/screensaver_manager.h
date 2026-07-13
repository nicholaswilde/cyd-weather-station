#ifndef SCREENSAVER_MANAGER_H
#define SCREENSAVER_MANAGER_H

#include <stdint.h>
#include "backlight_manager.h"

class ScreenSaverManager {
public:
    ScreenSaverManager(BacklightManager& backlight, uint32_t timeoutMs = 300000); // 5 minutes default

    void begin();

    // Call this whenever a touch event is detected to reset the timer
    void feedActivity();

    // Call this periodically in loop() to check timeout and transition state
    void update(uint32_t currentMillis);

    // Explicitly wake or sleep
    void wake(uint8_t targetBrightness = 80);
    void sleep();

    // Getters
    bool isActive() const { return _active; }
    uint32_t getLastActivityTime() const { return _lastActivityTime; }

private:
    BacklightManager& _backlight;
    uint32_t _timeoutMs;
    uint32_t _lastActivityTime;
    bool _active;
};

extern ScreenSaverManager screensaver;

#endif // SCREENSAVER_MANAGER_H
