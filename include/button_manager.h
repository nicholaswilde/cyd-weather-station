#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <stdint.h>

enum class ButtonAction {
    NONE,
    SINGLE_PRESS,
    LONG_PRESS
};

class ButtonManager {
public:
    ButtonManager(uint8_t pin, uint32_t debounceDelayMs = 50, uint32_t longPressDelayMs = 2000);

    void begin();
    ButtonAction update(unsigned long currentMillis);
    bool isPressed() const;

private:
    uint8_t _pin;
    uint32_t _debounceDelay;
    uint32_t _longPressDelay;

    uint8_t _lastPinState;
    uint8_t _stableState;
    unsigned long _lastTransitionTime;
    unsigned long _pressStartTime;
    bool _longPressTriggered;
};

#endif // BUTTON_MANAGER_H
