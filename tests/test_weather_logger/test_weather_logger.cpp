#include <unity.h>
#include <ctime>
#include "weather_logger.h"
#include "sd_card_manager.h"
#include "../mocks/mocks.cpp"
#include "../../src/sd_card_manager.cpp"
#include "../../src/weather_logger.cpp"

void setUp(void) {
    mock_sd_card_present = true;
    mock_sd_card_mounted = false;
    mock_files.clear();
    SdCardManager::end();
}

void tearDown(void) {
    SdCardManager::end();
}

void test_format_csv_row(void) {
    struct tm timeinfo;
    timeinfo.tm_year = 2026 - 1900;
    timeinfo.tm_mon = 7 - 1; // July
    timeinfo.tm_mday = 11;
    timeinfo.tm_hour = 12;
    timeinfo.tm_min = 34;
    timeinfo.tm_sec = 56;

    WeatherData data;
    data.temperature = 72.5f;
    data.humidity = 60;
    data.windSpeed = 8.8f;
    data.windDirection = 180;
    data.weatherCode = 0;

    String expected = "2026-07-11 12:34:56,72.5,60,8.8,180,0\n";
    String actual = WeatherLogger::formatCsvRow(timeinfo, data);
    TEST_ASSERT_EQUAL_STRING(expected.c_str(), actual.c_str());
}

void test_log_weather_creates_file_with_header(void) {
    struct tm timeinfo;
    timeinfo.tm_year = 2026 - 1900;
    timeinfo.tm_mon = 7 - 1;
    timeinfo.tm_mday = 11;
    timeinfo.tm_hour = 12;
    timeinfo.tm_min = 34;
    timeinfo.tm_sec = 56;

    WeatherData data;
    data.temperature = 72.5f;
    data.humidity = 60;
    data.windSpeed = 8.8f;
    data.windDirection = 180;
    data.weatherCode = 0;

    // File should not exist initially
    TEST_ASSERT_FALSE(mock_files.count("/weather_history.csv") > 0);

    // Call logger
    bool success = WeatherLogger::logWeather(timeinfo, data);
    TEST_ASSERT_TRUE(success);

    // File should exist now and contain header + data
    TEST_ASSERT_TRUE(mock_files.count("/weather_history.csv") > 0);
    std::string expectedContent = 
        "Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode\n"
        "2026-07-11 12:34:56,72.5,60,8.8,180,0\n";
    TEST_ASSERT_EQUAL_STRING(expectedContent.c_str(), mock_files["/weather_history.csv"].c_str());
}

void test_log_weather_appends_without_header_if_exists(void) {
    // Pre-populate file with header and first record
    mock_files["/weather_history.csv"] = 
        "Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode\n"
        "2026-07-11 12:34:56,72.5,60,8.8,180,0\n";
    
    // Explicitly mount card to simulate already running state
    mock_sd_card_present = true;
    SdCardManager::begin();

    struct tm timeinfo2;
    timeinfo2.tm_year = 2026 - 1900;
    timeinfo2.tm_mon = 7 - 1;
    timeinfo2.tm_mday = 11;
    timeinfo2.tm_hour = 13;
    timeinfo2.tm_min = 34;
    timeinfo2.tm_sec = 56;

    WeatherData data2;
    data2.temperature = 73.0f;
    data2.humidity = 58;
    data2.windSpeed = 9.2f;
    data2.windDirection = 190;
    data2.weatherCode = 1;

    bool success = WeatherLogger::logWeather(timeinfo2, data2);
    TEST_ASSERT_TRUE(success);

    std::string expectedContent = 
        "Timestamp,Temperature,Humidity,WindSpeed,WindDirection,WeatherCode\n"
        "2026-07-11 12:34:56,72.5,60,8.8,180,0\n"
        "2026-07-11 13:34:56,73.0,58,9.2,190,1\n";
    TEST_ASSERT_EQUAL_STRING(expectedContent.c_str(), mock_files["/weather_history.csv"].c_str());
}

void test_log_weather_fails_if_no_sd(void) {
    struct tm timeinfo;
    WeatherData data;

    // Simulate SD card missing
    mock_sd_card_present = false;

    bool success = WeatherLogger::logWeather(timeinfo, data);
    TEST_ASSERT_FALSE(success);
    TEST_ASSERT_FALSE(mock_files.count("/weather_history.csv") > 0);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_format_csv_row);
    RUN_TEST(test_log_weather_creates_file_with_header);
    RUN_TEST(test_log_weather_appends_without_header_if_exists);
    RUN_TEST(test_log_weather_fails_if_no_sd);
    return UNITY_END();
}
