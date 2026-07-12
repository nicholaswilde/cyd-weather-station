#ifndef WEATHER_CACHE_H
#define WEATHER_CACHE_H

#include "weather_client.h"

class WeatherCache {
public:
    static bool saveCache(const WeatherData& data);
    static bool loadCache(WeatherData& data);
};

#endif // WEATHER_CACHE_H
