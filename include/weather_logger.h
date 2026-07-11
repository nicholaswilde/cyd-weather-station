#ifndef WEATHER_LOGGER_H
#define WEATHER_LOGGER_H

#include <Arduino.h>
#include <ctime>
#include "weather_client.h"

class WeatherLogger {
public:
    static String formatCsvRow(const struct tm& timeinfo, const WeatherData& data);
    static bool logWeather(const struct tm& timeinfo, const WeatherData& data);
};

#endif // WEATHER_LOGGER_H
