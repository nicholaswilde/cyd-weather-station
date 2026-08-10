#include "settings_manager.h"
#include <Preferences.h>
#include "config/config.h"

#ifndef OPENWEATHERMAP_API_KEY
#define OPENWEATHERMAP_API_KEY ""
#endif

#ifndef MQTT_SERVER
#define MQTT_SERVER ""
#define MQTT_PORT 1883
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#endif

SettingsManager::SettingsManager() {
    _unitSystem = UNIT_SYSTEM;
    _brightness = 80;
    _autoBrightness = USE_LDR_AUTO_BACKLIGHT;
    _timezone = TIMEZONE_DEFAULT;
    _themeFlavor = CATPPUCCIN_MOCHA;
    _sdLoggingEnabled = USE_SD_LOGGING;
    _screenshotServerEnabled = API_SERVER_ENABLED;
    _screenOrientation = 1;
    _ledEnabled = LED_ENABLED;
    _ledBrightness = LED_BRIGHTNESS;
    _mqttEnabled = MQTT_ENABLED;
    _mqttServer = MQTT_SERVER;
    _mqttPort = MQTT_PORT;
    _mqttUser = MQTT_USER;
    _mqttPassword = MQTT_PASSWORD;
    _wifiSSID = WIFI_SSID;
    _wifiPassword = WIFI_PASSWORD;
    _sdCacheEnabled = USE_SD_CACHE;
    _screensaverEnabled = SCREENSAVER_ENABLED;
    _zipCode = WEATHER_ZIP_CODE;
    _cityCode = WEATHER_CITY_CODE;
    _latitude = WEATHER_API_LATITUDE;
    _longitude = WEATHER_API_LONGITUDE;
    _owmApiKey = OPENWEATHERMAP_API_KEY;
    _ntpServer = NTP_SERVER;
}

