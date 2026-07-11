#ifndef SCREENSHOT_MANAGER_H
#define SCREENSHOT_MANAGER_H

#include <stdint.h>

#ifndef NATIVE_TEST
#include <Arduino.h>
#include <FS.h>
#else
#include "Arduino.h"
#include <string>
#endif

struct BmpHeader {
    uint8_t bytes[54];
};

class ScreenshotManager {
public:
    // --- BMP utility methods (host + target) ---
    static BmpHeader createHeader(uint32_t width, uint32_t height);
    static void convertRGB565ToBGR24(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b);
    static std::string generateFilename(const struct tm* timeinfo, unsigned long fallbackMillis);

    // --- Flush-interception capture API (host + target) ---
    // Call before forcing a full LVGL redraw; opens the SD file and writes header.
    static bool beginCapture(const char* filepath);

    // Called from disp_flush callback for every rendered tile.
    static void onFlushTile(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const uint16_t* pixels);

    // Called once the full frame has been captured; closes the SD file.
    static void endCapture();

    // Returns true if a capture is currently in progress.
    static bool isCaptureInProgress();

    // Convenience: trigger a full capture sequence to SD.
    static bool captureToSD(const char* filepath);
};

#endif // SCREENSHOT_MANAGER_H
