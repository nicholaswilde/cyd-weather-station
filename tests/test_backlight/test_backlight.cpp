#include <unity.h>
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "backlight_manager.h"
#include "../../src/backlight_manager.cpp"

void setUp(void) {}
void tearDown(void) {}

void test_backlight_initial_state(void) {
    BacklightManager bm(21, 0, 10.0f);
    TEST_ASSERT_EQUAL(0, bm.getDutyCycle());
}

void test_backlight_mapping_extremes(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    // First reading initializes the filter instantly.
    // 0 ADC should map to minimum duty (10% of 255 = 25.5 -> 26)
    bm.update(0);
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
    
    // Test direct jump to max extreme
    BacklightManager bm2(21, 0, 10.0f);
    bm2.update(4095);
    TEST_ASSERT_EQUAL(255, bm2.getDutyCycle());
}

void test_backlight_smoothing(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    // Initial reading: 0
    bm.update(0);
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
    
    // Subsequent reading: 4095.
    // Since alpha is 0.1, the new filtered light should be:
    // filtered = 0.1 * 4095 + 0.9 * 0 = 409.5
    // 409.5 is 10% of 4095, so lightPercent is 10.
    // Because percentToDuty clamps at minBrightnessPercent (10),
    // duty should be the minimum duty cycle (26).
    bm.update(4095);
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
}

void test_backlight_manual_brightness(void) {
    BacklightManager bm(21, 0, 10.0f);
    
    bm.setManualBrightness(50); // 50%
    // 50% with min 10% -> normalized 40/90 = 0.444. gamma=0.168. duty=26+0.168*229 = 64
    TEST_ASSERT_EQUAL(64, bm.getDutyCycle());
    
    bm.setManualBrightness(100); // 100%
    TEST_ASSERT_EQUAL(255, bm.getDutyCycle());
    
    bm.setManualBrightness(0); // Should be capped at min (10% -> 26)
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
}

void test_backlight_fade_to(void) {
    BacklightManager bm(21, 0, 10.0f);
    bm.setManualBrightness(10); // Start at 10%
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
    
    bm.fadeTo(50, 50); // Fade to 50% (should be 64 duty cycle)
    TEST_ASSERT_EQUAL(64, bm.getDutyCycle());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_backlight_initial_state);
    RUN_TEST(test_backlight_mapping_extremes);
    RUN_TEST(test_backlight_smoothing);
    RUN_TEST(test_backlight_manual_brightness);
    RUN_TEST(test_backlight_fade_to);
    return UNITY_END();
}
