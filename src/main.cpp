#include <Arduino.h>
#include <WiFi.h>
#include "display.h"
#include "wifi_manager.h"
#include "mqtt_manager.h"
#include "ui.h"
#include "weather_client.h"
#include "config/config.h"
#include "led_manager.h"
#include "sd_card_manager.h"
#include "weather_logger.h"
#include "weather_cache.h"
#include "screenshot_manager.h"
#include "backlight_manager.h"
#include "settings_manager.h"
#include "screensaver_manager.h"
#include "button_manager.h"
BacklightManager backlight(TFT_BL, 0, 10.0f);
ScreenSaverManager screensaver(backlight, SCREENSAVER_TIMEOUT_MS);
ButtonManager buttonManager(BOOT_BUTTON_PIN);
unsigned long lastBacklightUpdate = 0;
const unsigned long backlightUpdateInterval = 1000; // 1 second
SettingsManager settings;

WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
MqttManager mqtt("", 1883, "", "");
WeatherClient weather;

#if USE_RGB_LED_STATUS
LedManager led(4, 16, 17);
WifiState lastWifiState = WIFI_STATE_DISCONNECTED;
#endif

unsigned long lastWifiUpdate = 0;
const unsigned long wifiUpdateInterval = 1000; // 1 second

unsigned long lastWeatherUpdate = 0;
bool hasInitialFetch = false;
bool ntpInitialized = false;

// --- Wi-Fi Event Handlers ---
void onWiFiGotIP(WiFiEvent_t event, WiFiEventInfo_t info) {
    if (settings.getMqttEnabled()) {
        Serial.println("[System] Wi-Fi connected with IP! Signaling MQTT Manager...");
        mqtt.onNetworkAvailable();
    } else {
        Serial.println("[System] Wi-Fi connected with IP! MQTT is disabled, skipping connection.");
    }
}

void onWiFiDisconnect(WiFiEvent_t event, WiFiEventInfo_t info) {
    Serial.println("[System] Wi-Fi disconnected! Signaling MQTT Manager...");
    mqtt.onNetworkDisconnected();
}

void setup() {
    Serial.begin(115200);
    Serial.println("[System] Booting ESP32 CYD Weather Station...");

    // Load saved preferences
    settings.begin();
    screensaver.setTimeoutMs(settings.getScreensaverTimeout());

    // Check if SD card is present by attempting a begin/mount
    bool sdPresent = SdCardManager::begin();
    if (sdPresent && !settings.getSdLoggingEnabled()) {
        SdCardManager::end();
    }

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
    screensaver.begin();

    // Load offline cache on boot if available
    WeatherData cachedData;
    if (settings.getSdCacheEnabled() && WeatherCache::loadCache(cachedData)) {
        Serial.println("[System] Restored weather data from offline cache on boot.");
        updateWeatherUI(cachedData.temperature, cachedData.humidity, cachedData.status.c_str(), cachedData.weatherCode, cachedData.windSpeed, cachedData.windDirection);
        updateForecastUI(cachedData);
        updateHourlyUI(cachedData);
        updateOfflineIndicator(true);
        updateFooterUI("--:-- (Cached)", cachedData.cityName.c_str());
    }

    mqtt.updateConfig(settings.getMqttServer(), settings.getMqttPort(), settings.getMqttUser(), settings.getMqttPassword());
    mqtt.begin();

    mqtt.onMessage([](const String& topic, const String& payload) {
        if (topic.endsWith("command/reboot")) {
            if (payload == "1" || payload == "true" || payload == "ON") {
                Serial.println("[System] Reboot command received from MQTT.");
                ESP.restart();
            }
        } else if (topic.endsWith("command/brightness")) {
            int target = payload.toInt();
            if (target >= 0 && target <= 100) {
                Serial.printf("[System] Setting brightness to %d%% via MQTT.\n", target);
                settings.setBrightness(target);
                backlight.fadeTo(target, 500); // 500ms fade
            }
        }
    });

    // Register the Wi-Fi events so MQTT knows when the network drops/connects
    WiFi.onEvent(onWiFiGotIP, ARDUINO_EVENT_WIFI_STA_GOT_IP);
    WiFi.onEvent(onWiFiDisconnect, ARDUINO_EVENT_WIFI_STA_DISCONNECTED);
        
    // Initialize WiFi Connection
    wifi.setCredentials(settings.getWifiSSID(), settings.getWifiPassword());
    wifi.begin();

#if USE_RGB_LED_STATUS
    led.begin();
    led.setEnabled(settings.getLedEnabled());
    led.setBrightness(settings.getLedBrightness());
#endif
    
    // Initialize physical BOOT button (GPIO 0)
    buttonManager.begin();
}

