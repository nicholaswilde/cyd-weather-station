#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <Arduino.h>

struct WeatherData {
    float temperature;
    int humidity;
    String status;
    bool valid;
    int weatherCode;
    float windSpeed;
};

class WeatherClient {
public:
    WeatherClient(const char* latitude, const char* longitude);
    WeatherClient(const char* zipCode);
    WeatherData fetchWeather();
    static String getWeatherDesc(int code);

private:
    bool geocodeZip();

    const char* _latitude;
    const char* _longitude;
    const char* _zipCode;
    bool _useZip;
    bool _zipResolved;
    String _resolvedLat;
    String _resolvedLng;
};

#endif // WEATHER_CLIENT_H
