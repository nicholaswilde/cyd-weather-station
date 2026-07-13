#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <stdarg.h>

// --- FreeRTOS Mocks ---
#ifndef FREERTOS_MOCKS_H
#define FREERTOS_MOCKS_H
typedef void* TimerHandle_t;
typedef void (*TimerCallbackFunction_t)(TimerHandle_t xTimer);

#define pdFALSE (0)
#define pdTRUE (1)

inline void* xTimerCreate(const char* name, int period, int autoReload, void* id, TimerCallbackFunction_t callback) { return (void*)1; }
inline void xTimerStart(void* timer, int ticks) {}
inline void xTimerStop(void* timer, int ticks) {}
inline void* pvTimerGetTimerID(void* timer) { return nullptr; }
#define pdMS_TO_TICKS(x) (x)
#endif // FREERTOS_MOCKS_H


// --- Wi-Fi Mocks ---
#define WL_CONNECTED 3
#define WL_CONNECT_FAILED 4
#define WL_NO_SSID_AVAIL 1
#define WIFI_STA 1
typedef int wl_status_t;

// --- GPIO Mocks ---
#define OUTPUT 0x03
#define INPUT 0x01
#define HIGH 0x01
#define LOW 0x00

extern uint8_t mock_pin_modes[100];
extern uint8_t mock_pin_states[100];
extern unsigned long mock_millis_val;

inline unsigned long millis() { return mock_millis_val; }
inline void pinMode(uint8_t pin, uint8_t mode) { if (pin < 100) mock_pin_modes[pin] = mode; }
inline void digitalWrite(uint8_t pin, uint8_t val) { if (pin < 100) mock_pin_states[pin] = val; }

// --- Class Mocks ---
class String : public std::string {
public:
    String(const char* s) : std::string(s) {}
    String(std::string s) : std::string(s) {}
    String() : std::string() {}
    const char* c_str() const { return std::string::c_str(); }
};

class IPAddress {
public:
    String toString() const { return "192.168.1.100"; }
};

class SerialMock {
public:
    void println(const char* s) { std::cout << s << std::endl; }
    void println(std::string s) { std::cout << s << std::endl; }
    void println(IPAddress ip) { std::cout << ip.toString().c_str() << std::endl; }
    void print(const char* s) { std::cout << s; }
    void printf(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
};

extern SerialMock Serial;

inline long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// --- SPI Pins ---
#define XPT2046_CS 33
#define XPT2046_IRQ 36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK 25

#endif // ARDUINO_MOCK_H
