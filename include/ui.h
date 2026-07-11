#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "weather_client.h"

extern volatile bool settings_unit_changed;
extern volatile bool settings_brightness_changed;
extern volatile bool settings_timezone_changed;
extern volatile bool settings_theme_changed;

void initUI();
void updateWifiStatus(bool connected);
void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed);
void updateTimeUI(const char* time_str);
void updateForecastUI(const WeatherData& data);
void updateFooterUI(const char* update_time, const char* city);

#endif // UI_H
