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
    
    // AP / Captive Portal Mock additions
    bool softAP(const char* ssid, const char* passphrase = NULL, int channel = 1, int ssid_hidden = 0, int max_connection = 4) { return true; }
    IPAddress softAPIP() { return IPAddress(); }
    String macAddress() { return "AA:BB:CC:DD:EE:FF"; }
    int scanNetworks() { return 2; }
    String SSID(int i) { return i == 0 ? "WiFi_1" : "WiFi_2"; }
    int32_t RSSI(int i) { return -50 - i * 10; }
    uint8_t encryptionType(int i) { return 3; } // WIFI_AUTH_WPA2_PSK or similar dummy

    // Test helper to inject status
    wl_status_t _status = 0;
};

extern WiFiMock WiFi;

#endif // WIFI_MOCK_H
