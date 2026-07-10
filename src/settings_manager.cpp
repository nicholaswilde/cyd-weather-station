#include "settings_manager.h"
#include <Preferences.h>
#include "config/config.h"

SettingsManager::SettingsManager() {
    _unitSystem = UNIT_SYSTEM;
    _brightness = 80;
    _autoBrightness = USE_LDR_AUTO_BACKLIGHT;
    _timezoneOffset = GMT_OFFSET_SEC / 3600;
}

void SettingsManager::begin() {
    Preferences prefs;
    prefs.begin("settings", false);
    
    _unitSystem = prefs.getInt("unit", UNIT_SYSTEM);
    _brightness = prefs.getInt("bright", 80);
    _autoBrightness = prefs.getBool("auto_bright", USE_LDR_AUTO_BACKLIGHT);
    _timezoneOffset = prefs.getInt("tz_offset", GMT_OFFSET_SEC / 3600);
    
    prefs.end();
}

int SettingsManager::getUnitSystem() const {
    return _unitSystem;
}

void SettingsManager::setUnitSystem(int unitSystem) {
    if (_unitSystem != unitSystem) {
        _unitSystem = unitSystem;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("unit", _unitSystem);
        prefs.end();
    }
}

int SettingsManager::getBrightness() const {
    return _brightness;
}

void SettingsManager::setBrightness(int brightness) {
    if (brightness < 10) brightness = 10;
    if (brightness > 100) brightness = 100;
    
    if (_brightness != brightness) {
        _brightness = brightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("bright", _brightness);
        prefs.end();
    }
}

bool SettingsManager::getAutoBrightness() const {
    return _autoBrightness;
}

void SettingsManager::setAutoBrightness(bool autoBrightness) {
    if (_autoBrightness != autoBrightness) {
        _autoBrightness = autoBrightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("auto_bright", _autoBrightness);
        prefs.end();
    }
}

int SettingsManager::getTimezoneOffset() const {
    return _timezoneOffset;
}

void SettingsManager::setTimezoneOffset(int offset) {
    if (offset < -12) offset = -12;
    if (offset > 14) offset = 14;

    if (_timezoneOffset != offset) {
        _timezoneOffset = offset;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("tz_offset", _timezoneOffset);
        prefs.end();
    }
}
