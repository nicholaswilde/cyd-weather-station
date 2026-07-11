#include "screenshot_manager.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

// --- Platform headers ---
#ifndef NATIVE_TEST
#include <lvgl.h>
#include <SPI.h>
#include <SD.h>
#include "sd_card_manager.h"
#else
#include <time.h>
#include "SD.h"
#include "FS.h"
#endif

// --- Internal capture state ---
static File     _captureFile;
static bool     _captureInProgress = false;
static uint32_t _captureWidth  = 320;
static uint32_t _captureHeight = 240;

// --- BMP pixel data begins at byte 54 ---
static const uint32_t BMP_HEADER_SIZE = 54;

/**
 * @brief Constructs a 54-byte BMP file header for a 24-bit top-down image.
 *        A negative height field signals top-down row order, matching LVGL.
 */
BmpHeader ScreenshotManager::createHeader(uint32_t width, uint32_t height) {
    BmpHeader header;
    memset(header.bytes, 0, sizeof(header.bytes));

    uint32_t imageSize = width * height * 3;
    uint32_t fileSize  = BMP_HEADER_SIZE + imageSize;

    // --- BMP file header (14 bytes) ---
    header.bytes[0]  = 'B';
    header.bytes[1]  = 'M';
    header.bytes[2]  = fileSize & 0xFF;
    header.bytes[3]  = (fileSize >> 8)  & 0xFF;
    header.bytes[4]  = (fileSize >> 16) & 0xFF;
    header.bytes[5]  = (fileSize >> 24) & 0xFF;
    header.bytes[10] = BMP_HEADER_SIZE; // pixel data offset

    // --- DIB header (BITMAPINFOHEADER, 40 bytes) ---
    header.bytes[14] = 40; // header size

    // Width (positive)
    header.bytes[18] = width  & 0xFF;
    header.bytes[19] = (width  >> 8)  & 0xFF;
    header.bytes[20] = (width  >> 16) & 0xFF;
    header.bytes[21] = (width  >> 24) & 0xFF;

    // Height: NEGATIVE = top-down row order (matches LVGL flush order)
    int32_t negHeight = -(int32_t)height;
    header.bytes[22] = negHeight & 0xFF;
    header.bytes[23] = (negHeight >> 8)  & 0xFF;
    header.bytes[24] = (negHeight >> 16) & 0xFF;
    header.bytes[25] = (negHeight >> 24) & 0xFF;

    // Color planes (1)
    header.bytes[26] = 1;
    // Bits per pixel (24)
    header.bytes[28] = 24;

    // Image data size
    header.bytes[34] = imageSize & 0xFF;
    header.bytes[35] = (imageSize >> 8)  & 0xFF;
    header.bytes[36] = (imageSize >> 16) & 0xFF;
    header.bytes[37] = (imageSize >> 24) & 0xFF;

    return header;
}

/**
 * @brief Converts a packed RGB565 pixel into separate 8-bit R, G, B components.
 */
void ScreenshotManager::convertRGB565ToBGR24(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b) {
    uint8_t rawR = (rgb565 >> 11) & 0x1F;
    uint8_t rawG = (rgb565 >> 5)  & 0x3F;
    uint8_t rawB =  rgb565        & 0x1F;

    r = (rawR * 255) / 31;
    g = (rawG * 255) / 63;
    b = (rawB * 255) / 31;
}

/**
 * @brief Generates a timestamped BMP filename; falls back to millis if time unavailable.
 */
std::string ScreenshotManager::generateFilename(const struct tm* timeinfo, unsigned long fallbackMillis) {
    char filename[64];
    if (timeinfo) {
        strftime(filename, sizeof(filename), "/screenshot_%Y%m%d_%H%M%S.bmp", timeinfo);
    } else {
        snprintf(filename, sizeof(filename), "/screenshot_%lu.bmp", fallbackMillis);
    }
    return std::string(filename);
}

/**
 * @brief Returns true if a flush-interception capture is currently in progress.
 */
bool ScreenshotManager::isCaptureInProgress() {
    return _captureInProgress;
}

/**
 * @brief Opens the SD file, writes the BMP header, pre-fills pixel area with
 *        zeros (enabling random-access seek+write per tile), and sets the
 *        capture-in-progress flag.
 */
