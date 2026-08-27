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
#include <DHT.h>

#define DHTPIN 22
DHT dht22(DHTPIN, DHT22);
DHT dht11(DHTPIN, DHT11);
unsigned long lastDhtRead = 0;
BacklightManager backlight(TFT_BL, 0, 10.0f);
ScreenSaverManager screensaver(backlight, SCREENSAVER_TIMEOUT_MS);
ButtonManager buttonManager(BOOT_BUTTON_PIN);
unsigned long lastBacklightUpdate = 0;
const unsigned long backlightUpdateInterval = 1000; // 1 second
SettingsManager settings;

WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
MqttManager mqtt("", 1883, "", "", "");
WeatherClient weather;

#if USE_RGB_LED_STATUS
LedManager led(4, 16, 17);
WifiState lastWifiState = WIFI_STATE_DISCONNECTED;
#endif

unsigned long lastWifiUpdate = 0;
const unsigned long wifiUpdateInterval = 1000; // 1 second

unsigned long lastWeatherUpdate = 0;
bool hasInitialFetch = false;
bool footerNeedsTimeUpdate = false;
String lastCityName = "";
bool ntpInitialized = false;
unsigned long lastAutoBrightnessCheck = 0;
bool force_mqtt_publish = false;
bool pending_ui_sync = false;

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

    dht22.begin();
    dht11.begin();
    Serial.println("[Sensor] DHT sensors initialized on GPIO 22");

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

    mqtt.updateConfig(settings.getMqttServer(), settings.getMqttPort(), settings.getMqttUser(), settings.getMqttPassword(), settings.getMqttBaseTopic());
    mqtt.begin();

    mqtt.onMessage([](const String& topic, const String& payload) {
        if (topic.endsWith("command/reboot")) {
            if (payload == "1" || payload == "true" || payload == "ON" || payload == "REBOOT") {
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
        } else if (topic.endsWith("command/auto_brightness")) {
            bool en = (payload == "ON" || payload == "1" || payload == "true");
            settings.setAutoBrightness(en);
            settings_brightness_changed = true;
        } else if (topic.endsWith("command/screensaver")) {
            bool en = (payload == "ON" || payload == "1" || payload == "true");
            settings.setScreensaverEnabled(en);
            // No explicit flag for screensaver, it will be handled implicitly
            // when it timeouts. We can just update it.
            if (!en) screensaver.wake(settings.getBrightness());
        } else if (topic.endsWith("command/sleep_schedule")) {
            bool en = (payload == "ON" || payload == "1" || payload == "true");
            settings.setSleepScheduleEnabled(en);
        } else if (topic.endsWith("command/sleep_start")) {
            settings.setSleepStartTime(payload);
        } else if (topic.endsWith("command/sleep_end")) {
            settings.setSleepEndTime(payload);
        } else if (topic.endsWith("command/theme")) {
            int theme = 1; // Default Mocha
            if (payload == "Macchiato") theme = 2;
            else if (payload == "Frappe") theme = 3;
            else if (payload == "Latte") theme = 4;
            settings.setThemeFlavor(theme);
            settings_theme_changed = true;
        } else if (topic.endsWith("command/units")) {
            int units = 2; // Default Imperial
            if (payload == "Metric") units = 1;
            settings.setUnitSystem(units);
            settings_unit_changed = true;
        } else if (topic.endsWith("command/screen_orientation")) {
            int orient = 1; // Default Landscape
            if (payload == "Portrait") orient = 0;
            else if (payload == "Portrait Rev") orient = 2;
            else if (payload == "Landscape Rev") orient = 3;
            settings.setScreenOrientation(orient);
            settings_orientation_changed = true;
        } else if (topic.endsWith("command/update_interval")) {
            int interval = payload.toInt();
            if (interval >= 1 && interval <= 120) {
                settings.setWeatherUpdateInterval(interval);
            }
        } else if (topic.endsWith("command/local_sensor")) {
            settings.setLocalSensorEnabled(payload == "ON");
            settings_local_sensor_changed = true;
        } else if (topic.endsWith("command/local_sensor_type")) {
            int type = 1;
            if (payload == "DHT22") type = 1;
            else if (payload == "SHT40") type = 2;
            else if (payload == "DHT11") type = 3;
            settings.setLocalSensorType(type);
            settings_local_sensor_changed = true;
        } else if (topic.endsWith("command/local_sensor_update_interval")) {
            int interval = payload.toInt();
            if (interval >= 1 && interval <= 120) {
                settings.setLocalSensorUpdateInterval(interval);
                settings_local_sensor_changed = true;
            }
        } else if (topic.endsWith("command/local_sensor_temp_offset")) {
            float offset = payload.toFloat();
            if (offset >= -10.0f && offset <= 10.0f) {
                settings.setLocalSensorTempOffset(offset);
                settings_local_sensor_changed = true;
            }
        } else if (topic.endsWith("command/local_sensor_hum_offset")) {
            float offset = payload.toFloat();
            if (offset >= -20.0f && offset <= 20.0f) {
                settings.setLocalSensorHumOffset(offset);
                settings_local_sensor_changed = true;
            }
        }
        
        force_mqtt_publish = true;
        pending_ui_sync = true;
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
    if (pending_ui_sync) {
        pending_ui_sync = false;
        ui_sync_toggles();
    }

    // Periodically call LVGL task handler
    lv_timer_handler();

    // Feed LVGL ticks
    delay(5);
    lv_tick_inc(5);

    wifi.update();

    unsigned long currentMillis = millis();

    // Sleep Schedule Logic
    static unsigned long lastSleepCheck = 0;
    if (currentMillis - lastSleepCheck >= 60000) {
        lastSleepCheck = currentMillis;
        if (settings.getSleepScheduleEnabled()) {
            struct tm timeinfo;
            if (getLocalTime(&timeinfo)) {
                int currentMins = timeinfo.tm_hour * 60 + timeinfo.tm_min;
                
                String startTime = settings.getSleepStartTime();
                String endTime = settings.getSleepEndTime();
                
                int startHour = startTime.substring(0, 2).toInt();
                int startMin = startTime.substring(3, 5).toInt();
                int endHour = endTime.substring(0, 2).toInt();
                int endMin = endTime.substring(3, 5).toInt();
                
                int startMins = startHour * 60 + startMin;
                int endMins = endHour * 60 + endMin;
                
                bool shouldSleep = false;
                if (startMins < endMins) {
                    shouldSleep = (currentMins >= startMins && currentMins < endMins);
                } else {
                    shouldSleep = (currentMins >= startMins || currentMins < endMins);
                }
                
                static bool scheduledSleepActive = false;
                if (shouldSleep && !scheduledSleepActive) {
                    screensaver.sleep();
                    scheduledSleepActive = true;
                } else if (!shouldSleep && scheduledSleepActive) {
                    screensaver.wake(settings.getBrightness());
                    scheduledSleepActive = false;
                }
            }
        }
    }

#if USE_RGB_LED_STATUS
    led.update(currentMillis);
#endif

    // Periodically read Local Sensor if enabled
    if (settings.getLocalSensorEnabled()) {
        unsigned long currentUpdateIntervalMs = settings.getLocalSensorUpdateInterval() * 1000UL;
        if (lastDhtRead == 0 || (currentMillis - lastDhtRead >= currentUpdateIntervalMs)) {
            lastDhtRead = currentMillis;
            
            if (settings.getLocalSensorType() == 1) { // DHT22
                float h = dht22.readHumidity();
                bool isFahrenheit = (settings.getUnitSystem() == UNIT_IMPERIAL);
                float t = dht22.readTemperature(isFahrenheit);
                
                if (isnan(h) || isnan(t)) {
                    Serial.println("[Sensor] Failed to read from DHT22 sensor!");
                } else {
                    h += settings.getLocalSensorHumOffset();
                    t += settings.getLocalSensorTempOffset();
                    Serial.printf("[Sensor] DHT22 -> Humidity: %.1f%%  Temperature: %.1f%s\n", h, t, isFahrenheit ? "°F" : "°C");
                    updateLocalSensorUI(t, h);
                    
                    if (mqtt.isConnected()) {
                        char tempPayload[16];
                        char humPayload[16];
                        snprintf(tempPayload, sizeof(tempPayload), "%.1f", t);
                        snprintf(humPayload, sizeof(humPayload), "%.1f", h);
                        mqtt.publish("sensor/local_temperature", tempPayload);
                        mqtt.publish("sensor/local_humidity", humPayload);
                    }
                }
            } else if (settings.getLocalSensorType() == 2) { // SHT40
                // Placeholder for next phase
            } else if (settings.getLocalSensorType() == 3) { // DHT11
                float h = dht11.readHumidity();
                bool isFahrenheit = (settings.getUnitSystem() == UNIT_IMPERIAL);
                float t = dht11.readTemperature(isFahrenheit);
                
                if (isnan(h) || isnan(t)) {
                    Serial.println("[Sensor] Failed to read from DHT11 sensor!");
                } else {
                    h += settings.getLocalSensorHumOffset();
                    t += settings.getLocalSensorTempOffset();
                    Serial.printf("[Sensor] DHT11 -> Humidity: %.1f%%  Temperature: %.1f%s\n", h, t, isFahrenheit ? "°F" : "°C");
                    updateLocalSensorUI(t, h);
                    
                    if (mqtt.isConnected()) {
                        char tempPayload[16];
                        char humPayload[16];
                        snprintf(tempPayload, sizeof(tempPayload), "%.1f", t);
                        snprintf(humPayload, sizeof(humPayload), "%.1f", h);
                        mqtt.publish("sensor/local_temperature", tempPayload);
                        mqtt.publish("sensor/local_humidity", humPayload);
                    }
                }
            }
        }
    }

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
            mqtt.updateConfig(settings.getMqttServer(), settings.getMqttPort(), settings.getMqttUser(), settings.getMqttPassword(), settings.getMqttBaseTopic());
            if (wifi.getState() == WIFI_STATE_CONNECTED) {
                Serial.println("[System] Wi-Fi connected, connecting to MQTT...");
                mqtt.onNetworkAvailable();
            }
        } else {
            mqtt.disconnect();
        }
    }

    if (settings_local_sensor_changed) {
        settings_local_sensor_changed = false;
        pending_ui_sync = true;
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
                
                if (footerNeedsTimeUpdate) {
                    updateFooterUI(timeStr, lastCityName.c_str());
                    footerNeedsTimeUpdate = false;
                }
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
                                                
                        mqtt.publish("weather/temperature", tempPayload.c_str());
                        mqtt.publish("weather/status", data.status.c_str());
                        mqtt.publish("weather/humidity", humPayload.c_str());
                        mqtt.publish("weather/wind_speed", windSpeedPayload.c_str());
                        mqtt.publish("weather/wind_direction", cardinalDirection);
                        mqtt.publish("weather/city", data.cityName.c_str());
                    }
                    // ----------------------------
#ifndef NATIVE_TEST
                    // Update footer: "Last Update: HH:MM | City"
                    struct tm timeinfo;
                    if (getLocalTime(&timeinfo, 10)) {
                        char timeStr[16];
                        strftime(timeStr, sizeof(timeStr), "%H:%M", &timeinfo);
                        updateFooterUI(timeStr, data.cityName.c_str());
                        footerNeedsTimeUpdate = false;

                        // Log weather data to SD card if enabled and card is present
                        if (settings.getSdLoggingEnabled() && SdCardManager::isCardPresent()) {
                            WeatherLogger::logWeather(timeinfo, data);
                        }
                    } else {
                        updateFooterUI("--:--", data.cityName.c_str());
                        lastCityName = data.cityName.c_str();
                        footerNeedsTimeUpdate = true;
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

    // Publish MQTT Diagnostics and Settings periodically
    static unsigned long lastMqttDiagMillis = 0;
    if (mqtt.isConnected() && (force_mqtt_publish || currentMillis - lastMqttDiagMillis >= 60000 || lastMqttDiagMillis == 0)) {
        force_mqtt_publish = false;
        lastMqttDiagMillis = currentMillis;
        
        mqtt.publish("system/uptime", String(currentMillis / 1000).c_str(), true);
        mqtt.publish("system/free_heap", String(ESP.getFreeHeap()).c_str(), true);
        mqtt.publish("system/wifi_rssi", String(WiFi.RSSI()).c_str(), true);
        mqtt.publish("system/ip", wifi.getIPAddress().c_str(), true);
        mqtt.publish("system/version", "v0.1.22", true);
        mqtt.publish("system/mac", WiFi.macAddress().c_str(), true);
        
        mqtt.publish("settings/auto_brightness", settings.getAutoBrightness() ? "ON" : "OFF", true);
        mqtt.publish("settings/screensaver", settings.getScreensaverEnabled() ? "ON" : "OFF", true);
        mqtt.publish("settings/sleep_schedule", settings.getSleepScheduleEnabled() ? "ON" : "OFF", true);
        mqtt.publish("settings/sleep_start", settings.getSleepStartTime().c_str(), true);
        mqtt.publish("settings/sleep_end", settings.getSleepEndTime().c_str(), true);
        
        String themeStr = "Mocha";
        switch (settings.getThemeFlavor()) {
            case 2: themeStr = "Macchiato"; break;
            case 3: themeStr = "Frappe"; break;
            case 4: themeStr = "Latte"; break;
        }
        mqtt.publish("settings/theme", themeStr.c_str(), true);
        
        mqtt.publish("settings/units", settings.getUnitSystem() == 1 ? "Metric" : "Imperial", true);
        
        String orientStr = "Landscape";
        switch (settings.getScreenOrientation()) {
            case 0: orientStr = "Portrait"; break;
            case 2: orientStr = "Portrait Rev"; break;
            case 3: orientStr = "Landscape Rev"; break;
        }
        mqtt.publish("settings/screen_orientation", orientStr.c_str(), true);
        mqtt.publish("settings/update_interval", String(settings.getWeatherUpdateInterval()).c_str(), true);
        
        mqtt.publish("settings/local_sensor", settings.getLocalSensorEnabled() ? "ON" : "OFF", true);
        String sensTypeStr = "DHT22";
        switch (settings.getLocalSensorType()) {
            case 1: sensTypeStr = "DHT22"; break;
            case 2: sensTypeStr = "SHT40"; break;
            case 3: sensTypeStr = "DHT11"; break;
        }
        mqtt.publish("settings/local_sensor_type", sensTypeStr.c_str(), true);
        mqtt.publish("settings/local_sensor_update_interval", String(settings.getLocalSensorUpdateInterval()).c_str(), true);
        mqtt.publish("settings/local_sensor_temp_offset", String(settings.getLocalSensorTempOffset(), 1).c_str(), true);
        mqtt.publish("settings/local_sensor_hum_offset", String(settings.getLocalSensorHumOffset(), 1).c_str(), true);
    }
}
