#include "weather_client.h"
#include <ArduinoJson.h>
#include "config/config.h"
#include "settings_manager.h"

extern SettingsManager settings;

#ifndef NATIVE_TEST
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#endif

WeatherClient::WeatherClient(const char* latitude, const char* longitude)
    : _latitude(latitude), _longitude(longitude), _zipCode(nullptr), _useZip(false), _zipResolved(false) {}

WeatherClient::WeatherClient(const char* zipCode)
    : _latitude(nullptr), _longitude(nullptr), _zipCode(zipCode), _useZip(true), _zipResolved(false) {}

bool WeatherClient::geocodeZip() {
#ifdef NATIVE_TEST
    _resolvedLat = "34.0736";
    _resolvedLng = "-118.4004";
    _zipResolved = true;
    return true;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot geocode zip code.");
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://geocoding-api.open-meteo.com/v1/search?name=";
    url += _zipCode;
    url += "&count=1&language=en&format=json";

    Serial.print("[Weather] Geocoding Zip Code: ");
    Serial.println(_zipCode);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error && doc["results"].is<JsonArray>() && doc["results"].size() > 0) {
                float lat = doc["results"][0]["latitude"] | 0.0f;
                float lng = doc["results"][0]["longitude"] | 0.0f;
                const char* name = doc["results"][0]["name"] | "Unknown";

                _resolvedLat = String(lat, 5);
                _resolvedLng = String(lng, 5);
                _zipResolved = true;

                Serial.printf("[Weather] Zip %s resolved to %s (%s, %s)\n",
                    _zipCode, name, _resolvedLat.c_str(), _resolvedLng.c_str());
                http.end();
                return true;
            } else {
                Serial.print("[Weather] Geocoding JSON parsing failed or empty: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("[Weather] Geocoding HTTP GET failed: %d\n", httpCode);
        }
        http.end();
    }
    return false;
#endif
}

WeatherData WeatherClient::fetchWeather() {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, {} };

    // Resolve zip code if using zip code and not yet resolved
    if (_useZip && !_zipResolved) {
        if (!geocodeZip()) {
            Serial.println("[Weather] Failed to resolve zip code. Cannot fetch weather.");
            return data;
        }
    }

    const char* lat = _useZip ? _resolvedLat.c_str() : _latitude;
    const char* lng = _useZip ? _resolvedLng.c_str() : _longitude;

#ifdef NATIVE_TEST
    // Mock response for native unit tests
    const char* mockJson = "{"
        "\"current\":{"
            "\"temperature_2m\":21.5,"
            "\"relative_humidity_2m\":60,"
            "\"weather_code\":0,"
            "\"wind_speed_10m\":8.8"
        "},"
        "\"daily\":{"
            "\"time\":[\"2026-07-09\",\"2026-07-10\",\"2026-07-11\"],"
            "\"weather_code\":[0,1,2],"
            "\"temperature_2m_max\":[25.5,26.0,24.8],"
            "\"temperature_2m_min\":[15.0,14.8,15.2]"
        "}"
    "}";
    parseWeatherJson(mockJson, data);
    return data;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch weather.");
        return data;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://api.open-meteo.com/v1/forecast?latitude=";
    url += lat;
    url += "&longitude=";
    url += lng;
    url += "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m";
    url += "&daily=weather_code,temperature_2m_max,temperature_2m_min&forecast_days=3";
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        url += "&temperature_unit=fahrenheit";
        url += "&windspeed_unit=mph";
    }

    Serial.print("[Weather] Fetching URL: ");
    Serial.println(url);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            if (parseWeatherJson(payload.c_str(), data)) {
                Serial.printf("[Weather] Success! Temp: %.1f, Hum: %d %%, Status: %s, Wind: %.1f\n",
                    data.temperature, data.humidity, data.status.c_str(), data.windSpeed);
            }
        } else {
            Serial.printf("[Weather] HTTP GET failed, error code: %d\n", httpCode);
        }
        http.end();
    } else {
        Serial.println("[Weather] Unable to connect to Open-Meteo API");
    }

    return data;
#endif
}

