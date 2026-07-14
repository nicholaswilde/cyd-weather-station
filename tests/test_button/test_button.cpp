#include <unity.h>
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "button_manager.h"
#include "../../src/button_manager.cpp"

void setUp(void) {
    mock_millis_val = 0;
    // Default pin 0 (BOOT button) to HIGH (not pressed)
    mock_pin_states[0] = HIGH;
}

void tearDown(void) {}

void test_button_initial_state(void) {
    ButtonManager bm(0);
    bm.begin();
    TEST_ASSERT_EQUAL(INPUT, mock_pin_modes[0]);
}

void test_button_no_press(void) {
    ButtonManager bm(0);
    bm.begin();
    
    // Update at 100ms
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(100));
}

void test_button_single_quick_press(void) {
    ButtonManager bm(0);
    bm.begin();

    // 100ms: Press button (LOW)
    mock_pin_states[0] = LOW;
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(100));

    // 150ms: Still pressed (stable after 50ms debounce)
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(150));

    // 300ms: Release button (HIGH)
    mock_pin_states[0] = HIGH;
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(300)); // Debouncing release

    // 350ms: Released (stable release, duration of press was 300-100 = 200ms)
    // This should register as a SINGLE_PRESS
    TEST_ASSERT_EQUAL(ButtonAction::SINGLE_PRESS, bm.update(350));
}

void test_button_long_press(void) {
    ButtonManager bm(0);
    bm.begin();

    // 100ms: Press button (LOW)
    mock_pin_states[0] = LOW;
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(100));

    // 150ms: Stable press
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(150));

    // 2099ms: Still pressed, duration 1999ms (< 2000ms)
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(2099));

    // 2100ms: Pressed, duration 2000ms. Should trigger LONG_PRESS immediately.
    TEST_ASSERT_EQUAL(ButtonAction::LONG_PRESS, bm.update(2100));

    // 2200ms: Still pressed, should not trigger again
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(2200));

    // 2300ms: Release button
    mock_pin_states[0] = HIGH;
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(2300)); // Debouncing release

    // 2350ms: Released. Since long press already triggered, should not trigger single press.
    TEST_ASSERT_EQUAL(ButtonAction::NONE, bm.update(2350));
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_button_initial_state);
    RUN_TEST(test_button_no_press);
    RUN_TEST(test_button_single_quick_press);
    RUN_TEST(test_button_long_press);
    return UNITY_END();
}
