#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

enum WifiState {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED
};

class WifiManager {
public:
    WifiManager(const char* ssid, const char* password);
    void begin();
    void update();
    WifiState getState() const;
    String getIPAddress() const;
    int8_t getRSSI() const;

private:
    const char* _ssid;
    const char* _password;
    WifiState _state;
    unsigned long _lastReconnectAttempt;
    const unsigned long _reconnectInterval = 10000; // 10 seconds
};

#endif // WIFI_MANAGER_H
