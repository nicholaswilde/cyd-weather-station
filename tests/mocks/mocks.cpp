#include "Arduino.h"
#include "WiFi.h"
#include "Preferences.h"
#include "SPI.h"
#include "SD.h"
#include "settings_manager.h"
#include "../../src/settings_manager.cpp"

SerialMock Serial;
WiFiMock WiFi;
SettingsManager settings;

SPIClass SPI(VSPI);
SDFS SD;
std::map<std::string, std::string> mock_files;
bool mock_sd_card_present = true;
bool mock_sd_card_mounted = false;

unsigned long mock_millis_val = 0;
uint8_t mock_pin_modes[100] = {0};
uint8_t mock_pin_states[100] = {0};
