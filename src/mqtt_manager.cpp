#include "mqtt_manager.h"

MqttManager::MqttManager(const char* server, uint16_t port, const char* user, const char* password)
    : _server(server), _port(port), _user(user), _password(password), _reconnectTimer(nullptr) {}

void MqttManager::begin() {
    // 1. Create a FreeRTOS timer for non-blocking reconnects.
    _reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void*)this, onMqttReconnectTimer);

    // 2. Configure broker details
    _mqttClient.setServer(_server, _port);
    _mqttClient.setCredentials(_user, _password);

    // 3. Register the asynchronous callbacks using C++ lambdas
    _mqttClient.onConnect([this](bool sessionPresent) {
        this->onMqttConnect(sessionPresent);
    });
    
    _mqttClient.onDisconnect([this](AsyncMqttClientDisconnectReason reason) {
        this->onMqttDisconnect(reason);
    });

    _mqttClient.onPublish([](uint16_t packetId) {
        Serial.printf("[MQTT] Broker acknowledged publish (Packet ID: %d)\n", packetId);
    });
}

void MqttManager::connectToMqtt() {
    Serial.printf("[MQTT] Connecting to broker at %s:%d...\n", _server, _port);
    _mqttClient.connect();
}

void MqttManager::onNetworkAvailable() {
    Serial.println("[MQTT] Network is up. Initiating broker connection...");
    connectToMqtt();
}

void MqttManager::onNetworkDisconnected() {
    Serial.println("[MQTT] Network is down. Halting reconnect timers...");
    if (_reconnectTimer) {
        xTimerStop(_reconnectTimer, 0);
    }
}

void MqttManager::onMqttConnect(bool sessionPresent) {
    Serial.println("[MQTT] Connected to broker!");
    
    // Publish a boot message
    _mqttClient.publish("cyd/status", 0, true, "online");
}

void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("[MQTT] Disconnected from broker! Reason code: %d\n", (int8_t)reason);
    
    if ((int8_t)reason == 4) {
        Serial.println("[MQTT] Hint: Reason 4 usually means Bad Username or Password.");
    }

    Serial.println("[MQTT] Reconnecting in 5 seconds...");
    
    // Only start the reconnect timer if Wi-Fi is still connected
    if (WiFi.status() == WL_CONNECTED && _reconnectTimer) {
        xTimerStart(_reconnectTimer, 0);
    }
}

bool MqttManager::isConnected() {
    return _mqttClient.connected();
}

void MqttManager::publish(const char* topic, const char* payload) {
    if (isConnected()) {
        Serial.printf("[MQTT] Publishing -> Topic: '%s' | Payload: '%s'\n", topic, payload);
        uint16_t packetId = _mqttClient.publish(topic, 0, false, payload);
        
        if (packetId == 0) {
            Serial.println("[MQTT] ERROR: Publish failed (buffer might be full)");
        }
    } else {
        Serial.printf("[MQTT] WARN: Cannot publish to '%s' - Not connected to broker.\n", topic);
    }
}

void MqttManager::disconnect() {
    Serial.println("[MQTT] Disconnecting from broker...");
    if (_reconnectTimer) {
        xTimerStop(_reconnectTimer, 0);
    }
    _mqttClient.disconnect();
}


void MqttManager::onMqttReconnectTimer(TimerHandle_t xTimer) {
    // Retrieve the class instance pointer from the timer ID
    MqttManager* instance = static_cast<MqttManager*>(pvTimerGetTimerID(xTimer));
    if (instance) {
        instance->connectToMqtt();
    }
}
