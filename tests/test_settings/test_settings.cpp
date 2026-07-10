#include <unity.h>
#include "Arduino.h"
#include "Preferences.h"
#include "../mocks/mocks.cpp"
#include "settings_manager.h"
#include "../../src/settings_manager.cpp"

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
}

void test_settings_save_and_load(void) {
    SettingsManager settings;
    settings.begin();
    
    // Modify settings
    settings.setUnitSystem(1); // 1 is UNIT_METRIC
    settings.setBrightness(50);
    settings.setAutoBrightness(true);
    settings.setTimezoneOffset(5);
    
    // Create new instance to simulate re-reading from preferences
    SettingsManager settings_new;
    settings_new.begin();
    
    TEST_ASSERT_EQUAL(1, settings_new.getUnitSystem());
    TEST_ASSERT_EQUAL(50, settings_new.getBrightness());
    TEST_ASSERT_EQUAL(true, settings_new.getAutoBrightness());
    TEST_ASSERT_EQUAL(5, settings_new.getTimezoneOffset());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_settings_default_values);
    RUN_TEST(test_settings_save_and_load);
    return UNITY_END();
}
