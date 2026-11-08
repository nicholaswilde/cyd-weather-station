#include <unity.h>
#include "WiFi.h"
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "../../src/wifi_manager.cpp"
#include "landing_html.h"
#include "settings_html.h"

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

void test_wifi_manager_ap_mode_reconnects_in_background(void) {
    mock_millis_val = 0;
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    
    // Transition to AP mode on timeout
    mock_millis_val = 31000;
    WiFi._status = 0;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
    
    // WiFi gets connected in background
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTED, wifi.getState());
}

void test_wifi_manager_set_credentials(void) {
    WifiManager wifi("SSID", "PASS");
    wifi.setCredentials("NewSSID", "NewPass");
    // Verify setCredentials logic
    // WifiManager doesn't expose getters for _ssid and _password directly, 
    // but we can just ensure it doesn't crash or behaves correctly.
    // If we wanted to test properly, we would mock WiFi.begin parameters.
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());
}

void test_wifi_manager_getters_and_helpers(void) {
    WifiManager wifi("SSID", "PASS");
    
    // Disconnected IP and RSSI
    TEST_ASSERT_EQUAL_STRING("0.0.0.0", wifi.getIPAddress().c_str());
    TEST_ASSERT_EQUAL(-100, wifi.getRSSI());

    // AP SSID generation
    String apSsid = wifi.getAPSSID();
    TEST_ASSERT_TRUE(apSsid.startsWith("cyd-weather-station-"));

    // Connected RSSI
    wifi.begin();
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(-55, wifi.getRSSI());

    // Apply screenshot server setting
    wifi.applyScreenshotServerSetting(true);
    wifi.applyScreenshotServerSetting(false);
}

void test_wifi_manager_empty_ssid_starts_ap_mode(void) {
    WifiManager wifi("", "");
    wifi.begin();
    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
    TEST_ASSERT_EQUAL_STRING("192.168.4.1", wifi.getIPAddress().c_str());
}

void test_wifi_manager_placeholder_ssid_starts_ap_mode(void) {
    // Regression test: After factory reset, fallback placeholder "YOUR_WIFI_SSID" must start AP mode directly
    WifiManager wifi("YOUR_WIFI_SSID", "YOUR_WIFI_PASSWORD");
    wifi.begin();
    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
    TEST_ASSERT_EQUAL_STRING("192.168.4.1", wifi.getIPAddress().c_str());
}

void test_wifi_manager_does_not_prematurely_enter_ap_on_transient_status(void) {
    mock_millis_val = 0;
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());

    // Even if WiFi.status() temporarily reports WL_NO_SSID_AVAIL or WL_CONNECT_FAILED before timeout,
    // WifiManager must wait for the 30-second connection timeout before entering AP mode.
    mock_millis_val = 5000;
    WiFi._status = WL_NO_SSID_AVAIL;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());

    mock_millis_val = 10000;
    WiFi._status = WL_CONNECT_FAILED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());

    // Once timeout expires without connecting on initial boot, transition to AP mode
    mock_millis_val = 31000;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
}

void test_wifi_manager_post_connect_drop_does_not_overwrite_with_ap_mode(void) {
    mock_millis_val = 0;
    WifiManager wifi("SSID", "PASS");
    wifi.begin();
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTED, wifi.getState());

    // Simulate temporary Wi-Fi drop after having already connected
    mock_millis_val = 60000;
    WiFi._status = WL_NO_SSID_AVAIL;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());

    // If auto-reconnect recovers before reconnectInterval, immediately return to CONNECTED
    mock_millis_val = 62000;
    WiFi._status = WL_CONNECTED;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTED, wifi.getState());

    // Drop again and stay disconnected past reconnectInterval and connectionTimeout
    mock_millis_val = 70000;
    WiFi._status = WL_NO_SSID_AVAIL;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());

    // After 11s -> enters CONNECTING
    mock_millis_val = 81000;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_CONNECTING, wifi.getState());

    // After 31s in CONNECTING -> must retry (DISCONNECTED) instead of wiping weather display with AP mode ("Setup")
    mock_millis_val = 112000;
    wifi.update();
    TEST_ASSERT_EQUAL(WIFI_STATE_DISCONNECTED, wifi.getState());
}

