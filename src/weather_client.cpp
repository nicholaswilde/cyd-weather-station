#include "weather_client.h"
#include <ArduinoJson.h>
#include "config/config.h"

#ifndef NATIVE_TEST
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#endif

WeatherClient::WeatherClient(const char* latitude, const char* longitude)
    : _latitude(latitude), _longitude(longitude) {}

WeatherData WeatherClient::fetchWeather() {
    WeatherData data = { 0.0f, 0, "Unknown", false };

#ifdef NATIVE_TEST
    // Mock response for native unit tests
    data.temperature = 21.5f;
    data.humidity = 60;
    data.status = "Clear sky";
    data.valid = true;
    return data;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch weather.");
        return data;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://api.open-meteo.com/v1/forecast?latitude=";
    url += _latitude;
    url += "&longitude=";
    url += _longitude;
    url += "&current=temperature_2m,relative_humidity_2m,weather_code";
#if UNIT_SYSTEM == UNIT_IMPERIAL
    url += "&temperature_unit=fahrenheit";
#endif

    Serial.print("[Weather] Fetching URL: ");
    Serial.println(url);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            StaticJsonDocument<1024> doc;
            DeserializationError error = deserializeJson(doc, payload);

            if (!error) {
                data.temperature = doc["current"]["temperature_2m"].as<float>();
                data.humidity = doc["current"]["relative_humidity_2m"].as<int>();
                int code = doc["current"]["weather_code"].is<int>() ? doc["current"]["weather_code"].as<int>() : -1;
                data.status = getWeatherDesc(code);
                data.valid = true;
                Serial.printf("[Weather] Success! Temp: %.1f C, Hum: %d %%, Status: %s\n",
                    data.temperature, data.humidity, data.status.c_str());
            } else {
                Serial.print("[Weather] JSON Deserialization failed: ");
                Serial.println(error.c_str());
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
