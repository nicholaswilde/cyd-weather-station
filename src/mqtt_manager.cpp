#include "mqtt_manager.h"

MqttManager::MqttManager(const String& server, uint16_t port, const String& user, const String& password)
    : _server(server), _port(port), _user(user), _password(password), _reconnectTimer(nullptr) {}

void MqttManager::updateConfig(const String& server, uint16_t port, const String& user, const String& password) {
    _server = server;
    _port = port;
    _user = user;
    _password = password;
    
    // If we're already connected, we should disconnect and let it reconnect with new settings, 
    // or just apply credentials for the next reconnect.
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
}

void MqttManager::begin() {
    // 1. Create a FreeRTOS timer for non-blocking reconnects.
    _reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void*)this, onMqttReconnectTimer);

    // 2. Configure broker details
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
    _mqttClient.setWill("cyd/status", 1, true, "offline");

    // 2.5 Configure unique Client ID
    static String clientId = "CYD-Weather-" + WiFi.macAddress();
    _mqttClient.setClientId(clientId.c_str());

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
    
    // Publish HA Discovery configuration
    publishHADiscovery();
}

void MqttManager::publishHADiscovery() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String deviceId = "cyd_weather_" + mac;
    String deviceJson = "\"device\":{\"identifiers\":[\"" + deviceId + "\"],\"name\":\"CYD Weather Station\",\"manufacturer\":\"Nicholas Wilde\",\"model\":\"CYD-28R/35C\"}";

    // Temperature
    String tempPayload = "{\"name\":\"Temperature\",\"state_topic\":\"cyd/weather/temperature\",\"unit_of_measurement\":\"°F\",\"device_class\":\"temperature\",\"unique_id\":\"" + deviceId + "_temp\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/temperature/config").c_str(), 0, true, tempPayload.c_str());

    // Humidity
    String humPayload = "{\"name\":\"Humidity\",\"state_topic\":\"cyd/weather/humidity\",\"unit_of_measurement\":\"%\",\"device_class\":\"humidity\",\"unique_id\":\"" + deviceId + "_hum\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/humidity/config").c_str(), 0, true, humPayload.c_str());

    // Wind Speed
    String windPayload = "{\"name\":\"Wind Speed\",\"state_topic\":\"cyd/weather/wind_speed\",\"unit_of_measurement\":\"mph\",\"device_class\":\"wind_speed\",\"unique_id\":\"" + deviceId + "_wind\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/wind_speed/config").c_str(), 0, true, windPayload.c_str());

    // Weather Condition/Status
    String statusPayload = "{\"name\":\"Weather Condition\",\"state_topic\":\"cyd/weather/status\",\"unique_id\":\"" + deviceId + "_cond\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/condition/config").c_str(), 0, true, statusPayload.c_str());

    // Connection Status
    String connPayload = "{\"name\":\"Connection Status\",\"state_topic\":\"cyd/status\",\"payload_on\":\"online\",\"payload_off\":\"offline\",\"device_class\":\"connectivity\",\"unique_id\":\"" + deviceId + "_conn\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/binary_sensor/" + deviceId + "/connection/config").c_str(), 0, true, connPayload.c_str());
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
