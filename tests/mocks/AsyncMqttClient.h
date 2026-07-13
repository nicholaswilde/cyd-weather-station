#ifndef ASYNC_MQTT_CLIENT_MOCK_H
#define ASYNC_MQTT_CLIENT_MOCK_H

#include <functional>
#include <cstdint>

// Define the reason enum so the mock understands it
enum AsyncMqttClientDisconnectReason {
    TCP_DISCONNECTED = 0
};

class AsyncMqttClient {
public:
    AsyncMqttClient() : _connected(false) {}

    void setServer(const char* host, uint16_t port) {}
    void setCredentials(const char* user, const char* pass) {}
    void connect() {
        _connected = true;
        if (_onConnectCallback) {
            _onConnectCallback(true);
        }
    }
    bool connected() { return _connected; }
    uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload) { return 1; }
    
    // Updated signatures to match production exactly
    void onConnect(std::function<void(bool)> cb) {
        _onConnectCallback = cb;
    }
    void onDisconnect(std::function<void(AsyncMqttClientDisconnectReason)> cb) {
        _onDisconnectCallback = cb;
    }
    void onPublish(std::function<void(uint16_t)> cb) {}

    void disconnect() {
        _connected = false;
        if (_onDisconnectCallback) {
            _onDisconnectCallback(TCP_DISCONNECTED);
        }
    }

private:
    bool _connected;
    std::function<void(bool)> _onConnectCallback;
    std::function<void(AsyncMqttClientDisconnectReason)> _onDisconnectCallback;
};

#endif
