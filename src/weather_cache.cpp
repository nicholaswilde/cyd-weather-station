#include "weather_cache.h"
#include "sd_card_manager.h"
#include <SD.h>
#include <FS.h>

bool WeatherCache::saveCache(const WeatherData& data) {
    if (!SdCardManager::begin()) {
        Serial.println("[WeatherCache] Error: SD card not mounted, cannot save cache.");
        return false;
    }

    String json = WeatherClient::serializeWeatherData(data);
    if (json.length() == 0) {
        Serial.println("[WeatherCache] Error: Serialization produced empty string.");
        return false;
    }

    File file = SD.open("/weather_cache.json", "w");
    if (!file) {
        Serial.println("[WeatherCache] Error: Failed to open cache file for writing.");
        return false;
    }

    size_t bytesWritten = file.print(json);
    file.close();

    if (bytesWritten == 0) {
        Serial.println("[WeatherCache] Error: Failed to write JSON cache data.");
        return false;
    }

    Serial.println("[WeatherCache] Successfully wrote weather cache to SD.");
    return true;
}

bool WeatherCache::loadCache(WeatherData& data) {
    if (!SdCardManager::begin()) {
        Serial.println("[WeatherCache] Error: SD card not mounted, cannot load cache.");
        return false;
    }

    if (!SD.exists("/weather_cache.json")) {
        Serial.println("[WeatherCache] Error: Cache file does not exist.");
        return false;
    }

    File file = SD.open("/weather_cache.json", "r");
    if (!file) {
        Serial.println("[WeatherCache] Error: Failed to open cache file for reading.");
        return false;
    }

    String json = "";
    while (file.available()) {
        json += (char)file.read();
    }
    file.close();

    bool success = WeatherClient::deserializeWeatherData(json, data);
    if (!success) {
        Serial.println("[WeatherCache] Error: Deserialization failed.");
        return false;
    }

    Serial.println("[WeatherCache] Successfully restored weather data from SD cache.");
    return true;
}
