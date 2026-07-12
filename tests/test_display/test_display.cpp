#include <unity.h>
#include "../mocks/mocks.cpp"
#include "display.h"
#include "../../src/display.cpp"
#include "../../src/screenshot_manager.cpp"

void setUp(void) {}
void tearDown(void) {}

void test_coordinate_mapping_landscape(void) {
    int16_t out_x, out_y;
    
    // Rotation 1 (Landscape, 320x240)
    mapTouchCoordinates(200, 200, out_x, out_y, 1);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
    
    mapTouchCoordinates(3700, 3900, out_x, out_y, 1);
    TEST_ASSERT_EQUAL(319, out_x);
    TEST_ASSERT_EQUAL(239, out_y);
}

void test_coordinate_mapping_landscape_rev(void) {
    int16_t out_x, out_y;
    
    // Rotation 3 (Landscape Rev, 320x240)
    mapTouchCoordinates(200, 200, out_x, out_y, 3);
    TEST_ASSERT_EQUAL(319, out_x);
    TEST_ASSERT_EQUAL(239, out_y);
    
    mapTouchCoordinates(3700, 3900, out_x, out_y, 3);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
}

void test_coordinate_mapping_portrait(void) {
    int16_t out_x, out_y;
    
    // Rotation 0 (Portrait Rev, 240x320)
    mapTouchCoordinates(200, 200, out_x, out_y, 0);
    TEST_ASSERT_EQUAL(239, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
    
    mapTouchCoordinates(3700, 3900, out_x, out_y, 0);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(319, out_y);
}

void test_coordinate_mapping_portrait_rev(void) {
    int16_t out_x, out_y;
    
    // Rotation 2 (Portrait, 240x320)
    mapTouchCoordinates(200, 200, out_x, out_y, 2);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(319, out_y);
    
    mapTouchCoordinates(3700, 3900, out_x, out_y, 2);
    TEST_ASSERT_EQUAL(239, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_coordinate_mapping_landscape);
    RUN_TEST(test_coordinate_mapping_landscape_rev);
    RUN_TEST(test_coordinate_mapping_portrait);
    RUN_TEST(test_coordinate_mapping_portrait_rev);
    return UNITY_END();
}
