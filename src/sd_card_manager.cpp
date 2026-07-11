#include "sd_card_manager.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include "config/config.h"

static SPIClass sdSPI(VSPI);
static bool sdMounted = false;

bool SdCardManager::begin() {
    if (sdMounted) {
        return true;
    }
    
    Serial.println("Initializing SD card...");
    
    static bool spiInitialized = false;
    if (!spiInitialized) {
        sdSPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
        spiInitialized = true;
    }
    sdMounted = SD.begin(SD_CS_PIN, sdSPI, 4000000);

    if (sdMounted) {
        Serial.println("SD card initialized successfully.");
    } else {
        Serial.println("Failed to initialize SD card.");
    }
    return sdMounted;
}

bool SdCardManager::isMounted() {
    return sdMounted;
}

void SdCardManager::end() {
    if (sdMounted) {
        SD.end();
        sdMounted = false;
        Serial.println("SD card unmounted.");
    }
}
