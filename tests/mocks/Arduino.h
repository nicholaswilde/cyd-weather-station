#ifndef ARDUINO_MOCK_H
#define ARDUINO_MOCK_H

#include <stdint.h>
#include <string>
#include <iostream>
#include <stdarg.h>

#define WL_CONNECTED 3
#define WL_CONNECT_FAILED 4
#define WL_NO_SSID_AVAIL 1
#define WIFI_STA 1

typedef int wl_status_t;

inline unsigned long millis() {
    static unsigned long t = 0;
    t += 1000; // Mock time advancement
    return t;
}

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

#endif // ARDUINO_MOCK_H
