# Initial Concept

A weather station app for the ESP32 Cheap Yellow Display (CYD)

# Product Guide: ESP32 CYD LVGL Weather Station

## Overview
The **ESP32 CYD LVGL Weather Station** is an interactive, touch-screen-enabled weather station app designed for the **ESP32-2432S028R** (Cheap Yellow Display / CYD) hardware. It fetches real-time weather data and forecasts from the free, open-source **Open-Meteo API** and displays it on the CYD screen using the **LVGL (Light and Versatile Graphics Library)** for a modern, responsive, and visually appealing user interface.

## Target Hardware
- **Display:** ESP32-2432S028R Cheap Yellow Display (CYD) 2.8" TFT touchscreen (typically ILI9341 driver + XPT2046 resistive touch controller).
- **Core:** ESP32 dual-core processor with integrated Wi-Fi.

## Key Features
1. **Real-time Weather Display:** Displays current weather conditions including temperature, humidity, wind speed, and precipitation.
2. **Forecast View:** Multi-day or hourly forecast screen to plan ahead.
3. **LVGL Touch UI:** A polished, touch-interactive interface with widgets, smooth animations, and visual clarity suited for a small display.
4. **Wi-Fi Connectivity:** Automated Wi-Fi connection with status indicators, reconnect logic, and an Access Point (AP) captive portal fallback configuration portal on timeout (30 seconds) allowing runtime Wi-Fi credentials setup.
5. **Open-Meteo Integration:** Automated periodic JSON API requests to fetch weather data without requiring an API key.
6. **Auto-Brightness Control:** Automatically dims the screen backlight in low-light environments and brightens it in well-lit rooms using the onboard LDR light sensor.
7. **Dynamic Weather Status Icons:** Displays a large (48px) weather icon corresponding to the current weather condition (clear, cloudy, rainy, snowy, stormy) next to the weather reading, dynamically colored using the Catppuccin color scheme.
8. **RGB LED Status Indicator:** Utilizes the onboard active-low RGB LED (GPIO 4/16/17) to provide secondary visual feedback about Wi-Fi connection states (slow blue blink for connecting, solid green for connected, fast red blink for disconnected, slow purple blink for AP mode) and brief weather status pulse notifications.
9. **Interactive Settings Screen:** Adds a Settings tab to the Tabview allowing runtime configuration of temperature units (Celsius/Fahrenheit), manual screen brightness, Auto Light brightness mode, and GMT timezone offsets (-12 to +14) via touch controls, persisting settings across boots.
10. **SD Weather History & Sensor Data Logging:** Periodically logs hourly weather and system variables (temperature, humidity, wind speed, wind direction, weather code) alongside synchronized NTP timestamps to a CSV format file (`weather_history.csv`) on the root of the microSD card, with boot-time mounting and robust error handling.
11. **Wireless OTA Firmware Updates:** Supports Over-The-Air (OTA) firmware updates via a web browser (`http://192.168.4.1/update`) when the device is in AP Mode, utilizing a beautiful Catppuccin-themed HTML upload portal with progress feedback and automatic reboot.
12. **Inactivity Sleep & Screen-Saver:** Auto-dims the screen backlight to 5% after 5 minutes of inactivity and displays a floating digital clock overlay that drifts periodically to prevent resistive screen pixel burn-in. Wakes instantly on any screen tap without triggering underlying widgets.
13. **Multi-Action Physical Button:** Re-purposes the onboard BOOT button (GPIO 0) for multiple actions: a single quick press triggers an immediate weather update (with an on-screen "Refreshing weather..." notification), and a long press (>= 2 seconds) takes a screenshot saved to the SD card (showing "Screenshot saved!"). Waking from screensaver ignores the action.
14. **SD Offline Cache Recovery:** Periodically saves weather data to microSD card (`/weather_cache.json`) in JSON format. On boot, if Wi-Fi connection fails or is unavailable, the device restores the cached weather data, updates the UI, and displays an offline status badge.
15. **IP-Based Auto-Location:** Automatically queries an IP geolocation API (`ip-api.com`) to resolve coordinates and city name on boot if the user has not configured manual coordinates or a ZIP code. Instantly updates the weather forecast metrics for the resolved city.

## References
- [Random Nerd Tutorials - ESP32 CYD LVGL Weather Station](https://randomnerdtutorials.com/esp32-cyd-lvgl-weather-station/)
- [GitHub Example Code](https://github.com/RuiSantosdotme/ESP32-TFT-Touchscreen/tree/main/examples/ESP32_LVGL_Weather_Station)
