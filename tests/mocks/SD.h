#ifndef SD_MOCK_H
#define SD_MOCK_H

#include "FS.h"
#include "SPI.h"

extern bool mock_sd_card_present;
extern bool mock_sd_card_mounted;

class SDFS : public FS {
public:
    bool begin(uint8_t ssPin = 5, SPIClass &spi = SPI, uint32_t frequency = 4000000, const char * mountpoint = "/sd", uint8_t maxfiles = 5, bool formatOnFail = false) {
        if (mock_sd_card_present) {
            mock_sd_card_mounted = true;
            return true;
        }
        mock_sd_card_mounted = false;
        return false;
    }
    void end() {
        mock_sd_card_mounted = false;
    }
};

extern SDFS SD;

#endif // SD_MOCK_H
