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
#ifdef AP_PASSWORD
    TEST_ASSERT_EQUAL_STRING(AP_PASSWORD, settings.getApPassword().c_str());
#else
    TEST_ASSERT_EQUAL_STRING("", settings.getApPassword().c_str());
#endif
    TEST_ASSERT_EQUAL_FLOAT(LOCAL_SENSOR_TEMP_OFFSET, settings.getLocalSensorTempOffset());
    TEST_ASSERT_EQUAL_FLOAT(LOCAL_SENSOR_HUM_OFFSET, settings.getLocalSensorHumOffset());
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
    settings.setApPassword("new_ap_pass");
    settings.setLocalSensorTempOffset(-1.5f);
    settings.setLocalSensorHumOffset(3.0f);
    
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
    TEST_ASSERT_EQUAL_STRING("new_ap_pass", settings_new.getApPassword().c_str());
    TEST_ASSERT_EQUAL_FLOAT(-1.5f, settings_new.getLocalSensorTempOffset());
    TEST_ASSERT_EQUAL_FLOAT(3.0f, settings_new.getLocalSensorHumOffset());
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

void test_settings_clamping(void) {
    SettingsManager settings;
    settings.begin();

    // Brightness clamping (10 - 100)
    settings.setBrightness(0);
    TEST_ASSERT_EQUAL(10, settings.getBrightness());
    settings.setBrightness(150);
    TEST_ASSERT_EQUAL(100, settings.getBrightness());

    // Theme flavor clamping (1 - 4)
    settings.setThemeFlavor(0);
    TEST_ASSERT_EQUAL(1, settings.getThemeFlavor());
    settings.setThemeFlavor(10);
    TEST_ASSERT_EQUAL(4, settings.getThemeFlavor());

    // Screen orientation clamping (0 - 3)
    settings.setScreenOrientation(-5);
    TEST_ASSERT_EQUAL(0, settings.getScreenOrientation());
    settings.setScreenOrientation(10);
    TEST_ASSERT_EQUAL(3, settings.getScreenOrientation());

    // LED brightness clamping (0 - 255)
    settings.setLedBrightness(-10);
    TEST_ASSERT_EQUAL(0, settings.getLedBrightness());
    settings.setLedBrightness(300);
    TEST_ASSERT_EQUAL(255, settings.getLedBrightness());

    // Weather update interval clamping (min 1)
    settings.setWeatherUpdateInterval(0);
    TEST_ASSERT_EQUAL(1, settings.getWeatherUpdateInterval());

    // Local sensor update interval clamping (min 1)
    settings.setLocalSensorUpdateInterval(0);
    TEST_ASSERT_EQUAL(1, settings.getLocalSensorUpdateInterval());
}

void test_settings_all_fields_and_factory_reset(void) {
    SettingsManager settings;
    settings.begin();

    // Test Theme Flavor
    settings.setThemeFlavor(3);
    TEST_ASSERT_EQUAL(3, settings.getThemeFlavor());

    // Test LED Settings
    settings.setLedEnabled(true);
    TEST_ASSERT_TRUE(settings.getLedEnabled());
    settings.setLedBrightness(120);
    TEST_ASSERT_EQUAL(120, settings.getLedBrightness());

    // Test Local Sensor Settings
    settings.setLocalSensorEnabled(true);
    TEST_ASSERT_TRUE(settings.getLocalSensorEnabled());
    settings.setLocalSensorType(3);
    TEST_ASSERT_EQUAL(3, settings.getLocalSensorType());
    settings.setLocalSensorUpdateInterval(30);
    TEST_ASSERT_EQUAL(30, settings.getLocalSensorUpdateInterval());

    // Test MQTT Settings
    settings.setMqttServer("192.168.1.50");
    TEST_ASSERT_EQUAL_STRING("192.168.1.50", settings.getMqttServer().c_str());
    settings.setMqttPort(1884);
    TEST_ASSERT_EQUAL(1884, settings.getMqttPort());
    settings.setMqttUser("mqtt_user");
    TEST_ASSERT_EQUAL_STRING("mqtt_user", settings.getMqttUser().c_str());
    settings.setMqttPassword("mqtt_pass");
    TEST_ASSERT_EQUAL_STRING("mqtt_pass", settings.getMqttPassword().c_str());
    settings.setMqttBaseTopic("home/weather/");
    TEST_ASSERT_EQUAL_STRING("home/weather/", settings.getMqttBaseTopic().c_str());

    // Test OWM API Key & NTP Server
    settings.setOwmApiKey("my_api_key_12345");
    TEST_ASSERT_EQUAL_STRING("my_api_key_12345", settings.getOwmApiKey().c_str());
    settings.setNtpServer("time.google.com");
    TEST_ASSERT_EQUAL_STRING("time.google.com", settings.getNtpServer().c_str());

    // Test Sleep Schedule Settings
    settings.setSleepScheduleEnabled(true);
    TEST_ASSERT_TRUE(settings.getSleepScheduleEnabled());
    settings.setSleepStartTime("23:30");
    TEST_ASSERT_EQUAL_STRING("23:30", settings.getSleepStartTime().c_str());
    settings.setSleepEndTime("06:30");
    TEST_ASSERT_EQUAL_STRING("06:30", settings.getSleepEndTime().c_str());

    // Test Factory Reset
    settings.factoryReset();
    SettingsManager freshSettings;
    freshSettings.begin();
    TEST_ASSERT_EQUAL_STRING("cyd/", freshSettings.getMqttBaseTopic().c_str());
}

void test_settings_24h_format(void) {
    SettingsManager settings;
    settings.begin();

    // Default depends on config.h, but we can just test setter/getter
    settings.setUse24HourFormat(true);
    TEST_ASSERT_TRUE(settings.getUse24HourFormat());

    settings.setUse24HourFormat(false);
    TEST_ASSERT_FALSE(settings.getUse24HourFormat());

    // Test persistence
    settings.setUse24HourFormat(true);
    SettingsManager settings_new;
    settings_new.begin();
    TEST_ASSERT_TRUE(settings_new.getUse24HourFormat());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_settings_24h_format);
    RUN_TEST(test_settings_default_values);
    RUN_TEST(test_settings_save_and_load);
    RUN_TEST(test_settings_wifi_credentials);
    RUN_TEST(test_settings_location_data);
    RUN_TEST(test_settings_clamping);
    RUN_TEST(test_settings_all_fields_and_factory_reset);
    return UNITY_END();
}
