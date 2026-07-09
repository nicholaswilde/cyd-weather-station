#include "Arduino.h"
#include "WiFi.h"

SerialMock Serial;
WiFiMock WiFi;

unsigned long mock_millis_val = 0;
uint8_t mock_pin_modes[100] = {0};
uint8_t mock_pin_states[100] = {0};
