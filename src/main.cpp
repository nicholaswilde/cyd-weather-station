#include <Arduino.h>
#include "display.h"
#include "wifi_manager.h"
#include "ui.h"
#include "config/config.h"

WifiManager wifi(WIFI_SSID, WIFI_PASSWORD);
unsigned long lastWifiUpdate = 0;
const unsigned long wifiUpdateInterval = 1000; // 1 second

void setup() {
    Serial.begin(115200);
    Serial.println("[System] Booting ESP32 CYD Weather Station...");

    // Initialize hardware display and touch drivers
    initDisplayAndTouch();

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

    // Periodically update Wi-Fi Connection Manager
    unsigned long currentMillis = millis();
    if (currentMillis - lastWifiUpdate >= wifiUpdateInterval) {
        lastWifiUpdate = currentMillis;
        wifi.update();

        // Reflect WiFi status in UI
        bool isConnected = (wifi.getState() == WIFI_STATE_CONNECTED);
        updateWifiStatus(isConnected);
    }
}
