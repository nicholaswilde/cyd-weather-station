#ifndef SD_CARD_MANAGER_H
#define SD_CARD_MANAGER_H

#include <Arduino.h>

class SdCardManager {
public:
    static bool begin();
    static bool isMounted();
    static void end();
    static bool isCardPresent();
};

#endif // SD_CARD_MANAGER_H