bool WeatherClient::parseWeatherJson(const char* json, WeatherData& data) {
    StaticJsonDocument<2048> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[Weather] JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    // Parse current weather
    if (doc.containsKey("current")) {
        data.temperature = doc["current"]["temperature_2m"].as<float>();
        data.humidity = doc["current"]["relative_humidity_2m"].as<int>();
        int code = doc["current"]["weather_code"].is<int>() ? doc["current"]["weather_code"].as<int>() : -1;
        data.status = getWeatherDesc(code);
        data.windSpeed = doc["current"]["wind_speed_10m"].as<float>();
        data.valid = true;
        data.weatherCode = code;
    }

    // Parse daily forecast
    if (doc.containsKey("daily")) {
        JsonArray daily_time = doc["daily"]["time"];
        JsonArray daily_max = doc["daily"]["temperature_2m_max"];
        JsonArray daily_min = doc["daily"]["temperature_2m_min"];
        JsonArray daily_code = doc["daily"]["weather_code"];

        for (int i = 0; i < 3; i++) {
            if (i < (int)daily_time.size()) {
                data.forecast[i].tempMax = daily_max[i].as<float>();
                data.forecast[i].tempMin = daily_min[i].as<float>();
                int code = daily_code[i].as<int>();
                data.forecast[i].weatherCode = code;
                data.forecast[i].status = getWeatherDesc(code);
                
                String dateStr = daily_time[i].as<String>();
                if (i == 0) {
                    data.forecast[i].dayName = "Today";
                } else if (i == 1) {
                    data.forecast[i].dayName = "Tomorrow";
                } else if (dateStr.length() >= 10) {
                    const char* dateCStr = dateStr.c_str();
                    int y = (dateCStr[0]-'0')*1000 + (dateCStr[1]-'0')*100 + (dateCStr[2]-'0')*10 + (dateCStr[3]-'0');
                    int m = (dateCStr[5]-'0')*10 + (dateCStr[6]-'0');
                    int d = (dateCStr[8]-'0')*10 + (dateCStr[9]-'0');
                    
                    static const char* days[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
                    int yr = y;
                    if (m < 3) yr -= 1;
                    static int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
                    int dayIdx = (yr + yr/4 - yr/100 + yr/400 + t[m-1] + d) % 7;
                    if (dayIdx >= 0 && dayIdx < 7) {
                        data.forecast[i].dayName = days[dayIdx];
                    } else {
                        data.forecast[i].dayName = dateStr;
                    }
                } else {
                    data.forecast[i].dayName = dateStr;
                }
            } else {
                data.forecast[i].tempMax = 0.0f;
                data.forecast[i].tempMin = 0.0f;
                data.forecast[i].weatherCode = -1;
                data.forecast[i].status = "Unknown";
                data.forecast[i].dayName = "N/A";
            }
        }
    }

    return data.valid;
}

String WeatherClient::getWeatherDesc(int code) {
    switch (code) {
        case 0: return "Clear sky";
        case 1: return "Mainly clear";
        case 2: return "Partly cloudy";
        case 3: return "Overcast";
        case 45: return "Foggy";
        case 48: return "Depositing rime fog";
        case 51: return "Light drizzle";
        case 53: return "Moderate drizzle";
        case 55: return "Dense drizzle";
        case 56: return "Light freezing drizzle";
        case 57: return "Dense freezing drizzle";
        case 61: return "Slight rain";
        case 63: return "Moderate rain";
        case 65: return "Heavy rain";
        case 66: return "Light freezing rain";
        case 67: return "Heavy freezing rain";
        case 71: return "Slight snow fall";
        case 73: return "Moderate snow fall";
        case 75: return "Heavy snow fall";
        case 77: return "Snow grains";
        case 80: return "Slight rain showers";
        case 81: return "Moderate rain showers";
        case 82: return "Violent rain showers";
        case 85: return "Slight snow showers";
        case 86: return "Heavy snow showers";
        case 95: return "Thunderstorm";
        case 96: return "Thunderstorm with slight hail";
        case 99: return "Thunderstorm with heavy hail";
        default: return "Unknown";
    }
}
