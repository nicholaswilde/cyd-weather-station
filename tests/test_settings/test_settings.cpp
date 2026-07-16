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
    TEST_ASSERT_EQUAL(UNIT_SYSTEM, settings.getUnitSystem());
    TEST_ASSERT_EQUAL(80, settings.getBrightness());
    TEST_ASSERT_EQUAL(USE_LDR_AUTO_BACKLIGHT, settings.getAutoBrightness());
    TEST_ASSERT_EQUAL(GMT_OFFSET_SEC / 3600, settings.getTimezoneOffset());
    TEST_ASSERT_EQUAL((DST_OFFSET_SEC > 0), settings.getDstEnabled());
    TEST_ASSERT_EQUAL(USE_SD_LOGGING, settings.getSdLoggingEnabled());
    TEST_ASSERT_EQUAL(false, settings.getScreenshotServerEnabled());
    TEST_ASSERT_EQUAL(1, settings.getScreenOrientation());
    TEST_ASSERT_EQUAL(MQTT_ENABLED, settings.getMqttEnabled());
    TEST_ASSERT_EQUAL(USE_SD_CACHE, settings.getSdCacheEnabled());
    TEST_ASSERT_EQUAL(SCREENSAVER_ENABLED, settings.getScreensaverEnabled());
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
    settings.setScreenOrientation(2);
    settings.setMqttEnabled(false);
    settings.setSdCacheEnabled(false);
    settings.setScreensaverEnabled(!SCREENSAVER_ENABLED);
    
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
    TEST_ASSERT_EQUAL(2, settings_new.getScreenOrientation());
    TEST_ASSERT_EQUAL(false, settings_new.getMqttEnabled());
    TEST_ASSERT_EQUAL(false, settings_new.getSdCacheEnabled());
    TEST_ASSERT_EQUAL(!SCREENSAVER_ENABLED, settings_new.getScreensaverEnabled());
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
