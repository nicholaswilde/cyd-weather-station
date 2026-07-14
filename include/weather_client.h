#ifndef WEATHER_CLIENT_H
#define WEATHER_CLIENT_H

#include <Arduino.h>

struct ForecastDay {
    float tempMax;
    float tempMin;
    int weatherCode;
    String status;
    String dayName;
};

struct HourlyForecast {
    float temperature;
    int precipitationProbability;
};

struct WeatherData {
    float temperature;
    int humidity;
    String status;
    bool valid;
    int weatherCode;
    float windSpeed;
    int windDirection;
    String cityName;
    ForecastDay forecast[3];
    HourlyForecast hourly[24];
};

class WeatherClient {
public:
    WeatherClient(const char* latitude, const char* longitude);
    WeatherClient(const char* zipCode);
    WeatherData fetchWeather();
    const String& getCityName() const { return _cityName; }
    static String getWeatherDesc(int code);
    static bool parseWeatherJson(const char* json, WeatherData& data);
    bool parseOwmJson(const char* json, WeatherData& data);
    static String serializeWeatherData(const WeatherData& data);
    static bool deserializeWeatherData(const String& json, WeatherData& data);
    bool isLocationEmpty() const;
    bool fetchIpLocation(String& latStr, String& lonStr, String& city);
    static bool parseIpLocationJson(const char* json, float& lat, float& lon, String& city);

private:
    bool geocodeZip();
    bool reverseGeocode();
    static int owmToWmoCode(int owmCode);

    const char* _latitude;
    const char* _longitude;
    const char* _zipCode;
    bool _useZip;
    bool _zipResolved;
    String _resolvedLat;
    String _resolvedLng;
    String _cityName;
};

#endif // WEATHER_CLIENT_H
