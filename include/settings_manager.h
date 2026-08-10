#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

#ifndef NATIVE_TEST
#include <Arduino.h>
#else
#include "Arduino.h"
#endif

class SettingsManager {
private:
    int _unitSystem;
    int _brightness;
    bool _autoBrightness;
    String _timezone;
    int _themeFlavor;
    bool _sdLoggingEnabled;
    bool _screenshotServerEnabled;
    int _screenOrientation;
    bool _ledEnabled;
    int _ledBrightness;
    bool _mqttEnabled;
    String _mqttServer;
    int _mqttPort;
    String _mqttUser;
    String _mqttPassword;
    String _wifiSSID;
    String _wifiPassword;
    bool _sdCacheEnabled;
    bool _screensaverEnabled;
    String _zipCode;
    String _cityCode;
    String _latitude;
    String _longitude;
    String _owmApiKey;
    String _ntpServer;

public:
    SettingsManager();
    void begin();
    
    int getUnitSystem() const;
    void setUnitSystem(int unitSystem);
    
    int getBrightness() const;
    void setBrightness(int brightness);
    
    bool getAutoBrightness() const;
    void setAutoBrightness(bool autoBrightness);
    
    const String& getTimezone() const;
    void setTimezone(const String& timezone);
    int getThemeFlavor() const;
    void setThemeFlavor(int flavor);

    bool getSdLoggingEnabled() const;
    void setSdLoggingEnabled(bool enabled);

    bool getScreenshotServerEnabled() const;
    void setScreenshotServerEnabled(bool enabled);

    int getScreenOrientation() const;
    void setScreenOrientation(int orientation);

    bool getLedEnabled() const;
    void setLedEnabled(bool enabled);

    int getLedBrightness() const;
    void setLedBrightness(int brightness);

    bool getMqttEnabled() const;
    void setMqttEnabled(bool enabled);

    const String& getMqttServer() const;
    void setMqttServer(const String& server);

    int getMqttPort() const;
    void setMqttPort(int port);

    const String& getMqttUser() const;
    void setMqttUser(const String& user);

    const String& getMqttPassword() const;
    void setMqttPassword(const String& password);

    const String& getWifiSSID() const;
    void setWifiSSID(const String& ssid);

    const String& getWifiPassword() const;
    void setWifiPassword(const String& password);

    bool getSdCacheEnabled() const;
    void setSdCacheEnabled(bool enabled);

    bool getScreensaverEnabled() const;
    void setScreensaverEnabled(bool enabled);

    const String& getZipCode() const;
    void setZipCode(const String& zipCode);

    const String& getCityCode() const;
    void setCityCode(const String& cityCode);

    const String& getLatitude() const;
    void setLatitude(const String& latitude);

    const String& getLongitude() const;
    void setLongitude(const String& longitude);

    const String& getOwmApiKey() const;
    void setOwmApiKey(const String& apiKey);

    const String& getNtpServer() const;
    void setNtpServer(const String& ntpServer);
};

#endif // SETTINGS_MANAGER_H
