#ifndef BACKLIGHT_MANAGER_H
#define BACKLIGHT_MANAGER_H

#include <stdint.h>

#define LDR_PIN 34

class BacklightManager {
public:
    // Parameters:
    // - pin: GPIO pin controlling the backlight (default 21 for CYD)
    // - channel: LEDC PWM channel (0-15)
    // - minBrightnessPercent: minimum backlight brightness floor (e.g., 10%)
    BacklightManager(uint8_t pin, uint8_t channel = 0, float minBrightnessPercent = 10.0f);

    // Initializer (configures PWM peripheral if not running in native test mode)
    void begin();

    // Periodically updates ambient light and calculates smoothed backlight level (0-255)
    // Parameters:
    // - rawAdcValue: raw LDR analog input (0 to 4095)
    uint8_t update(uint16_t rawAdcValue);

    // Explicit manual backlight setting (0 to 100 percent)
    // Sets backlight directly (bypassing auto-dimming calculation)
    void setManualBrightness(uint8_t percent);

    // Getters for status & testing
    uint8_t getDutyCycle() const { return _currentDuty; }
    float getFilteredLight() const { return _filteredLight; }

private:
    uint8_t _pin;
    uint8_t _channel;
    float _minBrightnessPercent;
    uint8_t _currentDuty;
    float _filteredLight;
    bool _isFirstReading;
};

#endif // BACKLIGHT_MANAGER_H
