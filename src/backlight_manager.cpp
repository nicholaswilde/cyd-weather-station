#include "backlight_manager.h"
#include <math.h>

#ifndef NATIVE_TEST
#include <Arduino.h>
#endif

BacklightManager::BacklightManager(uint8_t pin, uint8_t channel, float minBrightnessPercent)
    : _pin(pin), _channel(channel), _minBrightnessPercent(minBrightnessPercent), _currentDuty(0), _filteredLight(0.0f), _isFirstReading(true) {}

void BacklightManager::begin() {
#ifndef NATIVE_TEST
    // Configure LDR pin as analog input
    pinMode(LDR_PIN, INPUT);

    // Configure TFT_BL pin for LEDC PWM
    #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcAttach(_pin, 5000, 8); // 5000 Hz, 8-bit resolution
    #else
        ledcSetup(_channel, 5000, 8);
        ledcAttachPin(_pin, _channel);
    #endif

    // Set initial brightness to full
    setManualBrightness(100);
#endif
}

uint8_t BacklightManager::update(uint16_t rawAdcValue) {
    // 1. Apply EMA Filter
    if (_isFirstReading) {
        _filteredLight = rawAdcValue;
        _isFirstReading = false;
    } else {
        const float alpha = 0.1f;
        _filteredLight = (alpha * rawAdcValue) + ((1.0f - alpha) * _filteredLight);
    }

    // 2. Map Filtered Light to Duty Cycle (minDuty to 255)
    uint8_t minDuty = (uint8_t)(_minBrightnessPercent / 100.0f * 255.0f + 0.5f);
    float duty = minDuty + (_filteredLight / 4095.0f) * (255 - minDuty);
    _currentDuty = (uint8_t)(duty + 0.5f);

    // Constrain duty cycle boundaries
    if (_currentDuty < minDuty) _currentDuty = minDuty;
    if (_currentDuty > 255) _currentDuty = 255;

#ifndef NATIVE_TEST
    // Write duty cycle to hardware
    #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(_pin, _currentDuty);
    #else
        ledcWrite(_channel, _currentDuty);
    #endif
#endif

    return _currentDuty;
}

void BacklightManager::setManualBrightness(uint8_t percent) {
    // Constrain percent to valid range
    float minPercent = _minBrightnessPercent;
    float finalPercent = (float)percent;
    if (finalPercent < minPercent) finalPercent = minPercent;
    if (finalPercent > 100.0f) finalPercent = 100.0f;

    // Convert percent (minPercent to 100) to duty cycle (0 to 255)
    float duty = (finalPercent / 100.0f) * 255.0f;
    _currentDuty = (uint8_t)(duty + 0.5f);

#ifndef NATIVE_TEST
    #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(_pin, _currentDuty);
    #else
        ledcWrite(_channel, _currentDuty);
    #endif
#endif
}
