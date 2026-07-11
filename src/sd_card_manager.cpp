#include "sd_card_manager.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include "config/config.h"

static bool sdMounted = false;
static bool sdCardDetected = false;

bool SdCardManager::begin() {
    if (sdMounted) {
        return true;
    }
    
    Serial.println("Initializing SD card...");
    
    SPI.begin(SD_SCK_PIN, SD_MISO_PIN, SD_MOSI_PIN, SD_CS_PIN);
    sdMounted = SD.begin(SD_CS_PIN, SPI, 4000000, "/sd", 5, true);

    if (sdMounted) {
        Serial.println("SD card initialized successfully.");
        sdCardDetected = true;
    } else {
        Serial.println("Failed to initialize SD card.");
        sdCardDetected = false;
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

bool SdCardManager::isCardPresent() {
    return sdCardDetected;
}
