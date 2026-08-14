#include "mqtt_manager.h"

MqttManager::MqttManager(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic)
    : _server(server), _port(port), _user(user), _password(password), _baseTopic(baseTopic), _reconnectTimer(nullptr), _reconnectBackoffMs(5000), _messageCallback(nullptr) {}

void MqttManager::updateConfig(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic) {
    _server = server;
    _port = port;
    _user = user;
    _password = password;
    _baseTopic = baseTopic;
    _willTopic = _baseTopic + "status";
    
    // If we're already connected, we should disconnect and let it reconnect with new settings, 
    // or just apply credentials for the next reconnect.
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
    _mqttClient.setWill(_willTopic.c_str(), 1, true, "offline");
}

void MqttManager::begin() {
    // 1. Create a FreeRTOS timer for non-blocking reconnects.
    _reconnectTimer = xTimerCreate("mqttTimer", pdMS_TO_TICKS(5000), pdFALSE, (void*)this, onMqttReconnectTimer);

    // 2. Configure broker details
    _willTopic = _baseTopic + "status";
    _mqttClient.setServer(_server.c_str(), _port);
    _mqttClient.setCredentials(_user.c_str(), _password.c_str());
    _mqttClient.setWill(_willTopic.c_str(), 1, true, "offline");

    // 2.5 Configure unique Client ID
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    _clientId = "CYD-Weather-" + mac;
    _mqttClient.setClientId(_clientId.c_str());

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

    _mqttClient.onMessage([this](char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
        this->onMqttMessage(topic, payload, properties, len, index, total);
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
    
    // Reset backoff on successful connection
    _reconnectBackoffMs = 5000;
    
    // Publish a boot message
    _mqttClient.publish((_baseTopic + "status").c_str(), 0, true, "online");
    
    // Publish HA Discovery configuration
    publishHADiscovery();

    // Subscribe to commands
    subscribe("command/brightness", 0);
    subscribe("command/reboot", 0);
    subscribe("command/auto_brightness", 0);
    subscribe("command/screensaver", 0);
    subscribe("command/theme", 0);
    subscribe("command/units", 0);
    subscribe("command/screen_orientation", 0);
    subscribe("command/update_interval", 0);
}

void MqttManager::publishHADiscovery() {
    String mac = WiFi.macAddress();
    mac.replace(":", "");
    String deviceId = "cyd_weather_" + mac;
    String deviceJson = "\"device\":{\"identifiers\":[\"" + deviceId + "\"],\"name\":\"CYD Weather Station\",\"manufacturer\":\"Nicholas Wilde\",\"model\":\"CYD-28R/35C\"}";

    // Temperature
    String tempPayload = "{\"name\":\"Temperature\",\"state_topic\":\"" + _baseTopic + "weather/temperature\",\"unit_of_measurement\":\"°F\",\"device_class\":\"temperature\",\"unique_id\":\"" + deviceId + "_temp\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/temperature/config").c_str(), 0, true, tempPayload.c_str());

    // Humidity
    String humPayload = "{\"name\":\"Humidity\",\"state_topic\":\"" + _baseTopic + "weather/humidity\",\"unit_of_measurement\":\"%\",\"device_class\":\"humidity\",\"unique_id\":\"" + deviceId + "_hum\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/humidity/config").c_str(), 0, true, humPayload.c_str());

    // Wind Speed
    String windPayload = "{\"name\":\"Wind Speed\",\"state_topic\":\"" + _baseTopic + "weather/wind_speed\",\"unit_of_measurement\":\"mph\",\"device_class\":\"wind_speed\",\"unique_id\":\"" + deviceId + "_wind\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/wind_speed/config").c_str(), 0, true, windPayload.c_str());

    // Weather Condition/Status
    String statusPayload = "{\"name\":\"Weather Condition\",\"state_topic\":\"" + _baseTopic + "weather/status\",\"unique_id\":\"" + deviceId + "_cond\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/condition/config").c_str(), 0, true, statusPayload.c_str());

    // Connection Status
    String connPayload = "{\"name\":\"Connection Status\",\"state_topic\":\"" + _baseTopic + "status\",\"payload_on\":\"online\",\"payload_off\":\"offline\",\"device_class\":\"connectivity\",\"unique_id\":\"" + deviceId + "_conn\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/binary_sensor/" + deviceId + "/connection/config").c_str(), 0, true, connPayload.c_str());

    // Brightness Control (Number)
    String brightPayload = "{\"name\":\"Brightness\",\"command_topic\":\"" + _baseTopic + "command/brightness\",\"min\":1,\"max\":255,\"unique_id\":\"" + deviceId + "_bright\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/number/" + deviceId + "/brightness/config").c_str(), 0, true, brightPayload.c_str());

    // Reboot Control (Button)
    String rebootPayload = "{\"name\":\"Reboot\",\"command_topic\":\"" + _baseTopic + "command/reboot\",\"payload_press\":\"REBOOT\",\"device_class\":\"restart\",\"unique_id\":\"" + deviceId + "_reboot\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/button/" + deviceId + "/reboot/config").c_str(), 0, true, rebootPayload.c_str());

    // --- System Diagnostics ---
    // Uptime
    String uptimePayload = "{\"name\":\"Uptime\",\"state_topic\":\"" + _baseTopic + "system/uptime\",\"unit_of_measurement\":\"s\",\"device_class\":\"duration\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_uptime\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/uptime/config").c_str(), 0, true, uptimePayload.c_str());
    // Free Heap
    String heapPayload = "{\"name\":\"Free Memory\",\"state_topic\":\"" + _baseTopic + "system/free_heap\",\"unit_of_measurement\":\"B\",\"device_class\":\"data_size\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_heap\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/free_heap/config").c_str(), 0, true, heapPayload.c_str());
    // Wi-Fi RSSI
    String rssiPayload = "{\"name\":\"Wi-Fi Signal\",\"state_topic\":\"" + _baseTopic + "system/wifi_rssi\",\"unit_of_measurement\":\"dBm\",\"device_class\":\"signal_strength\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_rssi\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/wifi_rssi/config").c_str(), 0, true, rssiPayload.c_str());
    // IP Address
    String ipPayload = "{\"name\":\"IP Address\",\"state_topic\":\"" + _baseTopic + "system/ip\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_ip\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/ip/config").c_str(), 0, true, ipPayload.c_str());
    // Firmware Version
    String verPayload = "{\"name\":\"Firmware Version\",\"state_topic\":\"" + _baseTopic + "system/version\",\"entity_category\":\"diagnostic\",\"unique_id\":\"" + deviceId + "_version\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/sensor/" + deviceId + "/version/config").c_str(), 0, true, verPayload.c_str());

    // --- Operational Settings ---
    // Auto Brightness (Switch)
    String autoBrPayload = "{\"name\":\"Auto Brightness\",\"state_topic\":\"" + _baseTopic + "settings/auto_brightness\",\"command_topic\":\"" + _baseTopic + "command/auto_brightness\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_autobright\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/auto_brightness/config").c_str(), 0, true, autoBrPayload.c_str());
    // Screensaver (Switch)
    String ssPayload = "{\"name\":\"Screensaver\",\"state_topic\":\"" + _baseTopic + "settings/screensaver\",\"command_topic\":\"" + _baseTopic + "command/screensaver\",\"payload_on\":\"ON\",\"payload_off\":\"OFF\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_screensaver\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/switch/" + deviceId + "/screensaver/config").c_str(), 0, true, ssPayload.c_str());
    // Theme (Select)
    String themePayload = "{\"name\":\"Theme Flavor\",\"state_topic\":\"" + _baseTopic + "settings/theme\",\"command_topic\":\"" + _baseTopic + "command/theme\",\"options\":[\"Mocha\",\"Macchiato\",\"Frappe\",\"Latte\"],\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_theme\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/select/" + deviceId + "/theme/config").c_str(), 0, true, themePayload.c_str());
    // Unit System (Select)
    String unitsPayload = "{\"name\":\"Unit System\",\"state_topic\":\"" + _baseTopic + "settings/units\",\"command_topic\":\"" + _baseTopic + "command/units\",\"options\":[\"Imperial\",\"Metric\"],\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_units\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/select/" + deviceId + "/units/config").c_str(), 0, true, unitsPayload.c_str());
    // Screen Orientation (Select)
    String orientPayload = "{\"name\":\"Screen Orientation\",\"state_topic\":\"" + _baseTopic + "settings/screen_orientation\",\"command_topic\":\"" + _baseTopic + "command/screen_orientation\",\"options\":[\"Landscape\",\"Portrait\",\"Portrait Rev\",\"Landscape Rev\"],\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_orientation\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/select/" + deviceId + "/orientation/config").c_str(), 0, true, orientPayload.c_str());
    // Update Interval (Number)
    String updPayload = "{\"name\":\"Update Interval\",\"state_topic\":\"" + _baseTopic + "settings/update_interval\",\"command_topic\":\"" + _baseTopic + "command/update_interval\",\"min\":1,\"max\":120,\"unit_of_measurement\":\"min\",\"entity_category\":\"config\",\"unique_id\":\"" + deviceId + "_updint\"," + deviceJson + "}";
    _mqttClient.publish(("homeassistant/number/" + deviceId + "/update_interval/config").c_str(), 0, true, updPayload.c_str());
}

