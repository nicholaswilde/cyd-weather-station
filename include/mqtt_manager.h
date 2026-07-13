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

class MqttManager {
public:
    /**
     * @brief Construct a new Mqtt Manager object
     * @param server MQTT broker IP or hostname
     * @param port MQTT broker port (usually 1883)
     * @param user MQTT username
     * @param password MQTT password
     */
    MqttManager(const char* server, uint16_t port, const char* user, const char* password);

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
    void publish(const char* topic, const char* payload);

    /**
     * @brief Disconnects from the MQTT broker.
     */
    void disconnect();

private:
    void connectToMqtt();
    
    // Member callbacks for AsyncMqttClient events
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    
    // FreeRTOS timer callback must be static
    static void onMqttReconnectTimer(TimerHandle_t xTimer);

    AsyncMqttClient _mqttClient;
    TimerHandle_t _reconnectTimer;

    const char* _server;
    uint16_t _port;
    const char* _user;
    const char* _password;
};

#endif // MQTT_MANAGER_H
