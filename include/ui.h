#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "weather_client.h"

extern volatile bool settings_unit_changed;
extern volatile bool settings_brightness_changed;
extern volatile bool settings_timezone_changed;
extern volatile bool settings_theme_changed;
extern volatile bool settings_sd_logging_changed;
extern volatile bool settings_screenshot_server_changed;
extern volatile bool settings_orientation_changed;

void initUI();
void updateWifiStatus(bool connected);
void updateWifiAPMode(const char* apSSID);
void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed, int windDirection);
void updateTimeUI(const char* time_str);
void updateForecastUI(const WeatherData& data);
void updateFooterUI(const char* update_time, const char* city);

#endif // UI_H