void MqttManager::onMqttDisconnect(AsyncMqttClientDisconnectReason reason) {
    Serial.printf("[MQTT] Disconnected from broker! Reason code: %d\n", (int8_t)reason);
    
    if ((int8_t)reason == 4) {
        Serial.println("[MQTT] Hint: Reason 4 usually means Bad Username or Password.");
    }

    Serial.printf("[MQTT] Reconnecting in %lu seconds...\n", _reconnectBackoffMs / 1000);
    
    // Only start the reconnect timer if Wi-Fi is still connected
    if (WiFi.status() == WL_CONNECTED && _reconnectTimer) {
        xTimerChangePeriod(_reconnectTimer, pdMS_TO_TICKS(_reconnectBackoffMs), 0);
        xTimerStart(_reconnectTimer, 0);
        
        // Increase backoff for next time, capped at max limit (e.g., 2 minutes / 120000ms)
        _reconnectBackoffMs *= 2;
        if (_reconnectBackoffMs > 120000) {
            _reconnectBackoffMs = 120000;
        }
    }
}

void MqttManager::onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total) {
    if (_messageCallback) {
        String payloadStr;
        for (size_t i = 0; i < len; i++) {
            payloadStr += (char)payload[i];
        }
        _messageCallback(String(topic), payloadStr);
    }
}

