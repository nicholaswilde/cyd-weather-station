#ifndef XPT2046_TOUCHSCREEN_MOCK_H
#define XPT2046_TOUCHSCREEN_MOCK_H
#include "Arduino.h"
#include "SPI.h"
struct TS_Point {
    int16_t x;
    int16_t y;
    int16_t z;
};
class XPT2046_Touchscreen {
public:
    XPT2046_Touchscreen(uint8_t cs, uint8_t irq) {}
    void begin(SPIClass &spi) {}
    void setRotation(uint8_t r) {}
    bool touched() { return false; }
    TS_Point getPoint() { return {0, 0, 0}; }
};
#endif
