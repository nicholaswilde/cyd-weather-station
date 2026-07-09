#ifndef UI_H
#define UI_H

#include <lvgl.h>

void initUI();
void updateWifiStatus(bool connected);
void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode);
void updateTimeUI(const char* time_str);

#endif // UI_H
