#include <unity.h>
#include "../mocks/mocks.cpp"

void setUp(void) {
    // any setup
}

void tearDown(void) {
    // any cleanup
}

void test_settings_default_values(void) {
    SettingsManager settings;
    settings.begin();
    
    // Default values expected:
    // - Unit System: UNIT_IMPERIAL (since default config is Imperial)
    // - Brightness: 80
    // - Auto Brightness: false
    // - Timezone Offset: -8 (GMT-8, GMT_OFFSET_SEC = -8 * 3600)
    TEST_ASSERT_EQUAL(2, settings.getUnitSystem()); // 2 is UNIT_IMPERIAL
    TEST_ASSERT_EQUAL(80, settings.getBrightness());
    TEST_ASSERT_EQUAL(false, settings.getAutoBrightness());
    TEST_ASSERT_EQUAL(-8, settings.getTimezoneOffset());
    TEST_ASSERT_EQUAL(true, settings.getDstEnabled());
    TEST_ASSERT_EQUAL(true, settings.getSdLoggingEnabled());
    TEST_ASSERT_EQUAL(true, settings.getScreenshotServerEnabled());
}

void test_settings_save_and_load(void) {
    SettingsManager settings;
    settings.begin();
    
    // Modify settings
    settings.setUnitSystem(1); // 1 is UNIT_METRIC
    settings.setBrightness(50);
    settings.setAutoBrightness(true);
    settings.setTimezoneOffset(5);
    settings.setDstEnabled(false);
    settings.setSdLoggingEnabled(false);
    settings.setScreenshotServerEnabled(false);
    
    // Create new instance to simulate re-reading from preferences
    SettingsManager settings_new;
    settings_new.begin();
    
    TEST_ASSERT_EQUAL(1, settings_new.getUnitSystem());
    TEST_ASSERT_EQUAL(50, settings_new.getBrightness());
    TEST_ASSERT_EQUAL(true, settings_new.getAutoBrightness());
    TEST_ASSERT_EQUAL(5, settings_new.getTimezoneOffset());
    TEST_ASSERT_EQUAL(false, settings_new.getDstEnabled());
    TEST_ASSERT_EQUAL(false, settings_new.getSdLoggingEnabled());
    TEST_ASSERT_EQUAL(false, settings_new.getScreenshotServerEnabled());
}

void test_settings_wifi_credentials(void) {
    SettingsManager settings;
    settings.begin();

    // Verify default values fallback to secrets.h macros
    TEST_ASSERT_EQUAL_STRING(WIFI_SSID, settings.getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING(WIFI_PASSWORD, settings.getWifiPassword().c_str());

    // Modify WiFi credentials
    settings.setWifiSSID("New_SSID");
    settings.setWifiPassword("New_Password");

    // Re-instantiate to simulate reboot
    SettingsManager settings_new;
    settings_new.begin();

    TEST_ASSERT_EQUAL_STRING("New_SSID", settings_new.getWifiSSID().c_str());
    TEST_ASSERT_EQUAL_STRING("New_Password", settings_new.getWifiPassword().c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_settings_default_values);
    RUN_TEST(test_settings_save_and_load);
    RUN_TEST(test_settings_wifi_credentials);
    return UNITY_END();
}
