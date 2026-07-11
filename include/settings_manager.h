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
    String _wifiSSID;
    String _wifiPassword;

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

    String getWifiSSID() const;
    void setWifiSSID(const String& ssid);

    String getWifiPassword() const;
    void setWifiPassword(const String& password);
};

#endif // SETTINGS_MANAGER_H
