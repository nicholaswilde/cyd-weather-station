#include "settings_manager.h"
#include <Preferences.h>
#include "config/config.h"

SettingsManager::SettingsManager() {
    _unitSystem = UNIT_SYSTEM;
    _brightness = 80;
    _autoBrightness = USE_LDR_AUTO_BACKLIGHT;
    _timezoneOffset = GMT_OFFSET_SEC / 3600;
    _dstEnabled = (DST_OFFSET_SEC > 0);
    _themeFlavor = CATPPUCCIN_MOCHA;
    _sdLoggingEnabled = USE_SD_LOGGING;
    _screenshotServerEnabled = true;
    _screenOrientation = 1;
    _ledEnabled = LED_ENABLED;
    _ledBrightness = LED_BRIGHTNESS;
    _wifiSSID = WIFI_SSID;
    _wifiPassword = WIFI_PASSWORD;
}

void SettingsManager::begin() {
    Preferences prefs;
    prefs.begin("settings", false);
    
    _unitSystem = prefs.getInt("unit", UNIT_SYSTEM);
    _brightness = prefs.getInt("bright", 80);
    _autoBrightness = prefs.getBool("auto_bright", USE_LDR_AUTO_BACKLIGHT);
    _timezoneOffset = prefs.getInt("tz_offset", GMT_OFFSET_SEC / 3600);
    _dstEnabled = prefs.getBool("dst_enabled", DST_OFFSET_SEC > 0);
    _themeFlavor = prefs.getInt("theme", CATPPUCCIN_MOCHA);
    _sdLoggingEnabled = prefs.getBool("sd_log", USE_SD_LOGGING);
    _screenshotServerEnabled = prefs.getBool("scr_srv", true);
    _screenOrientation = prefs.getInt("screen_rot", 1);
    _ledEnabled = prefs.getBool("led_en", LED_ENABLED);
    _ledBrightness = prefs.getInt("led_bright", LED_BRIGHTNESS);
    _wifiSSID = prefs.getString("wifi_ssid", WIFI_SSID);
    _wifiPassword = prefs.getString("wifi_pass", WIFI_PASSWORD);
    
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

bool SettingsManager::getDstEnabled() const {
    return _dstEnabled;
}

void SettingsManager::setDstEnabled(bool enabled) {
    if (_dstEnabled != enabled) {
        _dstEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("dst_enabled", _dstEnabled);
        prefs.end();
    }
}

int SettingsManager::getThemeFlavor() const {
    return _themeFlavor;
}

void SettingsManager::setThemeFlavor(int flavor) {
    if (flavor < 1) flavor = 1;
    if (flavor > 4) flavor = 4;

    if (_themeFlavor != flavor) {
        _themeFlavor = flavor;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("theme", _themeFlavor);
        prefs.end();
    }
}

String SettingsManager::getWifiSSID() const {
    return _wifiSSID;
}

void SettingsManager::setWifiSSID(const String& ssid) {
    if (_wifiSSID != ssid) {
        _wifiSSID = ssid;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("wifi_ssid", _wifiSSID);
        prefs.end();
    }
}

String SettingsManager::getWifiPassword() const {
    return _wifiPassword;
}

void SettingsManager::setWifiPassword(const String& password) {
    if (_wifiPassword != password) {
        _wifiPassword = password;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("wifi_pass", _wifiPassword);
        prefs.end();
    }
}

bool SettingsManager::getSdLoggingEnabled() const {
    return _sdLoggingEnabled;
}

void SettingsManager::setSdLoggingEnabled(bool enabled) {
    if (_sdLoggingEnabled != enabled) {
        _sdLoggingEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("sd_log", _sdLoggingEnabled);
        prefs.end();
    }
}

bool SettingsManager::getScreenshotServerEnabled() const {
    return _screenshotServerEnabled;
}

void SettingsManager::setScreenshotServerEnabled(bool enabled) {
    if (_screenshotServerEnabled != enabled) {
        _screenshotServerEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("scr_srv", _screenshotServerEnabled);
        prefs.end();
    }
}

int SettingsManager::getScreenOrientation() const {
    return _screenOrientation;
}

void SettingsManager::setScreenOrientation(int orientation) {
    if (orientation < 0) orientation = 0;
    if (orientation > 3) orientation = 3;

    if (_screenOrientation != orientation) {
        _screenOrientation = orientation;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("screen_rot", _screenOrientation);
        prefs.end();
    }
}

bool SettingsManager::getLedEnabled() const {
    return _ledEnabled;
}

void SettingsManager::setLedEnabled(bool enabled) {
    if (_ledEnabled != enabled) {
        _ledEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("led_en", _ledEnabled);
        prefs.end();
    }
}

int SettingsManager::getLedBrightness() const {
    return _ledBrightness;
}

void SettingsManager::setLedBrightness(int brightness) {
    if (brightness < 0) brightness = 0;
    if (brightness > 255) brightness = 255;

    if (_ledBrightness != brightness) {
        _ledBrightness = brightness;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("led_bright", _ledBrightness);
        prefs.end();
    }
}
