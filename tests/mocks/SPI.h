#ifndef SPI_MOCK_H
#define SPI_MOCK_H

#include <stdint.h>

#define VSPI 3
#define HSPI 2

class SPIClass {
public:
    SPIClass(uint8_t bus) : _bus(bus) {}
    void begin(int8_t sck = -1, int8_t miso = -1, int8_t mosi = -1, int8_t ss = -1) {}
    void end() {}
private:
    uint8_t _bus;
};

extern SPIClass SPI;

#endif // SPI_MOCK_H
