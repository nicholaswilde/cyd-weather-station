#include <Arduino.h>
#include "display.h"
#include "wifi_manager.h"
#include "ui.h"
#include "weather_client.h"
#include "config/config.h"

#if USE_LDR_AUTO_BACKLIGHT
#include "backlight_manager.h"
BacklightManager backlight(TFT_BL, 0, 10.0f);
unsigned long lastBacklightUpdate = 0;
const unsigned long backlightUpdateInterval = 1000; // 1 second
#endif

WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
#if USE_ZIP_CODE
WeatherClient weather(WEATHER_ZIP_CODE);
#else
WeatherClient weather(WEATHER_API_LATITUDE, WEATHER_API_LONGITUDE);
#endif

unsigned long lastWifiUpdate = 0;
const unsigned long wifiUpdateInterval = 1000; // 1 second

unsigned long lastWeatherUpdate = 0;
const unsigned long weatherUpdateInterval = WEATHER_UPDATE_INTERVAL_MINS * 60 * 1000UL;
bool hasInitialFetch = false;
bool ntpInitialized = false;

void setup() {
    Serial.begin(115200);
    Serial.println("[System] Booting ESP32 CYD Weather Station...");

    // Initialize hardware display and touch drivers
    initDisplayAndTouch();

#if USE_LDR_AUTO_BACKLIGHT
    backlight.begin();
#endif

    // Initialize LVGL UI framework
    initLVGL();

    // Initialize UI layouts
    initUI();

    // Initialize WiFi Connection
    wifi.begin();
}

void loop() {
    // Periodically call LVGL task handler
    lv_timer_handler();

    // Feed LVGL ticks
    delay(5);
    lv_tick_inc(5);

    unsigned long currentMillis = millis();

#if USE_LDR_AUTO_BACKLIGHT
    #ifndef NATIVE_TEST
    if (currentMillis - lastBacklightUpdate >= backlightUpdateInterval) {
        lastBacklightUpdate = currentMillis;
        uint16_t ldrRaw = analogRead(LDR_PIN);
        backlight.update(ldrRaw);
    }
    #endif
#endif

    // Periodically update Wi-Fi Connection Manager and fetch weather/time
    if (currentMillis - lastWifiUpdate >= wifiUpdateInterval) {
        lastWifiUpdate = currentMillis;
        wifi.update();

        // Reflect WiFi status in UI
        bool isConnected = (wifi.getState() == WIFI_STATE_CONNECTED);
        updateWifiStatus(isConnected);

        if (isConnected) {
            // Initialize NTP once connection is established
#ifndef NATIVE_TEST
            if (!ntpInitialized) {
                Serial.println("[System] Initializing NTP client...");
                configTime(GMT_OFFSET_SEC, DST_OFFSET_SEC, NTP_SERVER);
                ntpInitialized = true;
            }
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                char timeStr[16];
                strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
                updateTimeUI(timeStr);
            }
#else
            updateTimeUI("12:00");
#endif

            // Check for weather updates
            if (!hasInitialFetch || (currentMillis - lastWeatherUpdate >= weatherUpdateInterval)) {
                Serial.println("[System] Fetching weather update...");
                WeatherData data = weather.fetchWeather();
                if (data.valid) {
                    updateWeatherUI(data.temperature, data.humidity, data.status.c_str());
                    lastWeatherUpdate = currentMillis;
                    hasInitialFetch = true;
                }
            }
        }
    }
}