void loop() {
    // Periodically call LVGL task handler
    lv_timer_handler();

    // Feed LVGL ticks
    delay(5);
    lv_tick_inc(5);

    wifi.update();

    unsigned long currentMillis = millis();

#if USE_RGB_LED_STATUS
    led.update(currentMillis);
#endif

    // Handle physical BOOT button events
    static bool ignoreButtonAction = false;
    static bool lastRawButtonState = HIGH;
    bool currentRawButtonState = digitalRead(BOOT_BUTTON_PIN);

    if (currentRawButtonState == LOW && lastRawButtonState == HIGH) {
#ifndef NATIVE_TEST
        if (screensaver.isActive()) {
            screensaver.wake(settings.getBrightness());
            ignoreButtonAction = true;
        } else {
            ignoreButtonAction = false;
        }
#else
        ignoreButtonAction = false;
#endif
    } else if (currentRawButtonState == HIGH && lastRawButtonState == LOW) {
        // Clear ignore flag once fully released
        if (ignoreButtonAction && !buttonManager.isPressed()) {
            ignoreButtonAction = false;
        }
    }
    lastRawButtonState = currentRawButtonState;

    ButtonAction action = buttonManager.update(currentMillis);
    if (ignoreButtonAction) {
        action = ButtonAction::NONE;
        // Keep checking if we can clear the ignore flag once the button is released
        if (currentRawButtonState == HIGH && !buttonManager.isPressed()) {
            ignoreButtonAction = false;
        }
    }

    if (action == ButtonAction::SINGLE_PRESS) {
        Serial.println("[System] BOOT button single press: Triggering weather refresh...");
        showUIStatusMessage("Refreshing weather...");
        hasInitialFetch = false;
        lastWifiUpdate = 0; // Force immediate refresh on next loop iteration
    } else if (action == ButtonAction::LONG_PRESS) {
        Serial.println("[System] BOOT button long press: Taking screenshot...");
        showUIStatusMessage("Screenshot saved!");
        
        struct tm timeinfo;
        std::string filename;
#ifndef NATIVE_TEST
        if (getLocalTime(&timeinfo, 10)) {
            filename = ScreenshotManager::generateFilename(&timeinfo, 0);
        } else {
            filename = ScreenshotManager::generateFilename(nullptr, millis());
        }
#else
        filename = ScreenshotManager::generateFilename(nullptr, millis());
#endif
        ScreenshotManager::captureToSD(filename.c_str());
    }

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
            Serial.println("[System] Timezone/DST settings changed. Reconfiguring NTP...");
            configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());
        }
#endif
    }

    if (settings_sd_logging_changed) {
        settings_sd_logging_changed = false;
        if (settings.getSdLoggingEnabled()) {
            Serial.println("[System] SD logging enabled, initializing SD card...");
            SdCardManager::begin();
        } else {
            Serial.println("[System] SD logging disabled, unmounting SD card...");
            SdCardManager::end();
        }
    }

    if (settings_screenshot_server_changed) {
        settings_screenshot_server_changed = false;
        bool enabled = settings.getScreenshotServerEnabled();
        Serial.printf("[System] Screenshot server %s.\n", enabled ? "enabled" : "disabled");
        wifi.applyScreenshotServerSetting(enabled);
    }

    if (settings_mqtt_changed) {
        settings_mqtt_changed = false;
        bool enabled = settings.getMqttEnabled();
        Serial.printf("[System] MQTT %s.\n", enabled ? "enabled" : "disabled");
        if (enabled) {
            mqtt.updateConfig(settings.getMqttServer(), settings.getMqttPort(), settings.getMqttUser(), settings.getMqttPassword());
            if (wifi.getState() == WIFI_STATE_CONNECTED) {
                Serial.println("[System] Wi-Fi connected, connecting to MQTT...");
                mqtt.onNetworkAvailable();
            }
        } else {
            mqtt.disconnect();
        }
    }

#if USE_RGB_LED_STATUS
    if (settings_led_changed) {
        settings_led_changed = false;
        led.setEnabled(settings.getLedEnabled());
        led.setBrightness(settings.getLedBrightness());
        Serial.printf("[System] LED enabled=%s brightness=%d.\n",
            settings.getLedEnabled() ? "true" : "false",
            settings.getLedBrightness());
    }
