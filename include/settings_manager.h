#ifndef SETTINGS_MANAGER_H
#define SETTINGS_MANAGER_H

class SettingsManager {
private:
    int _unitSystem;
    int _brightness;
    bool _autoBrightness;
    int _timezoneOffset;

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
};

#endif // SETTINGS_MANAGER_H
