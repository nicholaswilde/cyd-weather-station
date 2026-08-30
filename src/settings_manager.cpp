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
    _themeFlavor = DEFAULT_THEME_FLAVOR;
    _sdLoggingEnabled = USE_SD_LOGGING;
    _screenshotServerEnabled = false; // Default to false
    _apiServerEnabled = API_SERVER_ENABLED;
    _screenOrientation = 1;
    _ledEnabled = LED_ENABLED;
    _ledBrightness = LED_BRIGHTNESS;
    _mqttEnabled = MQTT_ENABLED;
    _mqttServer = MQTT_SERVER;
    _mqttPort = MQTT_PORT;
    _mqttUser = MQTT_USER;
    _mqttPassword = MQTT_PASSWORD;
    _mqttBaseTopic = "cyd/";
    _wifiSSID = WIFI_SSID;
    _wifiPassword = WIFI_PASSWORD;
    _sdCacheEnabled = USE_SD_CACHE;
    _screensaverEnabled = SCREENSAVER_ENABLED;
    _screensaverTimeout = SCREENSAVER_TIMEOUT_MS;
    _staticIpEnabled = false;
    _staticIp = "";
    _staticGateway = "";
    _staticSubnet = "255.255.255.0";
    _staticDns = "1.1.1.1";
#ifdef AP_PASSWORD
    _apPassword = AP_PASSWORD;
#else
    _apPassword = "";
#endif
    _zipCode = WEATHER_ZIP_CODE;
    _cityCode = WEATHER_CITY_CODE;
    _latitude = WEATHER_API_LATITUDE;
    _longitude = WEATHER_API_LONGITUDE;
    _owmApiKey = OPENWEATHERMAP_API_KEY;
    _ntpServer = NTP_SERVER;
    _weatherUpdateInterval = WEATHER_UPDATE_INTERVAL_MINS;
    _localSensorEnabled = LOCAL_SENSOR_ENABLED;
    _localSensorType = LOCAL_SENSOR_TYPE;
    _localSensorUpdateInterval = LOCAL_SENSOR_UPDATE_INTERVAL;
    _localSensorTempOffset = LOCAL_SENSOR_TEMP_OFFSET;
    _localSensorHumOffset = LOCAL_SENSOR_HUM_OFFSET;
    _sleepScheduleEnabled = DEFAULT_SLEEP_SCHEDULE_ENABLED;
    _sleepStartTime = DEFAULT_SLEEP_START_TIME;
    _sleepEndTime = DEFAULT_SLEEP_END_TIME;
    _use24HourFormat = USE_24_HOUR_FORMAT;
}

