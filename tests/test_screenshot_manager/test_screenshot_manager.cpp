#include <unity.h>
#include "../mocks/mocks.cpp"
#include "screenshot_manager.h"
#include "../../src/screenshot_manager.cpp"

void setUp(void) {}
void tearDown(void) {}

void test_bmp_header_generation(void) {
    BmpHeader header = ScreenshotManager::createHeader(320, 240);
    
    // Check Signature
    TEST_ASSERT_EQUAL('B', header.bytes[0]);
    TEST_ASSERT_EQUAL('M', header.bytes[1]);
    
    // Check File Size (54 + 320 * 240 * 3 = 230454 = 0x038436)
    uint32_t fileSize = header.bytes[2] | (header.bytes[3] << 8) | (header.bytes[4] << 16) | (header.bytes[5] << 24);
    TEST_ASSERT_EQUAL(230454, fileSize);
    
    // Check Data Offset (54)
    uint32_t offset = header.bytes[10] | (header.bytes[11] << 8) | (header.bytes[12] << 16) | (header.bytes[13] << 24);
    TEST_ASSERT_EQUAL(54, offset);
    
    // Check Info Header Size (40)
    uint32_t infoSize = header.bytes[14] | (header.bytes[15] << 8) | (header.bytes[16] << 16) | (header.bytes[17] << 24);
    TEST_ASSERT_EQUAL(40, infoSize);
    
    // Check Width (320)
    uint32_t width = header.bytes[18] | (header.bytes[19] << 8) | (header.bytes[20] << 16) | (header.bytes[21] << 24);
    TEST_ASSERT_EQUAL(320, width);
    
    // Check Height (240)
    uint32_t height = header.bytes[22] | (header.bytes[23] << 8) | (header.bytes[24] << 16) | (header.bytes[25] << 24);
    TEST_ASSERT_EQUAL(240, height);
    
    // Check Planes (1)
    uint16_t planes = header.bytes[26] | (header.bytes[27] << 8);
    TEST_ASSERT_EQUAL(1, planes);
    
    // Check Bit Count (24)
    uint16_t bitCount = header.bytes[28] | (header.bytes[29] << 8);
    TEST_ASSERT_EQUAL(24, bitCount);
    
    // Check Compression (0)
    uint32_t compression = header.bytes[30] | (header.bytes[31] << 8) | (header.bytes[32] << 16) | (header.bytes[33] << 24);
    TEST_ASSERT_EQUAL(0, compression);
}

void test_color_conversion(void) {
    uint8_t r, g, b;
    
    // RGB565 Black: 0x0000 -> 0, 0, 0
    ScreenshotManager::convertRGB565ToBGR24(0x0000, r, g, b);
    TEST_ASSERT_EQUAL(0, r);
    TEST_ASSERT_EQUAL(0, g);
    TEST_ASSERT_EQUAL(0, b);
    
    // RGB565 White: 0xFFFF -> 255, 255, 255
    ScreenshotManager::convertRGB565ToBGR24(0xFFFF, r, g, b);
    TEST_ASSERT_EQUAL(255, r);
    TEST_ASSERT_EQUAL(255, g);
    TEST_ASSERT_EQUAL(255, b);
    
    // RGB565 Pure Red: 0xF800 -> 255, 0, 0
    ScreenshotManager::convertRGB565ToBGR24(0xF800, r, g, b);
    TEST_ASSERT_EQUAL(255, r);
    TEST_ASSERT_EQUAL(0, g);
    TEST_ASSERT_EQUAL(0, b);
    
    // RGB565 Pure Green: 0x07E0 -> 0, 255, 0
    ScreenshotManager::convertRGB565ToBGR24(0x07E0, r, g, b);
    TEST_ASSERT_EQUAL(0, r);
    TEST_ASSERT_EQUAL(255, g);
    TEST_ASSERT_EQUAL(0, b);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_bmp_header_generation);
    RUN_TEST(test_color_conversion);
    return UNITY_END();
}
