#ifndef MQTT_MANAGER_H
#define MQTT_MANAGER_H

#ifndef NATIVE_TEST
#include <AsyncMqttClient.h>
#else
#include "../tests/mocks/AsyncMqttClient.h"
#include "../tests/mocks/Arduino.h"
#endif
#include <Arduino.h>
#include <WiFi.h>
#include <functional>

typedef std::function<void(const String& topic, const String& payload)> MqttMessageCallback;

class MqttManager {
public:
    /**
     * @brief Construct a new Mqtt Manager object
     * @param server MQTT broker IP or hostname
     * @param port MQTT broker port (usually 1883)
     * @param user MQTT username
     * @param password MQTT password
     */
    MqttManager(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic);

    void updateConfig(const String& server, uint16_t port, const String& user, const String& password, const String& baseTopic);

    /**
     * @brief Initializes the FreeRTOS timers and MQTT client callbacks.
     */
    void begin();

    /**
     * @brief Called by Wi-Fi events when an IP is obtained.
     */
    void onNetworkAvailable();

    /**
     * @brief Called by Wi-Fi events when the network disconnects.
     */
    void onNetworkDisconnected();

    /**
     * @brief Checks if the client is currently connected to the broker.
     */
    bool isConnected();

    /**
     * @brief Publishes a message to a specific topic (QoS 0).
     */
    void publish(const char* topic, const char* payload, bool retain = false);

    /**
     * @brief Subscribes to an MQTT topic.
     */
    void subscribe(const char* topic, uint8_t qos = 0);

    /**
     * @brief Registers a callback for incoming MQTT messages.
     */
    void onMessage(MqttMessageCallback cb);

    /**
     * @brief Disconnects from the MQTT broker.
     */
    void disconnect();

#ifdef NATIVE_TEST
public:
#else
private:
#endif
    void connectToMqtt();
    void publishHADiscovery();
    
    // Member callbacks for AsyncMqttClient events
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    void onMqttMessage(char* topic, char* payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    
    // FreeRTOS timer callback must be static
    static void onMqttReconnectTimer(TimerHandle_t xTimer);

    AsyncMqttClient _mqttClient;
    TimerHandle_t _reconnectTimer;
    uint32_t _reconnectBackoffMs;

    String _server;
    uint16_t _port;
    String _user;
    String _password;
    String _baseTopic;
    String _clientId;
    String _willTopic;
    
    MqttMessageCallback _messageCallback;
};

#endif // MQTT_MANAGER_H
