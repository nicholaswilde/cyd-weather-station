#ifndef WIFI_MOCK_H
#define WIFI_MOCK_H

#include "Arduino.h"

class WiFiMock {
public:
    void mode(int m) {}
    void begin(const char* ssid, const char* password) {}
    wl_status_t status() { return _status; }
    IPAddress localIP() { return IPAddress(); }
    int8_t RSSI() { return -55; }

    // Test helper to inject status
    wl_status_t _status = 0;
};

extern WiFiMock WiFi;

#endif // WIFI_MOCK_H