bool ScreenshotManager::beginCapture(const char* filepath) {
    if (_captureInProgress) {
        Serial.println("[Screenshot] beginCapture: capture already in progress.");
        return false;
    }

    // --- Mount SD if needed ---
#ifndef NATIVE_TEST
    bool wasMounted = SdCardManager::isMounted();
    if (!wasMounted && !SdCardManager::begin()) {
        Serial.println("[Screenshot] Error: Failed to mount SD card.");
        return false;
    }
    _captureFile = SD.open(filepath, FILE_WRITE);
#else
    extern bool mock_sd_card_present;
    extern bool mock_sd_card_mounted;
    if (!mock_sd_card_present) {
        return false;
    }
    mock_sd_card_mounted = true;
    _captureFile = SD.open(filepath, "w");
#endif

    if (!_captureFile) {
        Serial.println("[Screenshot] Error: Failed to open file for writing.");
        return false;
    }

    // --- Write BMP header (54 bytes) ---
    BmpHeader header = createHeader(_captureWidth, _captureHeight);
    _captureFile.write(header.bytes, sizeof(header.bytes));

    // --- Pre-fill entire pixel area with zeros using a small stack buffer.
    //     This reserves the space so later seek+write calls land in bounds. ---
    uint8_t zeroBuf[_captureWidth * 3]; // 960 bytes on stack
    memset(zeroBuf, 0, sizeof(zeroBuf));
    for (uint32_t row = 0; row < _captureHeight; row++) {
        _captureFile.write(zeroBuf, sizeof(zeroBuf));
    }

    _captureInProgress = true;
    Serial.printf("[Screenshot] Capture started: %s\n", filepath);
    return true;
}

/**
 * @brief Receives one LVGL flush tile and seeks to the correct BMP file offset
 *        to write those pixels — correctly handles any tile size or position.
 */
void ScreenshotManager::onFlushTile(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint16_t* pixels) {
    if (!_captureInProgress || !_captureFile) return;

    uint32_t tileW = (uint32_t)(x2 - x1 + 1);
    uint32_t tileH = (uint32_t)(y2 - y1 + 1);

    // --- BGR row buffer sized to the tile width (max 320*3 = 960 bytes) ---
    uint8_t rowBuf[320 * 3];

    for (uint32_t row = 0; row < tileH; row++) {
        const uint16_t* srcRow = pixels + row * tileW;

        // --- Convert this tile row from RGB565 to BGR24 ---
        for (uint32_t col = 0; col < tileW; col++) {
            uint8_t r, g, b;
            convertRGB565ToBGR24(srcRow[col], r, g, b);
            rowBuf[col * 3]     = b; // BGR order
            rowBuf[col * 3 + 1] = g;
            rowBuf[col * 3 + 2] = r;
        }

        // --- Seek to the exact file offset for this row's x1 column.
        //     top-down BMP: row (y1+row) starts at byte 54 + (y1+row)*width*3
        //     then x1 columns in: + x1*3                                   ---
        uint32_t absRow  = (uint32_t)y1 + row;
        uint32_t fileOff = BMP_HEADER_SIZE + absRow * _captureWidth * 3 + (uint32_t)x1 * 3;
        _captureFile.seek(fileOff);
        _captureFile.write(rowBuf, tileW * 3);
    }
}

/**
 * @brief Closes the SD file and clears the capture-in-progress flag.
 */
void ScreenshotManager::endCapture() {
    if (!_captureInProgress) return;
    _captureFile.close();
    _captureInProgress = false;

#ifdef NATIVE_TEST
    extern bool mock_sd_card_mounted;
    mock_sd_card_mounted = false;
#endif

    Serial.println("[Screenshot] Capture complete.");
}

/**
 * @brief Convenience wrapper: opens the file, forces a full LVGL redraw so
 *        every pixel is flushed through onFlushTile, then closes the file.
 */
bool ScreenshotManager::captureToSD(const char* filepath) {
    if (!beginCapture(filepath)) return false;

#ifndef NATIVE_TEST
    // --- Force LVGL to re-render the entire screen through the flush callback ---
    lv_obj_invalidate(lv_scr_act());
    lv_refr_now(NULL);
#else
    // --- Native test: simulate tiles covering the full 320x240 frame ---
    uint16_t tile[320 * 120] = {};
    onFlushTile(0,   0, 319, 119, tile);
    onFlushTile(0, 120, 319, 239, tile);
#endif

    endCapture();
    return true;
}
