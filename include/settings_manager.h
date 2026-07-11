#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

class SettingsManager {
private:
    int _unitSystem;
    int _brightness;
    bool _autoBrightness;
    int _timezoneOffset;
    int _themeFlavor;
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

    int getThemeFlavor() const;
    void setThemeFlavor(int flavor);

    String getWifiSSID() const;
    void setWifiSSID(const String& ssid);

    String getWifiPassword() const;
    void setWifiPassword(const String& password);
};

#endif // SETTINGS_MANAGER_H
