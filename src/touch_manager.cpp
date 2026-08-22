#include "touch_manager.h"

#if defined(NATIVE_TEST)
static bool mockTouched = false;
static int mockX = 0;
static int mockY = 0;

void TouchManager::begin() {}
bool TouchManager::isTouched() { return mockTouched; }
bool TouchManager::getTouchPoint(int& x, int& y) {
    x = mockX;
    y = mockY;
    return mockTouched;
}
void setMockTouch(bool touched, int x, int y) {
    mockTouched = touched;
    mockX = x;
    mockY = y;
}
#elif defined(ILI9341_DRIVER) && !defined(HAS_CAPACITIVE_TOUCH)
#include <Arduino.h>
#include <SPI.h>
#include <XPT2046_Touchscreen.h>

static SPIClass touchSPI(HSPI);
static XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

static bool s_lastTouched = false;
static int s_lastX = 0;
static int s_lastY = 0;
static unsigned long s_lastSampleMs = 0;

static void updateTouchSample() {
    unsigned long now = millis();
    if (now - s_lastSampleMs < 10) {
        return;
    }
    s_lastSampleMs = now;

    if (touch.touched()) {
        TS_Point p = touch.getPoint();
        if (p.x == 0 && p.y == 0) {
            s_lastTouched = false; // Ignore phantom touches
        } else {
            s_lastX = p.x;
            s_lastY = p.y;
            s_lastTouched = true;
        }
    } else {
        s_lastTouched = false;
    }
}

void TouchManager::begin() {
    touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    touch.begin(touchSPI);
    touch.setRotation(1); // landscape
}

bool TouchManager::isTouched() {
    updateTouchSample();
    return s_lastTouched;
}

bool TouchManager::getTouchPoint(int& x, int& y) {
    updateTouchSample();
    x = s_lastX;
    y = s_lastY;
    return s_lastTouched;
}
#elif defined(HAS_CAPACITIVE_TOUCH)
#include <Arduino.h>
#include <Wire.h>

#define I2C_SDA 33
#define I2C_SCL 32
#define GT911_ADDR1 0x5D
#define GT911_ADDR2 0x14
#define CST820_ADDR  0x15

static uint8_t touchAddress = 0x5D;
static bool initialized = false;

static bool s_lastTouched = false;
static int s_lastX = 0;
static int s_lastY = 0;
static unsigned long s_lastTouchTimeMs = 0;
static unsigned long s_lastSampleMs = 0;

static void updateTouchSample() {
    unsigned long now = millis();
    if (now - s_lastSampleMs < 10) {
        return;
    }
    s_lastSampleMs = now;

    if (!initialized) {
        s_lastTouched = false;
        return;
    }

    if (touchAddress == GT911_ADDR1 || touchAddress == GT911_ADDR2) {
        Wire.beginTransmission(touchAddress);
        Wire.write(0x81);
        Wire.write(0x4E);
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(touchAddress, (uint8_t)1);
            if (Wire.available()) {
                uint8_t status = Wire.read();
                bool bufferReady = (status & 0x80) != 0;
                uint8_t touchCount = status & 0x0F;
                
                if (bufferReady) {
                    if (touchCount > 0) {
                        Wire.beginTransmission(touchAddress);
                        Wire.write(0x81);
                        Wire.write(0x50);
                        if (Wire.endTransmission() == 0) {
                            Wire.requestFrom(touchAddress, (uint8_t)4);
                            if (Wire.available() >= 4) {
                                uint8_t xl = Wire.read();
                                uint8_t xh = Wire.read();
                                uint8_t yl = Wire.read();
                                uint8_t yh = Wire.read();
                                s_lastX = (xh << 8) | xl;
                                s_lastY = (yh << 8) | yl;
                                s_lastTouched = true;
                                s_lastTouchTimeMs = now;
                            }
                        }
                    } else {
                        s_lastTouched = false;
                    }
                    
                    // Clear buffer ready flag
                    Wire.beginTransmission(touchAddress);
                    Wire.write(0x81);
                    Wire.write(0x4E);
                    Wire.write(0x00);
                    Wire.endTransmission();
                } else {
                    // GT911 buffer not ready yet (mid-scan cycle).
                    // Maintain s_lastTouched state unless > 50ms elapses without a scan frame
                    if (now - s_lastTouchTimeMs > 50) {
                        s_lastTouched = false;
                    }
                }
            }
        }
    } else if (touchAddress == CST820_ADDR) {
        Wire.beginTransmission(touchAddress);
        Wire.write(0x02);
        if (Wire.endTransmission() == 0) {
            Wire.requestFrom(touchAddress, (uint8_t)1);
            if (Wire.available()) {
                uint8_t touches = Wire.read();
                if (touches > 0) {
                    Wire.beginTransmission(touchAddress);
                    Wire.write(0x03);
                    if (Wire.endTransmission() == 0) {
                        Wire.requestFrom(touchAddress, (uint8_t)4);
                        if (Wire.available() >= 4) {
                            uint8_t xh = Wire.read();
                            uint8_t xl = Wire.read();
                            uint8_t yh = Wire.read();
                            uint8_t yl = Wire.read();
                            s_lastX = ((xh & 0x0F) << 8) | xl;
                            s_lastY = ((yh & 0x0F) << 8) | yl;
                            s_lastTouched = true;
                            s_lastTouchTimeMs = now;
                        }
                    }
                } else {
                    s_lastTouched = false;
                }
            }
        }
    }
}

void TouchManager::begin() {
    Wire.begin(I2C_SDA, I2C_SCL);
    
    Wire.beginTransmission(GT911_ADDR1);
    if (Wire.endTransmission() == 0) {
        touchAddress = GT911_ADDR1;
        initialized = true;
        Serial.println("[Touch] GT911 found at 0x5D");
        return;
    }
    Wire.beginTransmission(GT911_ADDR2);
    if (Wire.endTransmission() == 0) {
        touchAddress = GT911_ADDR2;
        initialized = true;
        Serial.println("[Touch] GT911 found at 0x14");
        return;
    }
    Wire.beginTransmission(CST820_ADDR);
    if (Wire.endTransmission() == 0) {
        touchAddress = CST820_ADDR;
        initialized = true;
        Serial.println("[Touch] CST820 found at 0x15");
        return;
    }
    Serial.println("[Touch] Error: Touch controller not found!");
}

bool TouchManager::isTouched() {
    updateTouchSample();
    return s_lastTouched;
}

bool TouchManager::getTouchPoint(int& x, int& y) {
    updateTouchSample();
    x = s_lastX;
    y = s_lastY;
    return s_lastTouched;
}
#endif

bool TouchManager::isCapacitive() {
#if defined(HAS_CAPACITIVE_TOUCH)
    return true;
#else
    return false;
#endif
}
