#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <WiFi.h>

#ifndef NATIVE_TEST
#include <DNSServer.h>
#include <WebServer.h>
#endif

enum WifiState {
    WIFI_STATE_DISCONNECTED,
    WIFI_STATE_CONNECTING,
    WIFI_STATE_CONNECTED,
    WIFI_STATE_AP_MODE
};

class WifiManager {
public:
    WifiManager(const char* ssid, const char* password);
    void begin();
    void update();
    WifiState getState() const;
    String getIPAddress() const;
    int8_t getRSSI() const;
    void setCredentials(const String& ssid, const String& password);
    String getAPSSID();

private:
    void startAPMode();
    void handleRoot();
    void handleSave();
    void handleNotFound();

    String _ssid;
    String _password;
    WifiState _state;
    unsigned long _lastReconnectAttempt;
    unsigned long _connectionStartTime;
    const unsigned long _reconnectInterval = 10000; // 10 seconds
    const unsigned long _connectionTimeout = 30000; // 30 seconds

#ifndef NATIVE_TEST
    DNSServer* _dnsServer = nullptr;
    WebServer* _webServer = nullptr;
#endif
};

#endif // WIFI_MANAGER_H
