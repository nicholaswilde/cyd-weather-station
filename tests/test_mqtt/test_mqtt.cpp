#include <unity.h>
#include "../mocks/mocks.cpp"
#include "mqtt_manager.h"
#include "../../src/mqtt_manager.cpp"

void test_mqtt_initialization(void) {
    MqttManager mqtt("broker.local", 1883, "user", "pass");
    mqtt.begin();
    TEST_ASSERT_FALSE(mqtt.isConnected()); // Initially disconnected
}

void test_mqtt_connect_and_disconnect(void) {
    MqttManager mqtt("broker.local", 1883, "user", "pass");
    mqtt.begin();
    TEST_ASSERT_FALSE(mqtt.isConnected());
    
    mqtt.onNetworkAvailable();
    TEST_ASSERT_TRUE(mqtt.isConnected());
    
    mqtt.disconnect();
    TEST_ASSERT_FALSE(mqtt.isConnected());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_mqtt_initialization);
    RUN_TEST(test_mqtt_connect_and_disconnect);
    return UNITY_END();
}
