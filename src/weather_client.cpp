#include "weather_client.h"
#include <ArduinoJson.h>
#include "config/config.h"
#include "settings_manager.h"
#include <time.h>

#ifndef OPENWEATHERMAP_API_KEY
#define OPENWEATHERMAP_API_KEY ""
#endif

extern SettingsManager settings;

#ifndef NATIVE_TEST
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClient.h>
#endif

WeatherClient::WeatherClient(const char* latitude, const char* longitude)
    : _latitude(latitude), _longitude(longitude), _zipCode(nullptr), _useZip(false), _zipResolved(false), _cityName("") {}

WeatherClient::WeatherClient(const char* zipCode)
    : _latitude(nullptr), _longitude(nullptr), _zipCode(zipCode), _useZip(true), _zipResolved(false), _cityName("") {}

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
                _cityName    = String(name);
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

bool WeatherClient::reverseGeocode() {
#ifdef NATIVE_TEST
    _cityName = "Mock City";
    return true;
#else
    if (WiFi.status() != WL_CONNECTED) return false;

    WiFiClient client;
    HTTPClient http;

    // Nominatim reverse geocoding — free, no API key required.
    // Policy: include a descriptive User-Agent (required by Nominatim ToS).
    String url = "http://nominatim.openstreetmap.org/reverse?format=json&lat=";
    url += _latitude;
    url += "&lon=";
    url += _longitude;
    url += "&zoom=10&addressdetails=1";

    Serial.print("[Weather] Reverse geocoding coordinates: ");
    Serial.print(_latitude); Serial.print(", "); Serial.println(_longitude);

    if (http.begin(client, url)) {
        http.addHeader("User-Agent", "CYD-Weather-Station/1.0");
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            StaticJsonDocument<2048> doc;
            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
                // Prefer city > town > village > county in that order
                const char* city = nullptr;
                if (!city && doc["address"]["city"].is<const char*>())
                    city = doc["address"]["city"];
                if (!city && doc["address"]["town"].is<const char*>())
                    city = doc["address"]["town"];
                if (!city && doc["address"]["village"].is<const char*>())
                    city = doc["address"]["village"];
                if (!city && doc["address"]["county"].is<const char*>())
                    city = doc["address"]["county"];
                if (city) {
                    _cityName = String(city);
                    Serial.printf("[Weather] Reverse geocode: %s\n", _cityName.c_str());
                    http.end();
                    return true;
                }
            } else {
                Serial.print("[Weather] Reverse geocode JSON error: ");
                Serial.println(error.c_str());
            }
        } else {
            Serial.printf("[Weather] Reverse geocode HTTP failed: %d\n", httpCode);
        }
        http.end();
    }
    return false;
#endif
}

