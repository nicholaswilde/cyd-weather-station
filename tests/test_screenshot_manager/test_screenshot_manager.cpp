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
    
    // Check Height: negative = top-down BMP (-240 = 0xFFFFFF10)
    int32_t height = (int32_t)(header.bytes[22] | (header.bytes[23] << 8) | (header.bytes[24] << 16) | (header.bytes[25] << 24));
    TEST_ASSERT_EQUAL(-240, height);
    
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

void test_filename_generation(void) {
    struct tm timeinfo;
    timeinfo.tm_year = 126; // 2026
    timeinfo.tm_mon = 6;    // July (0-indexed)
    timeinfo.tm_mday = 11;
    timeinfo.tm_hour = 13;
    timeinfo.tm_min = 45;
    timeinfo.tm_sec = 30;
    
    std::string filename = ScreenshotManager::generateFilename(&timeinfo, 12345);
    TEST_ASSERT_EQUAL_STRING("/screenshot_20260711_134530.bmp", filename.c_str());
    
    std::string fallback = ScreenshotManager::generateFilename(nullptr, 12345);
    TEST_ASSERT_EQUAL_STRING("/screenshot_12345.bmp", fallback.c_str());
}

void test_capture_to_sd_success(void) {
    mock_sd_card_present = true;
    mock_sd_card_mounted = false;
    mock_files.clear();
    
    bool result = ScreenshotManager::captureToSD("/test_screenshot.bmp");
    TEST_ASSERT_TRUE(result);
    TEST_ASSERT_TRUE(mock_files.find("/test_screenshot.bmp") != mock_files.end());
    TEST_ASSERT_EQUAL(230454, mock_files["/test_screenshot.bmp"].size());
}

void test_begin_capture_flush_end(void) {
    mock_sd_card_present = true;
    mock_sd_card_mounted = false;
    mock_files.clear();
    
    TEST_ASSERT_FALSE(ScreenshotManager::isCaptureInProgress());
    TEST_ASSERT_TRUE(ScreenshotManager::beginCapture("/flush_test.bmp"));
    TEST_ASSERT_TRUE(ScreenshotManager::isCaptureInProgress());
    
    // Simulate two horizontal tile flushes covering full 320x240 frame
    uint16_t tile[320 * 120] = {};
    ScreenshotManager::onFlushTile(0,   0, 319, 119, tile);
    ScreenshotManager::onFlushTile(0, 120, 319, 239, tile);
    
    ScreenshotManager::endCapture();
    TEST_ASSERT_FALSE(ScreenshotManager::isCaptureInProgress());
    TEST_ASSERT_EQUAL(230454, mock_files["/flush_test.bmp"].size());
}

void test_capture_to_sd_no_card(void) {
    mock_sd_card_present = false;
    mock_sd_card_mounted = false;
    mock_files.clear();
    
    bool result = ScreenshotManager::captureToSD("/test_screenshot.bmp");
    TEST_ASSERT_FALSE(result);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_bmp_header_generation);
    RUN_TEST(test_color_conversion);
    RUN_TEST(test_filename_generation);
    RUN_TEST(test_capture_to_sd_success);
    RUN_TEST(test_begin_capture_flush_end);
    RUN_TEST(test_capture_to_sd_no_card);
    return UNITY_END();
}
