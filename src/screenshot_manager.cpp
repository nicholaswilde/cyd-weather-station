#include "screenshot_manager.h"
#include <string.h>
#include <stdlib.h>

#ifndef NATIVE_TEST
#include <lvgl.h>
#include "sd_card_manager.h"
#include "settings_manager.h"
#include <SPI.h>
#include <SD.h>
extern SettingsManager settings;
extern SdCardManager sdManager;
#endif

BmpHeader ScreenshotManager::createHeader(uint32_t width, uint32_t height) {
    BmpHeader header;
    memset(header.bytes, 0, sizeof(header.bytes));
    
    uint32_t imageSize = width * height * 3;
    uint32_t fileSize = 54 + imageSize;
    
    // Signature
    header.bytes[0] = 'B';
    header.bytes[1] = 'M';
    
    // File Size
    header.bytes[2] = fileSize & 0xFF;
    header.bytes[3] = (fileSize >> 8) & 0xFF;
    header.bytes[4] = (fileSize >> 16) & 0xFF;
    header.bytes[5] = (fileSize >> 24) & 0xFF;
    
    // Offset to pixel data (54)
    header.bytes[10] = 54;
    
    // Info Header Size (40)
    header.bytes[14] = 40;
    
    // Width
    header.bytes[18] = width & 0xFF;
    header.bytes[19] = (width >> 8) & 0xFF;
    header.bytes[20] = (width >> 16) & 0xFF;
    header.bytes[21] = (width >> 24) & 0xFF;
    
    // Height
    header.bytes[22] = height & 0xFF;
    header.bytes[23] = (height >> 8) & 0xFF;
    header.bytes[24] = (height >> 16) & 0xFF;
    header.bytes[25] = (height >> 24) & 0xFF;
    
    // Planes (1)
    header.bytes[26] = 1;
    header.bytes[27] = 0;
    
    // Bit Count (24)
    header.bytes[28] = 24;
    header.bytes[29] = 0;
    
    // Image Size
    header.bytes[34] = imageSize & 0xFF;
    header.bytes[35] = (imageSize >> 8) & 0xFF;
    header.bytes[36] = (imageSize >> 16) & 0xFF;
    header.bytes[37] = (imageSize >> 24) & 0xFF;
    
    return header;
}

void ScreenshotManager::convertRGB565ToBGR24(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t rawR = (rgb565 >> 11) & 0x1F;
    uint8_t rawG = (rgb565 >> 5) & 0x3F;
    uint8_t rawB = rgb565 & 0x1F;
    
    r = (rawR * 255) / 31;
    g = (rawG * 255) / 63;
    b = (rawB * 255) / 31;
}

#ifndef NATIVE_TEST
bool ScreenshotManager::captureToSD(const char* filepath) {
    lv_obj_t* screen = lv_scr_act();
    if (!screen) {
        Serial.println("[Screenshot] Error: Active screen is null.");
        return false;
    }
    
    uint32_t width = lv_obj_get_width(screen);
    uint32_t height = lv_obj_get_height(screen);
    uint32_t dataSize = width * height * sizeof(lv_color_t);
    
    // Allocate buffer for 16-bit RGB565 LVGL snapshot
    uint8_t* imgData = (uint8_t*)malloc(dataSize);
    if (!imgData) {
        Serial.println("[Screenshot] Error: Failed to allocate snapshot buffer.");
        return false;
    }
    
    lv_img_dsc_t snapshot;
    snapshot.data = imgData;
    
    lv_res_t res = lv_snapshot_take_to_buf(screen, LV_IMG_CF_TRUE_COLOR, &snapshot, imgData, dataSize);
    if (res != LV_RES_OK) {
        Serial.println("[Screenshot] Error: LVGL snapshot failed.");
        free(imgData);
        return false;
    }
    
    // Make sure SD is mounted
    bool wasMounted = sdManager.isMounted();
    if (!wasMounted) {
        if (!sdManager.begin()) {
            Serial.println("[Screenshot] Error: Failed to mount SD card.");
            free(imgData);
            return false;
        }
    }
    
    File file = SD.open(filepath, FILE_WRITE);
    if (!file) {
        Serial.println("[Screenshot] Error: Failed to open file for writing.");
        if (!wasMounted) {
            sdManager.end();
        }
        free(imgData);
        return false;
    }
    
    // Write BMP header
    BmpHeader header = createHeader(width, height);
    file.write(header.bytes, sizeof(header.bytes));
    
    // Write pixel rows bottom-to-top (standard BMP format)
    lv_color_t* pixels = (lv_color_t*)imgData;
    uint8_t* rowBuffer = (uint8_t*)malloc(width * 3);
    if (!rowBuffer) {
        Serial.println("[Screenshot] Error: Failed to allocate row buffer.");
        file.close();
        if (!wasMounted) {
            sdManager.end();
        }
        free(imgData);
        return false;
    }
    
    for (int y = (int)height - 1; y >= 0; y--) {
        for (uint32_t x = 0; x < width; x++) {
            lv_color_t pixel = pixels[y * width + x];
            uint8_t r, g, b;
            convertRGB565ToBGR24(pixel.full, r, g, b);
            
            rowBuffer[x * 3] = b;     // Blue first in BGR
            rowBuffer[x * 3 + 1] = g; // Green
            rowBuffer[x * 3 + 2] = r; // Red
        }
        file.write(rowBuffer, width * 3);
    }
    
    free(rowBuffer);
    file.close();
    
    if (!wasMounted) {
        sdManager.end();
    }
    
    free(imgData);
    Serial.printf("[Screenshot] Successfully saved to SD: %s\n", filepath);
    return true;
}
#endif
