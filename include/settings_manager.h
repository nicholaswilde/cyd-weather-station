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
    int _timezoneOffset;
    bool _dstEnabled;
    int _themeFlavor;
    bool _sdLoggingEnabled;
    bool _screenshotServerEnabled;
    int _screenOrientation;
    bool _ledEnabled;
    int _ledBrightness;
    bool _mqttEnabled;
    String _wifiSSID;
    String _wifiPassword;
    bool _sdCacheEnabled;
    bool _screensaverEnabled;

public:
    SettingsManager();
    void begin();
    
    int getUnitSystem() const;
    void setUnitSystem(int unitSystem);
    
    int getBrightness() const;
    void setBrightness(int brightness);
    
    bool getAutoBrightness() const;
    void setAutoBrightness(bool autoBrightness);
    
    int getTimezoneOffset() const;
    void setTimezoneOffset(int offset);
    
    bool getDstEnabled() const;
    void setDstEnabled(bool enabled);

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

    String getWifiSSID() const;
    void setWifiSSID(const String& ssid);

    String getWifiPassword() const;
    void setWifiPassword(const String& password);

    bool getSdCacheEnabled() const;
    void setSdCacheEnabled(bool enabled);

    bool getScreensaverEnabled() const;
    void setScreensaverEnabled(bool enabled);
};

#endif // SETTINGS_MANAGER_H