void SettingsManager::begin() {
    Preferences prefs;
    prefs.begin("settings", false);
    
    _unitSystem = prefs.getInt("unit", UNIT_SYSTEM);
    _brightness = prefs.getInt("bright", 80);
    _autoBrightness = prefs.getBool("auto_bright", USE_LDR_AUTO_BACKLIGHT);
    _timezone = prefs.getString("tz", TIMEZONE_DEFAULT);
    _themeFlavor = prefs.getInt("theme", CATPPUCCIN_MOCHA);
    _sdLoggingEnabled = prefs.getBool("sd_log", USE_SD_LOGGING);
    _screenshotServerEnabled = prefs.getBool("scr_srv", API_SERVER_ENABLED);
    _screenOrientation = prefs.getInt("screen_rot", 1);
    _ledEnabled = prefs.getBool("led_en", LED_ENABLED);
    _ledBrightness = prefs.getInt("led_bright", LED_BRIGHTNESS);
    _mqttEnabled = prefs.getBool("mqtt_en", MQTT_ENABLED);
    _mqttServer = prefs.getString("mqtt_srv", MQTT_SERVER);
    _mqttPort = prefs.getInt("mqtt_prt", MQTT_PORT);
    _mqttUser = prefs.getString("mqtt_usr", MQTT_USER);
    _mqttPassword = prefs.getString("mqtt_pwd", MQTT_PASSWORD);
    _wifiSSID = prefs.getString("wifi_ssid", WIFI_SSID);
    _wifiPassword = prefs.getString("wifi_pass", WIFI_PASSWORD);
    _sdCacheEnabled = prefs.getBool("sd_cache", USE_SD_CACHE);
    _screensaverEnabled = prefs.getBool("scr_enabled", SCREENSAVER_ENABLED);
    _zipCode = prefs.getString("zip", WEATHER_ZIP_CODE);
    _cityCode = prefs.getString("city", WEATHER_CITY_CODE);
    _latitude = prefs.getString("lat", WEATHER_API_LATITUDE);
    _longitude = prefs.getString("lon", WEATHER_API_LONGITUDE);
    _owmApiKey = prefs.getString("owm_api", OPENWEATHERMAP_API_KEY);
    _ntpServer = prefs.getString("ntp_srv", NTP_SERVER);
    
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

const String& SettingsManager::getTimezone() const {
    return _timezone;
}

void SettingsManager::setTimezone(const String& timezone) {
    if (_timezone != timezone) {
        _timezone = timezone;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("tz", _timezone);
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

const String& SettingsManager::getWifiSSID() const {
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

const String& SettingsManager::getWifiPassword() const {
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

bool SettingsManager::getMqttEnabled() const {
    return _mqttEnabled;
}

void SettingsManager::setMqttEnabled(bool enabled) {
    if (_mqttEnabled != enabled) {
        _mqttEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("mqtt_en", _mqttEnabled);
        prefs.end();
    }
}

bool SettingsManager::getSdCacheEnabled() const {
    return _sdCacheEnabled;
}

void SettingsManager::setSdCacheEnabled(bool enabled) {
    if (_sdCacheEnabled != enabled) {
        _sdCacheEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("sd_cache", _sdCacheEnabled);
        prefs.end();
    }
}

bool SettingsManager::getScreensaverEnabled() const {
    return _screensaverEnabled;
}

void SettingsManager::setScreensaverEnabled(bool enabled) {
    if (_screensaverEnabled != enabled) {
        _screensaverEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("scr_enabled", _screensaverEnabled);
        prefs.end();
    }
}

const String& SettingsManager::getZipCode() const {
    return _zipCode;
}

void SettingsManager::setZipCode(const String& zipCode) {
    if (_zipCode != zipCode) {
        _zipCode = zipCode;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("zip", _zipCode);
        prefs.end();
    }
}

const String& SettingsManager::getCityCode() const {
    return _cityCode;
}

void SettingsManager::setCityCode(const String& cityCode) {
    if (_cityCode != cityCode) {
        _cityCode = cityCode;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("city", _cityCode);
        prefs.end();
    }
}

const String& SettingsManager::getLatitude() const {
    return _latitude;
}

void SettingsManager::setLatitude(const String& latitude) {
    if (_latitude != latitude) {
        _latitude = latitude;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("lat", _latitude);
        prefs.end();
    }
}

const String& SettingsManager::getLongitude() const {
    return _longitude;
}

void SettingsManager::setLongitude(const String& longitude) {
    if (_longitude != longitude) {
        _longitude = longitude;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("lon", _longitude);
        prefs.end();
    }
}

const String& SettingsManager::getOwmApiKey() const {
    return _owmApiKey;
}

void SettingsManager::setOwmApiKey(const String& apiKey) {
    if (_owmApiKey != apiKey) {
        _owmApiKey = apiKey;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("owm_api", _owmApiKey);
        prefs.end();
    }
}

const String& SettingsManager::getNtpServer() const {
    return _ntpServer;
}

void SettingsManager::setNtpServer(const String& ntpServer) {
    if (_ntpServer != ntpServer) {
        _ntpServer = ntpServer;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("ntp_srv", _ntpServer);
        prefs.end();
    }
}

const String& SettingsManager::getMqttServer() const { return _mqttServer; }
void SettingsManager::setMqttServer(const String& server) {
    if (_mqttServer != server) {
        _mqttServer = server;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_srv", _mqttServer); prefs.end();
    }
}

int SettingsManager::getMqttPort() const { return _mqttPort; }
void SettingsManager::setMqttPort(int port) {
    if (_mqttPort != port) {
        _mqttPort = port;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putInt("mqtt_prt", _mqttPort); prefs.end();
    }
}

const String& SettingsManager::getMqttUser() const { return _mqttUser; }
void SettingsManager::setMqttUser(const String& user) {
    if (_mqttUser != user) {
        _mqttUser = user;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_usr", _mqttUser); prefs.end();
    }
}

const String& SettingsManager::getMqttPassword() const { return _mqttPassword; }
void SettingsManager::setMqttPassword(const String& password) {
    if (_mqttPassword != password) {
        _mqttPassword = password;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_pwd", _mqttPassword); prefs.end();
    }
}
