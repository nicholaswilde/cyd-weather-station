#include <Arduino.h>
#include "display.h"
#include "wifi_manager.h"
#include "ui.h"
#include "weather_client.h"
#include "config/config.h"
#include "led_manager.h"

#include "backlight_manager.h"
#include "settings_manager.h"
BacklightManager backlight(TFT_BL, 0, 10.0f);
unsigned long lastBacklightUpdate = 0;
const unsigned long backlightUpdateInterval = 1000; // 1 second
SettingsManager settings;

WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
#if USE_ZIP_CODE
WeatherClient weather(WEATHER_ZIP_CODE);
#else
WeatherClient weather(WEATHER_API_LATITUDE, WEATHER_API_LONGITUDE);
#endif

#if USE_RGB_LED_STATUS
LedManager led(4, 16, 17);
WifiState lastWifiState = WIFI_STATE_DISCONNECTED;
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

    // Load saved preferences
    settings.begin();

    // Initialize hardware display and touch drivers
    initDisplayAndTouch();

    backlight.begin();
    if (!settings.getAutoBrightness()) {
        backlight.setManualBrightness(settings.getBrightness());
    }

    // Initialize LVGL UI framework
    initLVGL();

    // Initialize UI layouts
    initUI();

    // Initialize WiFi Connection
    wifi.begin();

#if USE_RGB_LED_STATUS
    led.begin();
    led.setEnabled(true);
#endif
}

void loop() {
    // Periodically call LVGL task handler
    lv_timer_handler();

    // Feed LVGL ticks
    delay(5);
    lv_tick_inc(5);

    unsigned long currentMillis = millis();

#if USE_RGB_LED_STATUS
    led.update(currentMillis);
#endif

    // Handle runtime settings changes from UI
    if (settings_unit_changed) {
        settings_unit_changed = false;
        Serial.println("[System] Temperature unit changed. Triggering weather refetch...");
        hasInitialFetch = false;
    }

    if (settings_brightness_changed) {
        settings_brightness_changed = false;
        if (!settings.getAutoBrightness()) {
            backlight.setManualBrightness(settings.getBrightness());
        }
    }

    if (settings_timezone_changed) {
        settings_timezone_changed = false;
#ifndef NATIVE_TEST
        if (ntpInitialized) {
            Serial.println("[System] Timezone offset changed. Reconfiguring NTP...");
            configTime(settings.getTimezoneOffset() * 3600, DST_OFFSET_SEC, NTP_SERVER);
        }
#endif
    }

    if (settings_theme_changed) {
        settings_theme_changed = false;
        Serial.println("[System] Theme changed. Reloading UI...");
        
        // Clean active screen and rebuild
        lv_obj_clean(lv_scr_act());
        initUI();
        
        // Force immediate updates of WiFi, time, and weather
        bool isConnected = (wifi.getState() == WIFI_STATE_CONNECTED);
        updateWifiStatus(isConnected);
        
#ifndef NATIVE_TEST
        if (ntpInitialized) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                char timeStr[16];
                strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
                updateTimeUI(timeStr);
            }
        }
#else
        updateTimeUI("12:00");
#endif
        
        // Trigger weather fetch to redraw UI with new theme colors
        hasInitialFetch = false;
    }

#ifndef NATIVE_TEST
    if (settings.getAutoBrightness()) {
        if (currentMillis - lastBacklightUpdate >= backlightUpdateInterval) {
            lastBacklightUpdate = currentMillis;
            uint16_t ldrRaw = analogRead(LDR_PIN);
            backlight.update(ldrRaw);
        }
    }
#endif

    // Periodically update Wi-Fi Connection Manager and fetch weather/time
    if (currentMillis - lastWifiUpdate >= wifiUpdateInterval) {
        lastWifiUpdate = currentMillis;
        wifi.update();

        // Reflect WiFi status in UI
        bool isConnected = (wifi.getState() == WIFI_STATE_CONNECTED);
        updateWifiStatus(isConnected);

#if USE_RGB_LED_STATUS
        WifiState currentWifiState = wifi.getState();
        if (currentWifiState != lastWifiState) {
            if (currentWifiState == WIFI_STATE_CONNECTED) {
                led.setState(LedManager::STATE_CONNECTED);
            } else if (currentWifiState == WIFI_STATE_CONNECTING) {
                led.setState(LedManager::STATE_CONNECTING);
            } else {
                led.setState(LedManager::STATE_DISCONNECTED);
            }
            lastWifiState = currentWifiState;
        }
#endif

        if (isConnected) {
            // Initialize NTP once connection is established
#ifndef NATIVE_TEST
            if (!ntpInitialized) {
                Serial.println("[System] Initializing NTP client...");
                configTime(settings.getTimezoneOffset() * 3600, DST_OFFSET_SEC, NTP_SERVER);
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
                    updateWeatherUI(data.temperature, data.humidity, data.status.c_str(), data.weatherCode, data.windSpeed);
                    updateForecastUI(data);

#if USE_RGB_LED_STATUS
                    // Trigger brief weather status feedback pulse
                    switch (data.weatherCode) {
                        case 0:
                        case 1:
                            led.setState(LedManager::STATE_PULSE_YELLOW);
                            break;
                        case 2:
                        case 3:
                        case 45:
                        case 48:
                            led.setState(LedManager::STATE_PULSE_WHITE);
                            break;
                        case 51:
                        case 53:
                        case 55:
                        case 61:
                        case 63:
                        case 65:
                        case 80:
                        case 81:
                        case 82:
                        case 56:
                        case 57:
                        case 66:
                        case 67:
                        case 71:
                        case 73:
                        case 75:
                        case 77:
                        case 85:
                        case 86:
                            led.setState(LedManager::STATE_PULSE_BLUE);
                            break;
                        case 95:
                        case 96:
                        case 99:
                            led.setState(LedManager::STATE_ALERT_RED);
                            break;
                        default:
                            break;
                    }
#endif

                    lastWeatherUpdate = currentMillis;
                    hasInitialFetch = true;
                }
            }
        }
    }
}
