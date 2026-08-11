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
    TEST_ASSERT_EQUAL_STRING(TIMEZONE_DEFAULT, settings.getTimezone().c_str());
    TEST_ASSERT_EQUAL(USE_SD_LOGGING, settings.getSdLoggingEnabled());
    TEST_ASSERT_EQUAL(false, settings.getScreenshotServerEnabled());
    TEST_ASSERT_EQUAL(1, settings.getScreenOrientation());
    TEST_ASSERT_EQUAL(MQTT_ENABLED, settings.getMqttEnabled());
    TEST_ASSERT_EQUAL(USE_SD_CACHE, settings.getSdCacheEnabled());
    TEST_ASSERT_EQUAL(SCREENSAVER_ENABLED, settings.getScreensaverEnabled());
    TEST_ASSERT_EQUAL(SCREENSAVER_TIMEOUT_MS, settings.getScreensaverTimeout());
    TEST_ASSERT_EQUAL(WEATHER_UPDATE_INTERVAL_MINS, settings.getWeatherUpdateInterval());
    TEST_ASSERT_EQUAL(API_SERVER_ENABLED, settings.getApiServerEnabled());
    TEST_ASSERT_EQUAL(false, settings.getStaticIpEnabled());
    TEST_ASSERT_EQUAL_STRING("", settings.getStaticIp().c_str());
    TEST_ASSERT_EQUAL_STRING("", settings.getStaticGateway().c_str());
    TEST_ASSERT_EQUAL_STRING("255.255.255.0", settings.getStaticSubnet().c_str());
    TEST_ASSERT_EQUAL_STRING("1.1.1.1", settings.getStaticDns().c_str());
}

void test_settings_save_and_load(void) {
    SettingsManager settings;
    settings.begin();
    
    // Modify settings
    settings.setUnitSystem(1); // 1 is UNIT_METRIC
    settings.setBrightness(50);
    settings.setAutoBrightness(true);
    settings.setTimezone("EST5EDT,M3.2.0,M11.1.0");
    settings.setSdLoggingEnabled(false);
    settings.setScreenshotServerEnabled(false);
    settings.setScreenOrientation(2);
    settings.setMqttEnabled(false);
    settings.setSdCacheEnabled(false);
    settings.setScreensaverEnabled(!SCREENSAVER_ENABLED);
    settings.setScreensaverTimeout(600000);
    settings.setWeatherUpdateInterval(15);
    settings.setApiServerEnabled(!API_SERVER_ENABLED);
    settings.setStaticIpEnabled(true);
    settings.setStaticIp("192.168.1.100");
    settings.setStaticGateway("192.168.1.1");
    settings.setStaticSubnet("255.255.0.0");
    settings.setStaticDns("8.8.8.8");
    
    // Create new instance to simulate re-reading from preferences
    SettingsManager settings_new;
    settings_new.begin();
    
    TEST_ASSERT_EQUAL(1, settings_new.getUnitSystem());
    TEST_ASSERT_EQUAL(50, settings_new.getBrightness());
    TEST_ASSERT_EQUAL(true, settings_new.getAutoBrightness());
    TEST_ASSERT_EQUAL_STRING("EST5EDT,M3.2.0,M11.1.0", settings_new.getTimezone().c_str());
    TEST_ASSERT_EQUAL(false, settings_new.getSdLoggingEnabled());
    TEST_ASSERT_EQUAL(false, settings_new.getScreenshotServerEnabled());
    TEST_ASSERT_EQUAL(2, settings_new.getScreenOrientation());
    TEST_ASSERT_EQUAL(false, settings_new.getMqttEnabled());
    TEST_ASSERT_EQUAL(false, settings_new.getSdCacheEnabled());
    TEST_ASSERT_EQUAL(!SCREENSAVER_ENABLED, settings_new.getScreensaverEnabled());
    TEST_ASSERT_EQUAL(600000, settings_new.getScreensaverTimeout());
    TEST_ASSERT_EQUAL(15, settings_new.getWeatherUpdateInterval());
    TEST_ASSERT_EQUAL(!API_SERVER_ENABLED, settings_new.getApiServerEnabled());
    TEST_ASSERT_EQUAL(true, settings_new.getStaticIpEnabled());
    TEST_ASSERT_EQUAL_STRING("192.168.1.100", settings_new.getStaticIp().c_str());
    TEST_ASSERT_EQUAL_STRING("192.168.1.1", settings_new.getStaticGateway().c_str());
    TEST_ASSERT_EQUAL_STRING("255.255.0.0", settings_new.getStaticSubnet().c_str());
    TEST_ASSERT_EQUAL_STRING("8.8.8.8", settings_new.getStaticDns().c_str());
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
void test_settings_location_data(void) {
    SettingsManager settings;
    settings.begin();

    // Verify default values fallback to config.h/secrets.h macros
    TEST_ASSERT_EQUAL_STRING(WEATHER_ZIP_CODE, settings.getZipCode().c_str());
    TEST_ASSERT_EQUAL_STRING(WEATHER_CITY_CODE, settings.getCityCode().c_str());

    // Modify location data
    settings.setZipCode("90210");
    settings.setCityCode("2643743");
    settings.setLatitude("34.103");
    settings.setLongitude("-118.410");

    // Re-instantiate to simulate reboot
    SettingsManager settings_new;
    settings_new.begin();

    TEST_ASSERT_EQUAL_STRING("90210", settings_new.getZipCode().c_str());
    TEST_ASSERT_EQUAL_STRING("2643743", settings_new.getCityCode().c_str());
    TEST_ASSERT_EQUAL_STRING("34.103", settings_new.getLatitude().c_str());
    TEST_ASSERT_EQUAL_STRING("-118.410", settings_new.getLongitude().c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_settings_default_values);
    RUN_TEST(test_settings_save_and_load);
    RUN_TEST(test_settings_wifi_credentials);
    RUN_TEST(test_settings_location_data);
    return UNITY_END();
}
