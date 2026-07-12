#include <unity.h>
#include "weather_client.h"
#include "../mocks/mocks.cpp"
#include "../../src/weather_client.cpp"

void setUp(void) {
    // set up
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

    String json = WeatherClient::serializeWeatherData(original);
    
    // This will fail initially
    TEST_ASSERT_TRUE(json.length() > 0);

    WeatherData recovered;
    bool success = WeatherClient::deserializeWeatherData(json, recovered);
    
    // This will fail initially
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
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_cache_serialization_deserialization);
    return UNITY_END();
}