WeatherData WeatherClient::fetchWeather() {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };

    bool useOWM = (String(OPENWEATHERMAP_API_KEY).length() > 0);

    // Resolve zip code if using zip code and not yet resolved
    if (_useZip && !_zipResolved) {
        if (!geocodeZip()) {
            Serial.println("[Weather] Failed to resolve zip code. Cannot fetch weather.");
            return data;
        }
    }

    // Resolve city name from coordinates (once) if not using zip and not using OWM (OWM gets it directly)
    if (!_useZip && _cityName.length() == 0 && !useOWM) {
        reverseGeocode();
    }

    const char* lat = _useZip ? _resolvedLat.c_str() : ((_latitude && strlen(_latitude) > 0) ? _latitude : _resolvedLat.c_str());
    const char* lng = _useZip ? _resolvedLng.c_str() : ((_longitude && strlen(_longitude) > 0) ? _longitude : _resolvedLng.c_str());

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
    _cityName = "Mock City";
    parseWeatherJson(mockJson, data);
    data.cityName = _cityName;
    return data;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch weather.");
        return data;
    }

    WiFiClient client;
    HTTPClient http;

    String url;
    if (useOWM) {
        url = "http://api.openweathermap.org/data/2.5/forecast?lat=";
        url += lat;
        url += "&lon=";
        url += lng;
        url += "&appid=";
        url += OPENWEATHERMAP_API_KEY;
        url += "&units=";
        url += (settings.getUnitSystem() == UNIT_IMPERIAL ? "imperial" : "metric");
    } else {
        url = "http://api.open-meteo.com/v1/forecast?latitude=";
        url += lat;
        url += "&longitude=";
        url += lng;
        url += "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m,wind_direction_10m";
        url += "&daily=weather_code,temperature_2m_max,temperature_2m_min&forecast_days=3";
        url += "&timezone=auto"; // Return dates in local timezone, not UTC
        if (settings.getUnitSystem() == UNIT_IMPERIAL) {
            url += "&temperature_unit=fahrenheit";
            url += "&windspeed_unit=mph";
        }
    }

    Serial.print("[Weather] Fetching URL: ");
    Serial.println(url);

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            bool parseSuccess = false;
            if (useOWM) {
                parseSuccess = parseOwmJson(payload.c_str(), data);
            } else {
                parseSuccess = parseWeatherJson(payload.c_str(), data);
            }
            if (parseSuccess) {
                data.cityName = _cityName;
                Serial.printf("[Weather] Success! Temp: %.1f, Hum: %d %%, Status: %s, Wind: %.1f, City: %s\n",
                    data.temperature, data.humidity, data.status.c_str(), data.windSpeed, data.cityName.c_str());
            }
        } else {
            Serial.printf("[Weather] HTTP GET failed, error code: %d\n", httpCode);
        }
        http.end();
    } else {
        Serial.println("[Weather] Unable to connect to Weather API");
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
        data.windDirection = doc["current"]["wind_direction_10m"].is<int>() ? doc["current"]["wind_direction_10m"].as<int>() : 0;
        data.valid = true;
        data.weatherCode = code;
    }

    // Parse daily forecast
    if (doc.containsKey("daily")) {
        JsonArray daily_time = doc["daily"]["time"];
        JsonArray daily_max = doc["daily"]["temperature_2m_max"];
        JsonArray daily_min = doc["daily"]["temperature_2m_min"];
        JsonArray daily_code = doc["daily"]["weather_code"];

        // Compute today's and tomorrow's date in the user's configured timezone
        // so that "Today" / "Tomorrow" labels are correct regardless of UTC rollover.
        // Falls back to index-based labelling if NTP hasn't synced yet.
        char today_str[11]    = "";
        char tomorrow_str[11] = "";
#ifndef NATIVE_TEST
        time_t now = time(nullptr);
        const bool time_valid = (now > 946684800L); // after year 2000 = NTP synced
        if (time_valid) {
            // Apply the configured offset so we compare against local midnight,
            // not UTC midnight.
            long tz_sec = (long)settings.getTimezoneOffset() * 3600L;
            time_t local_now = now + tz_sec;
            struct tm tm_today, tm_tomorrow;
            gmtime_r(&local_now,          &tm_today);
            gmtime_r(&(local_now += 86400), &tm_tomorrow);
            snprintf(today_str,    sizeof(today_str),    "%04d-%02d-%02d",
                tm_today.tm_year + 1900,    tm_today.tm_mon + 1,    tm_today.tm_mday);
            snprintf(tomorrow_str, sizeof(tomorrow_str), "%04d-%02d-%02d",
                tm_tomorrow.tm_year + 1900, tm_tomorrow.tm_mon + 1, tm_tomorrow.tm_mday);
        }
#else
        const bool time_valid = false; // native test: fall back to index-based
#endif

        for (int i = 0; i < 3; i++) {
            if (i < (int)daily_time.size()) {
                data.forecast[i].tempMax = daily_max[i].as<float>();
                data.forecast[i].tempMin = daily_min[i].as<float>();
                int code = daily_code[i].as<int>();
                data.forecast[i].weatherCode = code;
                data.forecast[i].status = getWeatherDesc(code);

                String dateStr = daily_time[i].as<String>();

                // Determine the human-readable label for this date.
                // Priority: timezone-aware match > index-based fallback > day name.
                if (time_valid && dateStr == today_str) {
                    data.forecast[i].dayName = "Today";
                } else if (time_valid && dateStr == tomorrow_str) {
                    data.forecast[i].dayName = "Tmrw";
                } else if (!time_valid && i == 0) {
                    // NTP not yet synced: assume API day 0 is today
                    data.forecast[i].dayName = "Today";
                } else if (!time_valid && i == 1) {
                    data.forecast[i].dayName = "Tmrw";
                } else if (dateStr.length() >= 10) {
                    // Compute short day name from the date string
                    const char* dateCStr = dateStr.c_str();
                    int y = (dateCStr[0]-'0')*1000 + (dateCStr[1]-'0')*100
                          + (dateCStr[2]-'0')*10  + (dateCStr[3]-'0');
                    int m = (dateCStr[5]-'0')*10 + (dateCStr[6]-'0');
                    int d = (dateCStr[8]-'0')*10 + (dateCStr[9]-'0');

                    // Tomohiko Sakamoto's algorithm (0=Sun … 6=Sat)
                    static const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
                    static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
                    if (m < 3) y -= 1;
                    int dayIdx = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
                    data.forecast[i].dayName = (dayIdx >= 0 && dayIdx < 7)
                                               ? days[dayIdx] : dateStr.c_str();
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

bool WeatherClient::parseOwmJson(const char* json, WeatherData& data) {
    StaticJsonDocument<1024> filter;
    filter["list"][0]["dt_txt"] = true;
    filter["list"][0]["main"]["temp"] = true;
    filter["list"][0]["main"]["humidity"] = true;
    filter["list"][0]["main"]["temp_min"] = true;
    filter["list"][0]["main"]["temp_max"] = true;
    filter["list"][0]["wind"]["speed"] = true;
    filter["list"][0]["wind"]["deg"] = true;
    filter["list"][0]["weather"][0]["id"] = true;
    filter["list"][0]["weather"][0]["description"] = true;
    filter["city"]["name"] = true;

    DynamicJsonDocument doc(12288);
    DeserializationError error = deserializeJson(doc, json, DeserializationOption::Filter(filter));

    if (error) {
        Serial.print("[Weather] OWM JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    if (!doc.containsKey("list") || doc["list"].size() == 0) {
        Serial.println("[Weather] Invalid OWM JSON payload");
        return false;
    }

    JsonArray list = doc["list"];

    // 1. Parse current weather from list[0]
    JsonObject current = list[0];
    data.temperature = current["main"]["temp"].as<float>();
    data.humidity = current["main"]["humidity"].as<int>();
    data.windSpeed = current["wind"]["speed"].as<float>();
    data.windDirection = current["wind"]["deg"].is<int>() ? current["wind"]["deg"].as<int>() : 0;
    // Convert metric wind speed from m/s to km/h (if metric)
    if (settings.getUnitSystem() == UNIT_METRIC) {
        data.windSpeed *= 3.6f;
    }
    
    int owmCode = current["weather"][0]["id"].as<int>();
    data.weatherCode = owmToWmoCode(owmCode);
    data.status = current["weather"][0]["description"].as<String>();
    // Capitalize first letter of status description for better aesthetics
    if (data.status.length() > 0) {
        data.status[0] = toupper(data.status[0]);
    }
    data.valid = true;

    // Resolve city name from OWM city object
    if (_cityName.length() == 0 && doc.containsKey("city")) {
        const char* name = doc["city"]["name"] | "";
        if (name && name[0] != '\0') {
            _cityName = String(name);
        }
    }
    data.cityName = _cityName;

    // 2. Parse 3-day forecast (Today, Tomorrow, Day after)
    // We group OWM's 3-hourly forecasts by local date.
    // Each date will have max/min temps calculated, and we grab weather condition closest to midday (12:00:00).
    
    struct DayForecastTemp {
        float tempMin = 999.0f;
        float tempMax = -999.0f;
        int weatherCode = -1;
        String status = "";
        bool hasMidday = false;
    };

    String uniqueDays[5];
    DayForecastTemp dayTemps[5];
    int dayCount = 0;

    for (JsonObject entry : list) {
        String dt_txt = entry["dt_txt"].as<String>();
        if (dt_txt.length() < 10) continue;
        char dateBuf[11];
        strncpy(dateBuf, dt_txt.c_str(), 10);
        dateBuf[10] = '\0';
        String dateStr(dateBuf);
        
        // Find if this date is already tracked
        int idx = -1;
        for (int i = 0; i < dayCount; i++) {
            if (uniqueDays[i] == dateStr) {
                idx = i;
                break;
            }
        }
        
        if (idx == -1) {
            if (dayCount >= 5) continue; // Only track up to 5 days
            idx = dayCount;
            uniqueDays[idx] = dateStr;
            dayCount++;
        }

        // Update min/max temperatures
        float tMin = entry["main"]["temp_min"].as<float>();
        float tMax = entry["main"]["temp_max"].as<float>();
        if (tMin < dayTemps[idx].tempMin) dayTemps[idx].tempMin = tMin;
        if (tMax > dayTemps[idx].tempMax) dayTemps[idx].tempMax = tMax;

        // Pick weather condition. Prefer midday (12:00:00).
        // If not midday, and we don't have midday set yet, use the first/any entry.
        bool isMidday = strstr(dt_txt.c_str(), "12:00:00") != nullptr;
        if (isMidday || (!dayTemps[idx].hasMidday && dayTemps[idx].weatherCode == -1)) {
            int code = entry["weather"][0]["id"].as<int>();
            dayTemps[idx].weatherCode = owmToWmoCode(code);
            dayTemps[idx].status = entry["weather"][0]["description"].as<String>();
            if (dayTemps[idx].status.length() > 0) {
                dayTemps[idx].status[0] = toupper(dayTemps[idx].status[0]);
            }
            if (isMidday) {
                dayTemps[idx].hasMidday = true;
            }
        }
    }

    // Now populate the 3 forecast days.
    char today_str[11] = "";
    char tomorrow_str[11] = "";
    bool time_valid = false;
#ifndef NATIVE_TEST
    time_t now = time(nullptr);
    time_valid = (now > 946684800L); // after year 2000
    if (time_valid) {
        long tz_sec = (long)settings.getTimezoneOffset() * 3600L;
        time_t local_now = now + tz_sec;
        struct tm tm_today, tm_tomorrow;
        gmtime_r(&local_now, &tm_today);
        gmtime_r(&(local_now += 86400), &tm_tomorrow);
        snprintf(today_str, sizeof(today_str), "%04d-%02d-%02d",
            tm_today.tm_year + 1900, tm_today.tm_mon + 1, tm_today.tm_mday);
        snprintf(tomorrow_str, sizeof(tomorrow_str), "%04d-%02d-%02d",
            tm_tomorrow.tm_year + 1900, tm_tomorrow.tm_mon + 1, tm_tomorrow.tm_mday);
    }
#endif

    // Map uniqueDays to forecast slots
    for (int i = 0; i < 3; i++) {
        if (i < dayCount) {
            data.forecast[i].tempMax = dayTemps[i].tempMax;
            data.forecast[i].tempMin = dayTemps[i].tempMin;
            data.forecast[i].weatherCode = dayTemps[i].weatherCode;
            data.forecast[i].status = dayTemps[i].status;

            String dateStr = uniqueDays[i];
            if (time_valid && dateStr == today_str) {
                data.forecast[i].dayName = "Today";
            } else if (time_valid && dateStr == tomorrow_str) {
                data.forecast[i].dayName = "Tmrw";
            } else if (!time_valid && i == 0) {
                data.forecast[i].dayName = "Today";
            } else if (!time_valid && i == 1) {
                data.forecast[i].dayName = "Tmrw";
            } else {
                // Compute short day name using Sakamoto
                int y = (dateStr[0]-'0')*1000 + (dateStr[1]-'0')*100
                      + (dateStr[2]-'0')*10  + (dateStr[3]-'0');
                int m = (dateStr[5]-'0')*10 + (dateStr[6]-'0');
                int d = (dateStr[8]-'0')*10 + (dateStr[9]-'0');

                static const char* days[] = {"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
                static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
                if (m < 3) y -= 1;
                int dayIdx = (y + y/4 - y/100 + y/400 + t[m-1] + d) % 7;
                data.forecast[i].dayName = (dayIdx >= 0 && dayIdx < 7) ? days[dayIdx] : dateStr;
            }
        } else {
            data.forecast[i].tempMax = 0.0f;
            data.forecast[i].tempMin = 0.0f;
            data.forecast[i].weatherCode = -1;
            data.forecast[i].status = "Unknown";
            data.forecast[i].dayName = "N/A";
        }
    }

    return true;
}

int WeatherClient::owmToWmoCode(int owmCode) {
    if (owmCode >= 200 && owmCode < 300) return 95; // Thunderstorm
    if (owmCode >= 300 && owmCode < 400) return 51; // Drizzle
    if (owmCode >= 500 && owmCode < 600) {
        if (owmCode == 511) return 66; // Freezing rain
        return 61; // Rain
    }
    if (owmCode >= 600 && owmCode < 700) return 71; // Snow
    if (owmCode >= 700 && owmCode < 800) return 45; // Fog
    if (owmCode == 800) return 0; // Clear
    if (owmCode == 801) return 1; // Mainly clear
    if (owmCode == 802) return 2; // Partly cloudy
    if (owmCode >= 803 && owmCode < 900) return 3; // Overcast
    return -1;
}

String WeatherClient::serializeWeatherData(const WeatherData& data) {
    StaticJsonDocument<1024> doc;
    doc["temp"] = data.temperature;
    doc["hum"] = data.humidity;
    doc["status"] = data.status;
    doc["valid"] = data.valid;
    doc["code"] = data.weatherCode;
    doc["wind_spd"] = data.windSpeed;
    doc["wind_dir"] = data.windDirection;
    doc["city"] = data.cityName;

    JsonArray forecast = doc.createNestedArray("forecast");
    for (int i = 0; i < 3; i++) {
        JsonObject fDay = forecast.createNestedObject();
        fDay["temp_max"] = data.forecast[i].tempMax;
        fDay["temp_min"] = data.forecast[i].tempMin;
        fDay["code"] = data.forecast[i].weatherCode;
        fDay["status"] = data.forecast[i].status;
        fDay["day"] = data.forecast[i].dayName;
    }

    String output;
    serializeJson(doc, output);
    return output;
}

bool WeatherClient::deserializeWeatherData(const String& json, WeatherData& data) {
    if (json.length() == 0) return false;
    StaticJsonDocument<1024> doc;
    DeserializationError error = deserializeJson(doc, json);
    if (error) {
        return false;
    }

    data.temperature = doc["temp"] | 0.0f;
    data.humidity = doc["hum"] | 0;
    data.status = doc["status"].as<String>();
    data.valid = doc["valid"] | false;
    data.weatherCode = doc["code"] | 0;
    data.windSpeed = doc["wind_spd"] | 0.0f;
    data.windDirection = doc["wind_dir"] | 0;
    data.cityName = doc["city"].as<String>();

    JsonArray forecast = doc["forecast"];
    if (!forecast.isNull()) {
        for (int i = 0; i < 3 && i < (int)forecast.size(); i++) {
            JsonObject fDay = forecast[i];
            data.forecast[i].tempMax = fDay["temp_max"] | 0.0f;
            data.forecast[i].tempMin = fDay["temp_min"] | 0.0f;
            data.forecast[i].weatherCode = fDay["code"] | 0;
            data.forecast[i].status = fDay["status"].as<String>();
            data.forecast[i].dayName = fDay["day"].as<String>();
        }
    }
    return true;
}

bool WeatherClient::isLocationEmpty() const {
    if (_useZip) {
        return _zipCode == nullptr || strlen(_zipCode) == 0 || strcmp(_zipCode, "YOUR_ZIP_CODE") == 0;
    } else {
        return _latitude == nullptr || strlen(_latitude) == 0 || 
               _longitude == nullptr || strlen(_longitude) == 0 ||
               strcmp(_latitude, "YOUR_LATITUDE") == 0;
    }
}

bool WeatherClient::fetchIpLocation(String& latStr, String& lonStr, String& city) {
#ifdef NATIVE_TEST
    latStr = "37.7749";
    lonStr = "-122.4194";
    city = "San Francisco";
    _resolvedLat = latStr;
    _resolvedLng = lonStr;
    _cityName = city;
    _useZip = false;
    return true;
#else
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("[Weather] WiFi not connected. Cannot fetch IP location.");
        return false;
    }

    WiFiClient client;
    HTTPClient http;

    String url = "http://ip-api.com/json/";
    Serial.println("[Weather] Querying IP Geolocation fallback...");

    if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            float lat = 0.0f;
            float lon = 0.0f;
            if (parseIpLocationJson(payload.c_str(), lat, lon, city)) {
                latStr = String(lat, 5);
                lonStr = String(lon, 5);
                _resolvedLat = latStr;
                _resolvedLng = lonStr;
                _cityName = city;
                _useZip = false;
                http.end();
                return true;
            }
        } else {
            Serial.printf("[Weather] Geolocation HTTP GET failed: %d\n", httpCode);
        }
        http.end();
    }
    return false;
#endif
}

bool WeatherClient::parseIpLocationJson(const char* json, float& lat, float& lon, String& city) {
    StaticJsonDocument<512> doc;
    DeserializationError error = deserializeJson(doc, json);

    if (error) {
        Serial.print("[Weather] Geolocation JSON Deserialization failed: ");
        Serial.println(error.c_str());
        return false;
    }

    const char* status = doc["status"] | "fail";
    if (strcmp(status, "success") != 0) {
        Serial.print("[Weather] Geolocation failed status: ");
        Serial.println(status);
        return false;
    }

    lat = doc["lat"] | 0.0f;
    lon = doc["lon"] | 0.0f;
    const char* city_c = doc["city"] | "Unknown";
    city = String(city_c);

    return true;
}

