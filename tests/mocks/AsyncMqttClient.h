#ifndef ASYNC_MQTT_CLIENT_MOCK_H
#define ASYNC_MQTT_CLIENT_MOCK_H

#include <functional>
#include <cstdint>
#include <string>

// Define the reason enum so the mock understands it
enum AsyncMqttClientDisconnectReason {
    TCP_DISCONNECTED = 0
};

struct AsyncMqttClientMessageProperties {
    uint8_t qos;
    bool dup;
    bool retain;
};

class AsyncMqttClient {
public:
    AsyncMqttClient() : _connected(false) {}

    void setServer(const char* host, uint16_t port) {}
    void setCredentials(const char* user, const char* pass) {}
    void setClientId(const char* clientId) { mockClientId = clientId; }
    void setWill(const char* topic, uint8_t qos, bool retain, const char* payload) {
        mockWillTopic = topic;
        mockWillPayload = payload;
    }
    void connect() {
        _connected = true;
        if (_onConnectCallback) {
            _onConnectCallback(true);
        }
    }
    bool connected() { return _connected; }
    uint16_t publish(const char* topic, uint8_t qos, bool retain, const char* payload) {
        mockLastPublishTopic = topic;
        mockLastPublishRetain = retain;
        return 1;
    }
    uint16_t subscribe(const char* topic, uint8_t qos) {
        mockLastSubscribeTopic = topic;
        return 1;
    }
    
    // Updated signatures to match production exactly
    void onConnect(std::function<void(bool)> cb) {
        _onConnectCallback = cb;
    }
    void onDisconnect(std::function<void(AsyncMqttClientDisconnectReason)> cb) {
        _onDisconnectCallback = cb;
    }
    void onPublish(std::function<void(uint16_t)> cb) {}
    void onMessage(std::function<void(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)> cb) {
        _onMessageCallback = cb;
    }

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
    std::function<void(char*, char*, AsyncMqttClientMessageProperties, size_t, size_t, size_t)> _onMessageCallback;

public:
    std::string mockClientId;
    std::string mockWillTopic;
    std::string mockWillPayload;
    std::string mockLastPublishTopic;
    bool mockLastPublishRetain = false;
    std::string mockLastSubscribeTopic;
};

#endif
