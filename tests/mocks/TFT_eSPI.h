#ifndef TFT_ESPI_MOCK_H
#define TFT_ESPI_MOCK_H
#include "Arduino.h"
#define TFT_BLACK 0
class TFT_eSPI {
public:
    void init() {}
    void setRotation(uint8_t r) {}
    void fillScreen(uint32_t color) {}
    void startWrite() {}
    void setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h) {}
    void pushColors(uint16_t *data, uint32_t len, bool swap = true) {}
    void endWrite() {}
};
#endif
