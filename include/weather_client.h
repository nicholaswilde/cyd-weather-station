#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <Arduino.h>

struct WeatherData {
    float temperature;
    int humidity;
    String status;
    bool valid;
};

class WeatherClient {
public:
    WeatherClient(const char* latitude, const char* longitude);
    WeatherData fetchWeather();
    static String getWeatherDesc(int code);

private:
    const char* _latitude;
    const char* _longitude;
};

#endif // WEATHER_CLIENT_H
