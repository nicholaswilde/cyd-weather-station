#include <unity.h>
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "backlight_manager.h"
#include "screensaver_manager.h"
#include "../../src/backlight_manager.cpp"
#include "../../src/screensaver_manager.cpp"

void setUp(void) {}
void tearDown(void) {}

void test_screensaver_initial_state(void) {
    BacklightManager bm(21, 0, 10.0f);
    ScreenSaverManager ss(bm, 300000); // 5 mins

    ss.begin();
    TEST_ASSERT_FALSE(ss.isActive());
    TEST_ASSERT_EQUAL(0, ss.getLastActivityTime());
}

void test_screensaver_trigger_on_timeout(void) {
    BacklightManager bm(21, 0, 10.0f);
    bm.setManualBrightness(80);
    TEST_ASSERT_EQUAL(204, bm.getDutyCycle());

    ScreenSaverManager ss(bm, 300000);
    ss.begin();

    // 100 seconds elapsed: should not trigger
    ss.update(100000);
    TEST_ASSERT_FALSE(ss.isActive());
    TEST_ASSERT_EQUAL(204, bm.getDutyCycle());

    // 301 seconds elapsed: should trigger screensaver
    ss.update(301000);
    TEST_ASSERT_TRUE(ss.isActive());
    // Backlight should dim to 5% (min is 10% -> 26)
    TEST_ASSERT_EQUAL(26, bm.getDutyCycle());
}

void test_screensaver_wake_on_activity(void) {
    BacklightManager bm(21, 0, 10.0f);
    bm.setManualBrightness(80);

    ScreenSaverManager ss(bm, 300000);
    ss.begin();

    // Trigger screensaver
    ss.update(301000);
    TEST_ASSERT_TRUE(ss.isActive());

    // Wake up
    ss.wake(80);
    TEST_ASSERT_FALSE(ss.isActive());
    TEST_ASSERT_EQUAL(204, bm.getDutyCycle());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_screensaver_initial_state);
    RUN_TEST(test_screensaver_trigger_on_timeout);
    RUN_TEST(test_screensaver_wake_on_activity);
    return UNITY_END();
}
