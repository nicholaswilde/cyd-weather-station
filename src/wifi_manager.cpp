#include "wifi_manager.h"
#include <Arduino.h>

WifiManager::WifiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password), _state(WIFI_STATE_DISCONNECTED), _lastReconnectAttempt(0) {}

void WifiManager::begin() {
    Serial.println("[WiFi] Starting Wi-Fi Manager...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);
    _state = WIFI_STATE_CONNECTING;
    Serial.printf("[WiFi] Connecting to %s...\n", _ssid);
}

void WifiManager::update() {
    wl_status_t status = WiFi.status();

    switch (_state) {
        case WIFI_STATE_DISCONNECTED:
            if (millis() - _lastReconnectAttempt > _reconnectInterval) {
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Reconnecting...");
                WiFi.begin(_ssid, _password);
                _state = WIFI_STATE_CONNECTING;
            }
            break;

        case WIFI_STATE_CONNECTING:
            if (status == WL_CONNECTED) {
                _state = WIFI_STATE_CONNECTED;
                Serial.print("[WiFi] Connected! IP address: ");
                Serial.println(WiFi.localIP());
            } else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL) {
                _state = WIFI_STATE_DISCONNECTED;
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Connection failed.");
            }
            break;

        case WIFI_STATE_CONNECTED:
            if (status != WL_CONNECTED) {
                _state = WIFI_STATE_DISCONNECTED;
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Connection lost.");
            }
            break;
    }
}

WifiState WifiManager::getState() const {
    return _state;
}

String WifiManager::getIPAddress() const {
    if (_state == WIFI_STATE_CONNECTED) {
        return WiFi.localIP().toString();
    }
    return "0.0.0.0";
}

int8_t WifiManager::getRSSI() const {
    if (_state == WIFI_STATE_CONNECTED) {
        return WiFi.RSSI();
    }
    return -100;
}
