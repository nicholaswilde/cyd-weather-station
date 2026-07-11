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

    // Verify 3-day forecast data from the mock payload
    TEST_ASSERT_EQUAL_FLOAT(25.5f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(15.0f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.forecast[0].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(26.0f, data.forecast[1].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(14.8f, data.forecast[1].tempMin);
    TEST_ASSERT_EQUAL_STRING("Tomorrow", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Mainly clear", data.forecast[1].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(24.8f, data.forecast[2].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(15.2f, data.forecast[2].tempMin);
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Partly cloudy", data.forecast[2].status.c_str());
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

void test_weather_client_parse_weather_json_custom(void) {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };
    const char* customJson = "{"
        "\"current\":{"
            "\"temperature_2m\":15.5,"
            "\"relative_humidity_2m\":80,"
            "\"weather_code\":3,"
            "\"wind_speed_10m\":12.4"
        "},"
        "\"daily\":{"
            "\"time\":[\"2026-07-09\",\"2026-07-10\",\"2026-07-11\"],"
            "\"weather_code\":[3,45,95],"
            "\"temperature_2m_max\":[18.5,16.0,22.0],"
            "\"temperature_2m_min\":[12.0,11.0,14.5]"
        "}"
    "}";
    
    bool parsed = WeatherClient::parseWeatherJson(customJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(15.5f, data.temperature);
    TEST_ASSERT_EQUAL(80, data.humidity);
    TEST_ASSERT_EQUAL_STRING("Overcast", data.status.c_str());
    TEST_ASSERT_EQUAL(3, data.weatherCode);
    TEST_ASSERT_EQUAL_FLOAT(12.4f, data.windSpeed);

    TEST_ASSERT_EQUAL_FLOAT(18.5f, data.forecast[0].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(12.0f, data.forecast[0].tempMin);
    TEST_ASSERT_EQUAL_STRING("Today", data.forecast[0].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Overcast", data.forecast[0].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(16.0f, data.forecast[1].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(11.0f, data.forecast[1].tempMin);
    TEST_ASSERT_EQUAL_STRING("Tomorrow", data.forecast[1].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Foggy", data.forecast[1].status.c_str());

    TEST_ASSERT_EQUAL_FLOAT(22.0f, data.forecast[2].tempMax);
    TEST_ASSERT_EQUAL_FLOAT(14.5f, data.forecast[2].tempMin);
    TEST_ASSERT_EQUAL_STRING("Sat", data.forecast[2].dayName.c_str());
    TEST_ASSERT_EQUAL_STRING("Thunderstorm", data.forecast[2].status.c_str());
}

void test_weather_client_parse_owm_json(void) {
    WeatherClient client("33.60002", "-117.67200");
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {} };

    const char* owmJson = "{"
        "\"list\":["
            "{"
                "\"dt_txt\":\"2026-07-11 12:00:00\","
                "\"main\":{\"temp\":72.5,\"humidity\":50,\"temp_min\":68.0,\"temp_max\":75.0},"
                "\"wind\":{\"speed\":5.5,\"deg\":180},"
                "\"weather\":[{\"id\":800,\"description\":\"clear sky\"}]"
            "}"
        "],"
        "\"city\":{\"name\":\"Orange County\"}"
    "}";

    bool parsed = client.parseOwmJson(owmJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    TEST_ASSERT_EQUAL_FLOAT(72.5f, data.temperature);
    TEST_ASSERT_EQUAL(50, data.humidity);
    TEST_ASSERT_EQUAL_FLOAT(5.5f, data.windSpeed);
    TEST_ASSERT_EQUAL(180, data.windDirection);
    TEST_ASSERT_EQUAL_STRING("Orange County", data.cityName.c_str());
    TEST_ASSERT_EQUAL_STRING("Clear sky", data.status.c_str());
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_client_initialization);
    RUN_TEST(test_weather_client_get_weather_desc);
    RUN_TEST(test_weather_client_fetch_weather_native_mock);
    RUN_TEST(test_weather_client_zip_code_initialization);
    RUN_TEST(test_weather_client_parse_weather_json_custom);
    RUN_TEST(test_weather_client_parse_owm_json);
    return UNITY_END();
}