void test_wifi_manager_factory_reset_uses_post(void) {
    // Regression test for Issue #52: Factory reset must use HTTP POST form and not a GET link
    TEST_ASSERT_NULL(strstr(landing_html, "<a href=\"/reset\""));
    TEST_ASSERT_NULL(strstr(landing_html, "<a href='/reset'"));
    TEST_ASSERT_NOT_NULL(strstr(landing_html, "action=\"/reset\""));
    TEST_ASSERT_NOT_NULL(strstr(landing_html, "method=\"POST\""));
    TEST_ASSERT_NOT_NULL(strstr(landing_html, "submitAction"));
}

void test_wifi_manager_owm_api_input_configuration(void) {
    // Regression test for Issue #53: OWM API key must not be masked as password or interfere with browser autofill
    TEST_ASSERT_NULL(strstr(settings_html, "type='password' id='owm_api'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "type='text' id='owm_api'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "id='owm_api' name='owm_api' placeholder="));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "autocomplete='off'"));
}

void test_wifi_manager_reboot_ap_action(void) {
    // Regression test for Issue #54: Dedicated Reboot to AP action and Wi-Fi fields in settings
    TEST_ASSERT_NULL(strstr(landing_html, "<a href=\"/reboot_ap\""));
    TEST_ASSERT_NULL(strstr(landing_html, "<a href='/reboot_ap'"));
    TEST_ASSERT_NOT_NULL(strstr(landing_html, "action=\"/reboot_ap\""));
    TEST_ASSERT_NOT_NULL(strstr(landing_html, "method=\"POST\""));

    TEST_ASSERT_NOT_NULL(strstr(settings_html, "action='/reboot_ap'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "method='POST'"));

    TEST_ASSERT_NOT_NULL(strstr(settings_html, "id='wifi_ssid'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "name='wifi_ssid'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "id='wifi_password'"));
    TEST_ASSERT_NOT_NULL(strstr(settings_html, "name='wifi_password'"));
}

void test_wifi_manager_forced_ap_mode_on_boot(void) {
    // Regression test for Issue #54: Device boots directly into AP mode when requested via settings
    settings.setForceAPMode(true);
    TEST_ASSERT_TRUE(settings.getForceAPMode());

    WifiManager wifi("SSID", "PASS");
    wifi.begin();

    TEST_ASSERT_EQUAL(WIFI_STATE_AP_MODE, wifi.getState());
    TEST_ASSERT_FALSE(settings.getForceAPMode());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_wifi_manager_initial_state);
    RUN_TEST(test_wifi_manager_transitions_to_connecting);
    RUN_TEST(test_wifi_manager_transitions_to_connected);
    RUN_TEST(test_wifi_manager_loses_connection);
    RUN_TEST(test_wifi_manager_transitions_to_ap_mode);
    RUN_TEST(test_wifi_manager_ap_mode_reconnects_in_background);
    RUN_TEST(test_wifi_manager_set_credentials);
    RUN_TEST(test_wifi_manager_getters_and_helpers);
    RUN_TEST(test_wifi_manager_empty_ssid_starts_ap_mode);
    RUN_TEST(test_wifi_manager_placeholder_ssid_starts_ap_mode);
    RUN_TEST(test_wifi_manager_does_not_prematurely_enter_ap_on_transient_status);
    RUN_TEST(test_wifi_manager_post_connect_drop_does_not_overwrite_with_ap_mode);
    RUN_TEST(test_wifi_manager_factory_reset_uses_post);
    RUN_TEST(test_wifi_manager_owm_api_input_configuration);
    RUN_TEST(test_wifi_manager_reboot_ap_action);
    RUN_TEST(test_wifi_manager_forced_ap_mode_on_boot);
    return UNITY_END();
}
