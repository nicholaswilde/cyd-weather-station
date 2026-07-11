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
    static BmpHeader createHeader(uint32_t width, uint32_t height);
    static void convertRGB565ToBGR24(uint16_t rgb565, uint8_t& r, uint8_t& g, uint8_t& b);
    
#ifndef NATIVE_TEST
    static bool captureToSD(const char* filepath);
#endif
};

#endif // SCREENSHOT_MANAGER_H
