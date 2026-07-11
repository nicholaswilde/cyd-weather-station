#include <unity.h>
#include "WiFi.h"
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "../../src/wifi_manager.cpp"

void setUp(void) {
    WiFi._status = 0;
}

void tearDown(void) {
    // clean stuff up here
}

void test_wifi_manager_initial_state(void) {
    WifiManager wifi("SSID", "PASS");
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());
}

void test_wifi_manager_transitions_to_connecting(void) {
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());
}

void test_wifi_manager_transitions_to_connected(void) {
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTED, wifi.getState());
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", wifi.getIPAddress().c_str());
}

void test_wifi_manager_loses_connection(void) {
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTED, wifi.getState());
    
    // Connection lost
    WiFi._status = 0;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());
}

void test_wifi_manager_transitions_to_ap_mode(void) {
    mock_millis_val = 0;
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());

    // Simulate 31 seconds passing
    mock_millis_val = 31000;
    WiFi._status = 0; // not connected
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_wifi_manager_initial_state);
    RUN_TEST(test_wifi_manager_transitions_to_connecting);
    RUN_TEST(test_wifi_manager_transitions_to_connected);
    RUN_TEST(test_wifi_manager_loses_connection);
    RUN_TEST(test_wifi_manager_transitions_to_ap_mode);
    return UNITY_END();
}