void SettingsManager::begin() {
    Preferences prefs;
    prefs.begin("settings", false);
    
    _unitSystem = prefs.getInt("unit", UNIT_SYSTEM);
    _brightness = prefs.getInt("bright", 80);
    _autoBrightness = prefs.getBool("auto_bright", USE_LDR_AUTO_BACKLIGHT);
    _timezone = prefs.getString("tz", TIMEZONE_DEFAULT);
    _themeFlavor = prefs.getInt("theme", DEFAULT_THEME_FLAVOR);
    _sdLoggingEnabled = prefs.getBool("sd_log", USE_SD_LOGGING);
    _screenshotServerEnabled = prefs.getBool("scr_srv", false);
    _apiServerEnabled = prefs.getBool("api_srv", API_SERVER_ENABLED);
    _screenOrientation = prefs.getInt("screen_rot", 1);
    _ledEnabled = prefs.getBool("led_en", LED_ENABLED);
    _ledBrightness = prefs.getInt("led_bright", LED_BRIGHTNESS);
    _mqttEnabled = prefs.getBool("mqtt_en", MQTT_ENABLED);
    _mqttServer = prefs.getString("mqtt_srv", MQTT_SERVER);
    _mqttPort = prefs.getInt("mqtt_prt", MQTT_PORT);
    _mqttUser = prefs.getString("mqtt_usr", MQTT_USER);
    _mqttPassword = prefs.getString("mqtt_pwd", MQTT_PASSWORD);
    _mqttBaseTopic = prefs.getString("mqtt_base", "cyd/");
    _wifiSSID = prefs.getString("wifi_ssid", WIFI_SSID);
    _wifiPassword = prefs.getString("wifi_pass", WIFI_PASSWORD);
    _sdCacheEnabled = prefs.getBool("sd_cache", USE_SD_CACHE);
    _screensaverEnabled = prefs.getBool("scr_enabled", SCREENSAVER_ENABLED);
    _screensaverTimeout = prefs.getInt("scr_timeout", SCREENSAVER_TIMEOUT_MS);
    _staticIpEnabled = prefs.getBool("static_en", false);
    _staticIp = prefs.getString("static_ip", "");
    _staticGateway = prefs.getString("static_gw", "");
    _staticSubnet = prefs.getString("static_sn", "255.255.255.0");
    _staticDns = prefs.getString("static_dns", "1.1.1.1");
#ifdef AP_PASSWORD
    _apPassword = prefs.getString("ap_pass", AP_PASSWORD);
#else
    _apPassword = prefs.getString("ap_pass", "");
#endif
    _zipCode = prefs.getString("zip", WEATHER_ZIP_CODE);
    _cityCode = prefs.getString("city", WEATHER_CITY_CODE);
    _latitude = prefs.getString("lat", WEATHER_API_LATITUDE);
    _longitude = prefs.getString("lon", WEATHER_API_LONGITUDE);
    _owmApiKey = prefs.getString("owm_api", OPENWEATHERMAP_API_KEY);
    _ntpServer = prefs.getString("ntp_srv", NTP_SERVER);
    _weatherUpdateInterval = prefs.getInt("upd_int", WEATHER_UPDATE_INTERVAL_MINS);
    _localSensorEnabled = prefs.getBool("loc_sens_en", LOCAL_SENSOR_ENABLED);
    _localSensorType = prefs.getInt("loc_sens_typ", LOCAL_SENSOR_TYPE);
    _localSensorUpdateInterval = prefs.getInt("loc_sens_upd", LOCAL_SENSOR_UPDATE_INTERVAL);
    _localSensorTempOffset = prefs.getFloat("loc_sens_toff", LOCAL_SENSOR_TEMP_OFFSET);
    _localSensorHumOffset = prefs.getFloat("loc_sens_hoff", LOCAL_SENSOR_HUM_OFFSET);
    _sleepScheduleEnabled = prefs.getBool("sleep_sched", DEFAULT_SLEEP_SCHEDULE_ENABLED);
    _sleepStartTime = prefs.getString("sleep_start", DEFAULT_SLEEP_START_TIME);
    _sleepEndTime = prefs.getString("sleep_end", DEFAULT_SLEEP_END_TIME);
    _use24HourFormat = prefs.getBool("use_24h", USE_24_HOUR_FORMAT);
    
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

bool SettingsManager::getApiServerEnabled() const {
    return _apiServerEnabled;
}

void SettingsManager::setApiServerEnabled(bool enabled) {
    if (_apiServerEnabled != enabled) {
        _apiServerEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("api_srv", _apiServerEnabled);
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

int SettingsManager::getScreensaverTimeout() const {
    return _screensaverTimeout;
}

void SettingsManager::setScreensaverTimeout(int timeout) {
    if (_screensaverTimeout != timeout) {
        _screensaverTimeout = timeout;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("scr_timeout", _screensaverTimeout);
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

const String& SettingsManager::getMqttBaseTopic() const {
    return _mqttBaseTopic;
}

void SettingsManager::setMqttBaseTopic(const String& baseTopic) {
    if (_mqttBaseTopic != baseTopic) {
        _mqttBaseTopic = baseTopic;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("mqtt_base", _mqttBaseTopic); prefs.end();
    }
}

int SettingsManager::getWeatherUpdateInterval() const {
    return _weatherUpdateInterval;
}

void SettingsManager::setWeatherUpdateInterval(int interval) {
    if (interval < 1) interval = 1;
    if (_weatherUpdateInterval != interval) {
        _weatherUpdateInterval = interval;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("upd_int", _weatherUpdateInterval);
        prefs.end();
    }
}

bool SettingsManager::getStaticIpEnabled() const { return _staticIpEnabled; }
void SettingsManager::setStaticIpEnabled(bool enabled) {
    if (_staticIpEnabled != enabled) {
        _staticIpEnabled = enabled;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putBool("static_en", _staticIpEnabled); prefs.end();
    }
}
const String& SettingsManager::getStaticIp() const { return _staticIp; }
void SettingsManager::setStaticIp(const String& ip) {
    if (_staticIp != ip) {
        _staticIp = ip;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_ip", _staticIp); prefs.end();
    }
}
const String& SettingsManager::getStaticGateway() const { return _staticGateway; }
void SettingsManager::setStaticGateway(const String& gateway) {
    if (_staticGateway != gateway) {
        _staticGateway = gateway;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_gw", _staticGateway); prefs.end();
    }
}
const String& SettingsManager::getStaticSubnet() const { return _staticSubnet; }
void SettingsManager::setStaticSubnet(const String& subnet) {
    if (_staticSubnet != subnet) {
        _staticSubnet = subnet;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_sn", _staticSubnet); prefs.end();
    }
}
const String& SettingsManager::getStaticDns() const { return _staticDns; }
void SettingsManager::setStaticDns(const String& dns) {
    if (_staticDns != dns) {
        _staticDns = dns;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("static_dns", _staticDns); prefs.end();
    }
}
const String& SettingsManager::getApPassword() const { return _apPassword; }
void SettingsManager::setApPassword(const String& password) {
    if (_apPassword != password) {
        _apPassword = password;
        Preferences prefs; prefs.begin("settings", false);
        prefs.putString("ap_pass", _apPassword); prefs.end();
    }
}

void SettingsManager::factoryReset() {
    Preferences prefs;
    prefs.begin("settings", false);
    prefs.clear();
    prefs.end();
}

bool SettingsManager::getLocalSensorEnabled() const {
    return _localSensorEnabled;
}

void SettingsManager::setLocalSensorEnabled(bool enabled) {
    if (_localSensorEnabled != enabled) {
        _localSensorEnabled = enabled;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("loc_sens_en", _localSensorEnabled);
        prefs.end();
    }
}

int SettingsManager::getLocalSensorType() const {
    return _localSensorType;
}

void SettingsManager::setLocalSensorType(int type) {
    if (_localSensorType != type) {
        _localSensorType = type;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("loc_sens_typ", _localSensorType);
        prefs.end();
    }
}

int SettingsManager::getLocalSensorUpdateInterval() const {
    return _localSensorUpdateInterval;
}

void SettingsManager::setLocalSensorUpdateInterval(int interval) {
    if (interval < 1) interval = 1;
    if (_localSensorUpdateInterval != interval) {
        _localSensorUpdateInterval = interval;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putInt("loc_sens_upd", _localSensorUpdateInterval);
        prefs.end();
    }
}

float SettingsManager::getLocalSensorTempOffset() const {
    return _localSensorTempOffset;
}

void SettingsManager::setLocalSensorTempOffset(float offset) {
    if (_localSensorTempOffset != offset) {
        _localSensorTempOffset = offset;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putFloat("loc_sens_toff", _localSensorTempOffset);
        prefs.end();
    }
}

float SettingsManager::getLocalSensorHumOffset() const {
    return _localSensorHumOffset;
}

void SettingsManager::setLocalSensorHumOffset(float offset) {
    if (_localSensorHumOffset != offset) {
        _localSensorHumOffset = offset;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putFloat("loc_sens_hoff", _localSensorHumOffset);
        prefs.end();
    }
}


bool SettingsManager::getSleepScheduleEnabled() const { return _sleepScheduleEnabled; }
void SettingsManager::setSleepScheduleEnabled(bool enabled) { 
    if (_sleepScheduleEnabled != enabled) {
        _sleepScheduleEnabled = enabled; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("sleep_sched", _sleepScheduleEnabled);
        prefs.end();
    }
}

const String& SettingsManager::getSleepStartTime() const { return _sleepStartTime; }
void SettingsManager::setSleepStartTime(const String& startTime) { 
    if (_sleepStartTime != startTime) {
        _sleepStartTime = startTime; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("sleep_start", _sleepStartTime);
        prefs.end();
    }
}

const String& SettingsManager::getSleepEndTime() const { return _sleepEndTime; }
void SettingsManager::setSleepEndTime(const String& endTime) { 
    if (_sleepEndTime != endTime) {
        _sleepEndTime = endTime; 
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putString("sleep_end", _sleepEndTime);
        prefs.end();
    }
}

bool SettingsManager::getUse24HourFormat() const { return _use24HourFormat; }
void SettingsManager::setUse24HourFormat(bool use24HourFormat) {
    if (_use24HourFormat != use24HourFormat) {
        _use24HourFormat = use24HourFormat;
        Preferences prefs;
        prefs.begin("settings", false);
        prefs.putBool("use_24h", _use24HourFormat);
        prefs.end();
    }
}
