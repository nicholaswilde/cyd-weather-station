#include "weather_logger.h"
#include "sd_card_manager.h"
#include <SPI.h>
#include <SD.h>
#include <FS.h>
#include <cstdio>

String WeatherLogger::formatCsvRow(const struct tm& timeinfo, const WeatherData& data) {
    char timeStr[24];
    std::strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &timeinfo);
    
    char csvRow[128];
    // Format: Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode
    std::snprintf(csvRow, sizeof(csvRow), "%s,%.1f,%d,%.1f,%d,%d\n",
                  timeStr, data.temperature, data.humidity, data.windSpeed, data.windDirection, data.weatherCode);
    return String(csvRow);
}

bool WeatherLogger::logWeather(const struct tm& timeinfo, const WeatherData& data) {
    if (!SdCardManager::begin()) {
        Serial.println("[WeatherLogger] Error: SD card not mounted, cannot log weather data.");
        return false;
    }
    
    const char* filePath = "/weather_history.csv";
    bool fileExists = SD.exists(filePath);
    
    File file = SD.open(filePath, "a");
    if (!file) {
        Serial.println("[WeatherLogger] Error: Failed to open weather log file for appending.");
        return false;
    }
    
    if (!fileExists) {
        file.println("Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode");
    }
    
    String row = formatCsvRow(timeinfo, data);
    size_t bytesWritten = file.print(row);
    file.close();
    
    if (bytesWritten == 0) {
        Serial.println("[WeatherLogger] Error: Failed to write data to weather log file.");
        return false;
    }
    
    Serial.printf("[WeatherLogger] Successfully logged weather record: %s", row.c_str());
    return true;
}
