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
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
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
    TEST_ASSERT_EQUAL_STRING("Tmrw", data.forecast[1].dayName.c_str());
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

void test_weather_client_parse_ip_location_json(void) {
    const char* ipLocationJson = "{"
        "\"status\":\"success\","
        "\"city\":\"San Francisco\","
        "\"lat\":37.7749,"
        "\"lon\":-122.4194"
    "}";
    float lat = 0.0f;
    float lon = 0.0f;
    String city = "";
    bool parsed = WeatherClient::parseIpLocationJson(ipLocationJson, lat, lon, city);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_EQUAL_FLOAT(37.7749f, lat);
    TEST_ASSERT_EQUAL_FLOAT(-122.4194f, lon);
    TEST_ASSERT_EQUAL_STRING("San Francisco", city.c_str());
}

void test_weather_client_parse_ip_location_json_fail(void) {
    const char* ipLocationJson = "{"
        "\"status\":\"fail\","
        "\"message\":\"invalid query\""
    "}";
    float lat = 0.0f;
    float lon = 0.0f;
    String city = "";
    bool parsed = WeatherClient::parseIpLocationJson(ipLocationJson, lat, lon, city);
    TEST_ASSERT_FALSE(parsed);
}

void test_weather_client_is_location_empty(void) {
    WeatherClient client1("");
    TEST_ASSERT_TRUE(client1.isLocationEmpty());

    WeatherClient client2("YOUR_ZIP_CODE");
    TEST_ASSERT_TRUE(client2.isLocationEmpty());

    WeatherClient client3("90210");
    TEST_ASSERT_FALSE(client3.isLocationEmpty());

    WeatherClient client4("", "");
    TEST_ASSERT_TRUE(client4.isLocationEmpty());

    WeatherClient client5("YOUR_LATITUDE", "-122.4194");
    TEST_ASSERT_TRUE(client5.isLocationEmpty());

    WeatherClient client6("37.7749", "-122.4194");
    TEST_ASSERT_FALSE(client6.isLocationEmpty());
}

void test_weather_client_parse_weather_json_hourly(void) {
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {}, {} };
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
        "},"
        "\"hourly\":{"
            "\"temperature_2m\":[10.0,11.0,12.0,13.0,14.0,15.0,16.0,17.0,18.0,19.0,20.0,21.0,22.0,23.0,24.0,25.0,26.0,27.0,28.0,29.0,30.0,31.0,32.0,33.0],"
            "\"precipitation_probability\":[0,5,10,15,20,25,30,35,40,45,50,55,60,65,70,75,80,85,90,95,100,5,10,15]"
        "}"
    "}";
    
    bool parsed = WeatherClient::parseWeatherJson(customJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    
    // Check hourly data
    TEST_ASSERT_EQUAL_FLOAT(10.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(0, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(21.0f, data.hourly[11].temperature);
    TEST_ASSERT_EQUAL(55, data.hourly[11].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(33.0f, data.hourly[23].temperature);
    TEST_ASSERT_EQUAL(15, data.hourly[23].precipitationProbability);
}

void test_weather_client_parse_owm_json_hourly(void) {
    WeatherClient client("33.60002", "-117.67200");
    WeatherData data = { 0.0f, 0, "Unknown", false, -1, 0.0f, 0, "", {}, {} };

    // OWM payload with at least 8 elements (24 hours at 3-hour intervals)
    const char* owmJson = "{"
        "\"list\":["
            "{\"dt_txt\":\"2026-07-11 12:00:00\",\"main\":{\"temp\":70.0,\"humidity\":50},\"wind\":{\"speed\":5.0,\"deg\":180},\"weather\":[{\"id\":800,\"description\":\"clear sky\"}],\"pop\":0.10},"
            "{\"dt_txt\":\"2026-07-11 15:00:00\",\"main\":{\"temp\":71.0},\"weather\":[{\"id\":800}],\"pop\":0.20},"
            "{\"dt_txt\":\"2026-07-11 18:00:00\",\"main\":{\"temp\":72.0},\"weather\":[{\"id\":800}],\"pop\":0.30},"
            "{\"dt_txt\":\"2026-07-11 21:00:00\",\"main\":{\"temp\":73.0},\"weather\":[{\"id\":800}],\"pop\":0.40},"
            "{\"dt_txt\":\"2026-07-12 00:00:00\",\"main\":{\"temp\":74.0},\"weather\":[{\"id\":800}],\"pop\":0.50},"
            "{\"dt_txt\":\"2026-07-12 03:00:00\",\"main\":{\"temp\":75.0},\"weather\":[{\"id\":800}],\"pop\":0.60},"
            "{\"dt_txt\":\"2026-07-12 06:00:00\",\"main\":{\"temp\":76.0},\"weather\":[{\"id\":800}],\"pop\":0.70},"
            "{\"dt_txt\":\"2026-07-12 09:00:00\",\"main\":{\"temp\":77.0},\"weather\":[{\"id\":800}],\"pop\":0.80}"
        "],"
        "\"city\":{\"name\":\"Orange County\"}"
    "}";

    bool parsed = client.parseOwmJson(owmJson, data);
    TEST_ASSERT_TRUE(parsed);
    TEST_ASSERT_TRUE(data.valid);
    
    // Check hourly mapped from the 3-hour list
    TEST_ASSERT_EQUAL_FLOAT(70.0f, data.hourly[0].temperature);
    TEST_ASSERT_EQUAL(10, data.hourly[0].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, data.hourly[1].temperature); // Mapped
    TEST_ASSERT_EQUAL_FLOAT(71.0f, data.hourly[3].temperature);
    TEST_ASSERT_EQUAL(20, data.hourly[3].precipitationProbability);
    TEST_ASSERT_EQUAL_FLOAT(77.0f, data.hourly[23].temperature);
    TEST_ASSERT_EQUAL(80, data.hourly[23].precipitationProbability);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_weather_client_initialization);
    RUN_TEST(test_weather_client_get_weather_desc);
    RUN_TEST(test_weather_client_fetch_weather_native_mock);
    RUN_TEST(test_weather_client_zip_code_initialization);
    RUN_TEST(test_weather_client_parse_weather_json_custom);
    RUN_TEST(test_weather_client_parse_owm_json);
    RUN_TEST(test_weather_client_parse_ip_location_json);
    RUN_TEST(test_weather_client_parse_ip_location_json_fail);
    RUN_TEST(test_weather_client_is_location_empty);
    RUN_TEST(test_weather_client_parse_weather_json_hourly);
    RUN_TEST(test_weather_client_parse_owm_json_hourly);
    return UNITY_END();
}
