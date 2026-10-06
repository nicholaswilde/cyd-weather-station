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
extern volatile bool settings_led_changed;
extern volatile bool settings_mqtt_changed;
extern volatile bool settings_local_sensor_changed;

void initUI();
void ui_sync_toggles();
void updateWifiStatus(bool connected);
void updateWifiAPMode(const char* apSSID);
void updateOfflineIndicator(bool isOffline);
void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed, int windDirection);
void updateLocalSensorUI(float temperature, float humidity);
void updateTimeUI(const char* time_str);
void updateForecastUI(const WeatherData& data);
void updateHourlyUI(const WeatherData& data);
void updateFooterUI(const char* update_time, const char* city);
void showScreenSaver();
void hideScreenSaver();
void updateScreenSaverTime(const char* time_str);
void setUIActiveTab(int index);
void setUIOrientation(int rotation);
const char* getCardinalDirection(int degrees);
void showUIStatusMessage(const char* message);

inline void formatHourlyTickLabel(char* buf, size_t buf_len, int tick_idx) {
    if (buf == nullptr || buf_len == 0) return;
    int hour_offset = tick_idx * 4;
    if (hour_offset == 0) {
        lv_snprintf(buf, buf_len, "Now");
    } else {
        lv_snprintf(buf, buf_len, "+%dh", hour_offset);
    }
}

#endif // UI_H
