#ifndef UI_H
#define UI_H

#include <lvgl.h>
#include "weather_client.h"

void initUI();
void updateWifiStatus(bool connected);
void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed);
void updateTimeUI(const char* time_str);
void updateForecastUI(const WeatherData& data);

#endif // UI_H
