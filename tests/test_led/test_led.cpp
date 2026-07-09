#include <unity.h>
#include "Arduino.h"
#include "../mocks/mocks.cpp"
#include "led_manager.h"
#include "../../src/led_manager.cpp"

void setUp(void) {
    mock_millis_val = 0;
    for (int i = 0; i < 100; i++) {
        mock_pin_modes[i] = 0;
        mock_pin_states[i] = HIGH; // active-low default off
    }
}

void tearDown(void) {
    // cleanup
}

void test_led_manager_initialization(void) {
    LedManager led(4, 16, 17);
    led.begin();
    
    // Check pin modes are set to OUTPUT
    TEST_ASSERT_EQUAL(OUTPUT, mock_pin_modes[4]);
    TEST_ASSERT_EQUAL(OUTPUT, mock_pin_modes[16]);
    TEST_ASSERT_EQUAL(OUTPUT, mock_pin_modes[17]);
    
    // Check initial state is off (active-low, so pins should be HIGH)
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]);
    
    TEST_ASSERT_EQUAL(LedManager::STATE_OFF, led.getState());
}

void test_led_manager_wifi_connecting_blink(void) {
    LedManager led(4, 16, 17);
    led.begin();
    led.setState(LedManager::STATE_CONNECTING);
    
    // At millis = 0, slow blink is ON (Blue active, active-low -> LOW)
    led.update(0);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);  // Red off
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]); // Green off
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[17]);  // Blue on
    
    // At millis = 499, still ON
    mock_millis_val = 499;
    led.update(499);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[17]);
    
    // At millis = 500, slow blink toggles OFF
    mock_millis_val = 500;
    led.update(500);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]);
    
    // At millis = 1000, slow blink toggles ON again
    mock_millis_val = 1000;
    led.update(1000);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[17]);
}

void test_led_manager_wifi_connected_solid_and_timeout(void) {
    LedManager led(4, 16, 17);
    led.begin();
    led.setState(LedManager::STATE_CONNECTED);
    
    // At millis = 1000, connected -> Solid Green (active-low -> LOW)
    mock_millis_val = 1000;
    led.update(1000);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);  // Red off
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[16]);  // Green on
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]); // Blue off
    
    // At millis = 3999 (under 3 seconds elapsed), still ON
    mock_millis_val = 3999;
    led.update(3999);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[16]);
    
    // At millis = 4000 (3 seconds elapsed), turns OFF, state transitions to STATE_OFF
    mock_millis_val = 4000;
    led.update(4000);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]);
    TEST_ASSERT_EQUAL(LedManager::STATE_OFF, led.getState());
}

void test_led_manager_wifi_disconnected_blink(void) {
    LedManager led(4, 16, 17);
    led.begin();
    led.setState(LedManager::STATE_DISCONNECTED);
    
    // Fast blink Red (200ms on, 200ms off)
    mock_millis_val = 0;
    led.update(0);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[4]);   // Red on
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]); // Green off
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]); // Blue off
    
    mock_millis_val = 199;
    led.update(199);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[4]);
    
    mock_millis_val = 200;
    led.update(200);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);  // Red off
    
    mock_millis_val = 400;
    led.update(400);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[4]);   // Red on
}

void test_led_manager_weather_pulse_yellow(void) {
    LedManager led(4, 16, 17);
    led.begin();
    led.setState(LedManager::STATE_PULSE_YELLOW);
    
    // Yellow pulse: Red=LOW, Green=LOW, Blue=HIGH
    mock_millis_val = 1000;
    led.update(1000);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[4]);
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[16]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]);
    
    // After 1 second (1000ms), pulse completes and turns off
    mock_millis_val = 2000;
    led.update(2000);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]);
    TEST_ASSERT_EQUAL(LedManager::STATE_OFF, led.getState());
}

void test_led_manager_disabled_mode(void) {
    LedManager led(4, 16, 17);
    led.begin();
    led.setState(LedManager::STATE_CONNECTING);
    led.update(0);
    
    // Check that LED is active
    TEST_ASSERT_EQUAL(LOW, mock_pin_states[17]);
    
    // Disable LED manager
    led.setEnabled(false);
    TEST_ASSERT_FALSE(led.isEnabled());
    
    // All LEDs must be forced off (pins HIGH) immediately
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[4]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[16]);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]);
    
    // State remains, but updates do nothing
    mock_millis_val = 1000;
    led.update(1000);
    TEST_ASSERT_EQUAL(HIGH, mock_pin_states[17]);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_led_manager_initialization);
    RUN_TEST(test_led_manager_wifi_connecting_blink);
    RUN_TEST(test_led_manager_wifi_connected_solid_and_timeout);
    RUN_TEST(test_led_manager_wifi_disconnected_blink);
    RUN_TEST(test_led_manager_weather_pulse_yellow);
    RUN_TEST(test_led_manager_disabled_mode);
    return UNITY_END();
}
