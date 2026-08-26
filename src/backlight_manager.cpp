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

uint8_t BacklightManager::percentToDuty(float percent) {
    if (percent < _minBrightnessPercent) percent = _minBrightnessPercent;
    if (percent > 100.0f) percent = 100.0f;

    float normalizedPercent = (percent - _minBrightnessPercent) / (100.0f - _minBrightnessPercent);
    if (normalizedPercent < 0.0f) normalizedPercent = 0.0f;
    
    uint8_t minDuty = (uint8_t)((_minBrightnessPercent / 100.0f) * 255.0f + 0.5f);
    float gammaFactor = pow(normalizedPercent, 2.2f);
    
    float duty = minDuty + gammaFactor * (255.0f - minDuty);
    return (uint8_t)(duty + 0.5f);
}

float BacklightManager::dutyToPercent(uint8_t duty) {
    uint8_t minDuty = (uint8_t)((_minBrightnessPercent / 100.0f) * 255.0f + 0.5f);
    if (duty <= minDuty) return _minBrightnessPercent;
    if (duty >= 255) return 100.0f;
    
    float gammaFactor = (float)(duty - minDuty) / (255.0f - minDuty);
    float normalizedPercent = pow(gammaFactor, 1.0f / 2.2f);
    
    return _minBrightnessPercent + normalizedPercent * (100.0f - _minBrightnessPercent);
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

    // 2. Map Filtered Light to Duty Cycle
    float lightPercent = (_filteredLight / 4095.0f) * 100.0f;
    _currentDuty = percentToDuty(lightPercent);

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
    _currentDuty = percentToDuty((float)percent);

#ifndef NATIVE_TEST
    #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
        ledcWrite(_pin, _currentDuty);
    #else
        ledcWrite(_channel, _currentDuty);
    #endif
#endif
}

void BacklightManager::fadeTo(uint8_t targetPercent, uint16_t durationMs) {
    float startPercent = dutyToPercent(_currentDuty);
    float endPercent = (float)targetPercent;
    
    if (endPercent < _minBrightnessPercent) endPercent = _minBrightnessPercent;
    if (endPercent > 100.0f) endPercent = 100.0f;

    float diff = endPercent - startPercent;
    // Handle floating point precision and small differences
    if (fabs(diff) < 0.5f) {
        setManualBrightness(targetPercent);
        return;
    }

    int steps = 15;
    uint32_t stepDelay = durationMs / steps;
    if (stepDelay < 1) stepDelay = 1;

    for (int i = 1; i <= steps; ++i) {
        float progress = (float)i / steps;
        float currentStepPercent = startPercent + (progress * diff);
        _currentDuty = percentToDuty(currentStepPercent);
#ifndef NATIVE_TEST
        #if defined(ESP_ARDUINO_VERSION) && ESP_ARDUINO_VERSION >= ESP_ARDUINO_VERSION_VAL(3, 0, 0)
            ledcWrite(_pin, _currentDuty);
        #else
            ledcWrite(_channel, _currentDuty);
        #endif
        delay(stepDelay);
#endif
    }
}