bool MqttManager::isConnected() {
    return _mqttClient.connected();
}

void MqttManager::publish(const char* topic, const char* payload, bool retain) {
    if (isConnected()) {
        String fullTopic = String(topic);
        if (!fullTopic.startsWith("homeassistant/")) {
            fullTopic = _baseTopic + fullTopic;
        }
        Serial.printf("[MQTT] Publishing -> Topic: '%s' | Retain: %d | Payload: '%s'\n", fullTopic.c_str(), retain, payload);
        uint16_t packetId = _mqttClient.publish(fullTopic.c_str(), 0, retain, payload);
        
        if (packetId == 0) {
            Serial.println("[MQTT] ERROR: Publish failed (buffer might be full)");
        }
    } else {
        Serial.printf("[MQTT] WARN: Cannot publish to '%s' - Not connected to broker.\n", topic);
    }
}

void MqttManager::subscribe(const char* topic, uint8_t qos) {
    if (isConnected()) {
        String fullTopic = String(topic);
        if (!fullTopic.startsWith("homeassistant/")) {
            fullTopic = _baseTopic + fullTopic;
        }
        _mqttClient.subscribe(fullTopic.c_str(), qos);
        Serial.printf("[MQTT] Subscribed to topic: %s\n", fullTopic.c_str());
    } else {
        Serial.printf("[MQTT] WARN: Cannot subscribe to '%s' - Not connected to broker.\n", topic);
    }
}

void MqttManager::onMessage(MqttMessageCallback cb) {
    _messageCallback = cb;
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
