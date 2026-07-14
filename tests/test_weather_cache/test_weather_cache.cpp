#include <unity.h>
#include "weather_client.h"
#include "weather_cache.h"
#include "../mocks/mocks.cpp"
#include "../../src/weather_client.cpp"
#include "../../src/sd_card_manager.cpp"
#include "../../src/weather_cache.cpp"

void setUp(void) {
    mock_sd_card_present = true;
    mock_sd_card_mounted = false;
    mock_files.clear();
}

void tearDown(void) {
    // clean up
}

void test_weather_cache_serialization_deserialization(void) {
    WeatherData original;
    original.temperature = 22.5f;
    original.humidity = 55;
    original.status = "Partly Cloudy";
    original.valid = true;
    original.weatherCode = 2;
    original.windSpeed = 12.5f;
    original.windDirection = 180;
    original.cityName = "San Francisco";
    
    original.forecast[0] = {25.0f, 15.0f, 2, "Partly Cloudy", "Today"};
    original.forecast[1] = {27.0f, 16.0f, 3, "Overcast", "Tomorrow"};
    original.forecast[2] = {22.0f, 14.0f, 0, "Clear", "Tuesday"};

    for (int i = 0; i < 24; i++) {
        original.hourly[i] = {(float)(10.0f + i), i * 4};
    }

    String json = WeatherClient::serializeWeatherData(original);
    
    TEST_ASSERT_TRUE(json.length() > 0);

    WeatherData recovered;
    bool success = WeatherClient::deserializeWeatherData(json, recovered);
    
    TEST_ASSERT_TRUE(success);
    
    TEST_ASSERT_EQUAL_FLOAT(original.temperature, recovered.temperature);
    TEST_ASSERT_EQUAL(original.humidity, recovered.humidity);
    TEST_ASSERT_EQUAL_STRING(original.status.c_str(), recovered.status.c_str());
    TEST_ASSERT_EQUAL(original.valid, recovered.valid);
    TEST_ASSERT_EQUAL(original.weatherCode, recovered.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(original.windSpeed, recovered.windSpeed);
    TEST_ASSERT_EQUAL(original.windDirection, recovered.windDirection);
    TEST_ASSERT_EQUAL_STRING(original.cityName.c_str(), recovered.cityName.c_str());
    
    for (int i = 0; i < 3; i++) {
        TEST_ASSERT_EQUAL_FLOAT(original.forecast[i].tempMax, recovered.forecast[i].tempMax);
        TEST_ASSERT_EQUAL_FLOAT(original.forecast[i].tempMin, recovered.forecast[i].tempMin);
        TEST_ASSERT_EQUAL(original.forecast[i].weatherCode, recovered.forecast[i].weatherCode);
        TEST_ASSERT_EQUAL_STRING(original.forecast[i].status.c_str(), recovered.forecast[i].status.c_str());
        TEST_ASSERT_EQUAL_STRING(original.forecast[i].dayName.c_str(), recovered.forecast[i].dayName.c_str());
    }

    for (int i = 0; i < 24; i++) {
        TEST_ASSERT_EQUAL_FLOAT(original.hourly[i].temperature, recovered.hourly[i].temperature);
        TEST_ASSERT_EQUAL(original.hourly[i].precipitationProbability, recovered.hourly[i].precipitationProbability);
    }
}

void test_weather_cache_sd_save_and_load(void) {
    WeatherData original;
    original.temperature = 18.0f;
    original.humidity = 80;
    original.status = "Rain";
    original.valid = true;
    original.weatherCode = 61;
    original.windSpeed = 5.0f;
    original.windDirection = 90;
    original.cityName = "Seattle";
    
    original.forecast[0] = {20.0f, 12.0f, 61, "Rain", "Today"};
    original.forecast[1] = {21.0f, 13.0f, 2, "Partly Cloudy", "Tomorrow"};
    original.forecast[2] = {19.0f, 11.0f, 3, "Overcast", "Wednesday"};

    for (int i = 0; i < 24; i++) {
        original.hourly[i] = {(float)(10.0f + i), i * 4};
    }

    // Save should succeed
    bool save_ok = WeatherCache::saveCache(original);
    TEST_ASSERT_TRUE(save_ok);

    // Verify cache file exists in mock files map
    TEST_ASSERT_TRUE(mock_files.find("/weather_cache.json") != mock_files.end());

    // Load should succeed and match original
    WeatherData recovered;
    bool load_ok = WeatherCache::loadCache(recovered);
    TEST_ASSERT_TRUE(load_ok);

    TEST_ASSERT_EQUAL_FLOAT(original.temperature, recovered.temperature);
    TEST_ASSERT_EQUAL_STRING(original.cityName.c_str(), recovered.cityName.c_str());
    TEST_ASSERT_EQUAL_FLOAT(original.hourly[0].temperature, recovered.hourly[0].temperature);
    TEST_ASSERT_EQUAL(original.hourly[0].precipitationProbability, recovered.hourly[0].precipitationProbability);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_cache_serialization_deserialization);
    RUN_TEST(test_weather_cache_sd_save_and_load);
    return UNITY_END();
}
