#include <unity.h>
#include "weather_client.h"
#include "../mocks/mocks.cpp"
#include "../../src/weather_client.cpp"

void setUp(void) {
    // set up before tests
}

void tearDown(void) {
    // clean up after tests
}

void test_weather_client_initialization(void) {
    WeatherClient client("37.7749", "-122.4194");
    // Assert it builds and sets up without crashing
    TEST_ASSERT_TRUE(true);
}

void test_weather_client_get_weather_desc(void) {
    TEST_ASSERT_EQUAL_STRING("Clear sky", WeatherClient::getWeatherDesc(0).c_str());
    TEST_ASSERT_EQUAL_STRING("Mainly clear", WeatherClient::getWeatherDesc(1).c_str());
    TEST_ASSERT_EQUAL_STRING("Partly cloudy", WeatherClient::getWeatherDesc(2).c_str());
    TEST_ASSERT_EQUAL_STRING("Overcast", WeatherClient::getWeatherDesc(3).c_str());
    TEST_ASSERT_EQUAL_STRING("Foggy", WeatherClient::getWeatherDesc(45).c_str());
    TEST_ASSERT_EQUAL_STRING("Unknown", WeatherClient::getWeatherDesc(999).c_str());
}

void test_weather_client_fetch_weather_native_mock(void) {
    WeatherClient client("37.7749", "-122.4194");
    WeatherData data = client.fetchWeather();
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(21.5f, data.temperature);
    TEST_ASSERT_EQUAL(60, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
    TEST_ASSERT_EQUAL(0, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(8.8f, data.windSpeed);
}

void test_weather_client_zip_code_initialization(void) {
    WeatherClient client("90210");
    WeatherData data = client.fetchWeather();
    
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(21.5f, data.temperature);
    TEST_ASSERT_EQUAL(60, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
    TEST_ASSERT_EQUAL(0, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(8.8f, data.windSpeed);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_client_initialization);
    RUN_TEST(test_weather_client_get_weather_desc);
    RUN_TEST(test_weather_client_fetch_weather_native_mock);
    RUN_TEST(test_weather_client_zip_code_initialization);
    return UNITY_END();
}