#endif

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
            if (getLocalTime(&timeinfo, 10)) {
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

    if (settings_orientation_changed) {
        settings_orientation_changed = false;
        int orientation = settings.getScreenOrientation();
        Serial.printf("[System] Screen orientation changed to %d. Rebooting...\n", orientation);

#ifndef NATIVE_TEST
        delay(500); // Give serial time to flush
        ESP.restart();
#endif
    }

#ifndef NATIVE_TEST
    if (settings.getScreensaverEnabled()) {
        screensaver.update(currentMillis);
    }
    if (!screensaver.isActive() && settings.getAutoBrightness()) {
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

        // Reflect WiFi status in UI
        bool isConnected = (wifi.getState() == WIFI_STATE_CONNECTED);
        if (wifi.getState() == WIFI_STATE_AP_MODE) {
            updateWifiAPMode(wifi.getAPSSID().c_str());
        } else {
            updateWifiStatus(isConnected);
        }

#if USE_RGB_LED_STATUS
        WifiState currentWifiState = wifi.getState();
        if (currentWifiState != lastWifiState) {
            if (currentWifiState == WIFI_STATE_CONNECTED) {
                led.setState(LedManager::STATE_CONNECTED);
            } else if (currentWifiState == WIFI_STATE_CONNECTING) {
                led.setState(LedManager::STATE_CONNECTING);
            } else if (currentWifiState == WIFI_STATE_AP_MODE) {
                led.setState(LedManager::STATE_AP_MODE);
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
                configTzTime(settings.getTimezone().c_str(), settings.getNtpServer().c_str());
                ntpInitialized = true;
            }
            struct tm timeinfo;
            if (getLocalTime(&timeinfo, 10)) {
                char timeStr[16];
                strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
                updateTimeUI(timeStr);
            }
#else
            updateTimeUI("12:00");
#endif

            // Check for weather updates
            unsigned long currentUpdateInterval = settings.getWeatherUpdateInterval() * 60 * 1000UL;
            if (!hasInitialFetch || (currentMillis - lastWeatherUpdate >= currentUpdateInterval)) {
                if (weather.isLocationEmpty()) {
                    Serial.println("[System] Location settings empty/default. Querying IP location fallback...");
                    String latStr, lonStr, city;
                    if (weather.fetchIpLocation(latStr, lonStr, city)) {
                        Serial.printf("[System] Fallback geolocation resolved: %s (%s, %s)\n",
                                      city.c_str(), latStr.c_str(), lonStr.c_str());
                    } else {
                        Serial.println("[System] Fallback geolocation query failed.");
                    }
                }
                Serial.println("[System] Fetching weather update...");
                WeatherData data = weather.fetchWeather();
                if (data.valid) {
                    updateWeatherUI(data.temperature, data.humidity, data.status.c_str(), data.weatherCode, data.windSpeed, data.windDirection);
                    updateForecastUI(data);
                    updateHourlyUI(data);
                    updateOfflineIndicator(false);
                    if (settings.getSdCacheEnabled()) {
                        WeatherCache::saveCache(data);
                    }

                    // --- NEW: Publish to MQTT ---
                    if (mqtt.isConnected()) {
                        String tempPayload = String(data.temperature, 1);
                        String humPayload = String(data.humidity);
                        String windSpeedPayload = String(data.windSpeed, 1);
                        // Get the Cardinal direction using the newly exposed UI function
                        const char* cardinalDirection = getCardinalDirection(data.windDirection);
                                                
                        mqtt.publish("cyd/weather/temperature", tempPayload.c_str());
                        mqtt.publish("cyd/weather/status", data.status.c_str());
                        mqtt.publish("cyd/weather/humidity", humPayload.c_str());
                        mqtt.publish("cyd/weather/wind_speed", windSpeedPayload.c_str());
                        mqtt.publish("cyd/weather/wind_direction", cardinalDirection);
                        mqtt.publish("cyd/weather/city", data.cityName.c_str());
                    }
                    // ----------------------------
#ifndef NATIVE_TEST
                    // Update footer: "Last Update: HH:MM | City"
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo, 10)) {
                        char timeStr[16];
                        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
                        updateFooterUI(timeStr, data.cityName.c_str());

                        // Log weather data to SD card if enabled and card is present
                        if (settings.getSdLoggingEnabled() && SdCardManager::isCardPresent()) {
                            WeatherLogger::logWeather(timeinfo, data);
                        }
                    } else {
                        updateFooterUI("--:--", data.cityName.c_str());
                    }
#else
                    updateFooterUI("12:00", data.cityName.c_str());
#endif

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
