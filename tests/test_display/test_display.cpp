#include <unity.h>
#include "../mocks/mocks.cpp"
#include "display.h"
#include "../../src/touch_manager.cpp"
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

void test_coordinate_clamping_and_defaults(void) {
    int16_t out_x, out_y;
    
    // Out of bounds low: negative raw coordinates
    mapTouchCoordinates(-100, -100, out_x, out_y, 1);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
    
    // Out of bounds high: very high raw coordinates
    mapTouchCoordinates(5000, 5000, out_x, out_y, 1);
    TEST_ASSERT_EQUAL(319, out_x);
    TEST_ASSERT_EQUAL(239, out_y);

    // Default / unknown orientation fallback
    mapTouchCoordinates(200, 200, out_x, out_y, 99);
    TEST_ASSERT_EQUAL(0, out_x);
    TEST_ASSERT_EQUAL(0, out_y);
}

void test_touch_manager_mock_and_callback(void) {
    TouchManager::begin();
    TEST_ASSERT_FALSE(TouchManager::isCapacitive());
    
    setMockTouch(false, 0, 0);
    TEST_ASSERT_FALSE(TouchManager::isTouched());
    
    int tx = 0, ty = 0;
    TEST_ASSERT_FALSE(TouchManager::getTouchPoint(tx, ty));
    
    setMockTouch(true, 1500, 2000);
    TEST_ASSERT_TRUE(TouchManager::isTouched());
    TEST_ASSERT_TRUE(TouchManager::getTouchPoint(tx, ty));
    TEST_ASSERT_EQUAL(1500, tx);
    TEST_ASSERT_EQUAL(2000, ty);

    // Test touchpad read callback (touched)
    lv_indev_data_t data;
    my_touchpad_read(nullptr, &data);
    TEST_ASSERT_EQUAL(LV_INDEV_STATE_PR, data.state);
    TEST_ASSERT_GREATER_THAN(0, data.point.x);
    TEST_ASSERT_GREATER_THAN(0, data.point.y);

    // Test touchpad read callback (released)
    setMockTouch(false, 0, 0);
    my_touchpad_read(nullptr, &data);
    TEST_ASSERT_EQUAL(LV_INDEV_STATE_REL, data.state);
}

void test_disp_flush_and_init(void) {
    initDisplayAndTouch();
    initLVGL();

    lv_disp_drv_t drv;
    lv_area_t area = {0, 0, 9, 9}; // 10x10 area
    lv_color_t color_buf[100];
    for (int i = 0; i < 100; ++i) {
        color_buf[i].full = 0xFFFF;
    }

    // Flush without screenshot capture
    my_disp_flush(&drv, &area, color_buf);

    // Flush with screenshot capture active
    ScreenshotManager::beginCapture("/test_flush.bmp");
    my_disp_flush(&drv, &area, color_buf);
    ScreenshotManager::endCapture();
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_coordinate_mapping_landscape);
    RUN_TEST(test_coordinate_mapping_landscape_rev);
    RUN_TEST(test_coordinate_mapping_portrait);
    RUN_TEST(test_coordinate_mapping_portrait_rev);
    RUN_TEST(test_coordinate_clamping_and_defaults);
    RUN_TEST(test_touch_manager_mock_and_callback);
    RUN_TEST(test_disp_flush_and_init);
    return UNITY_END();
}
