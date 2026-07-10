#include "Arduino.h"
#include "WiFi.h"
#include "Preferences.h"
#include "settings_manager.h"
#include "../../src/settings_manager.cpp"

SerialMock Serial;
WiFiMock WiFi;
SettingsManager settings;

unsigned long mock_millis_val = 0;
uint8_t mock_pin_modes[100] = {0};
uint8_t mock_pin_states[100] = {0};
