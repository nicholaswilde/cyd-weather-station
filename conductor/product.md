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
4. **Wi-Fi Connectivity:** Automated Wi-Fi connection with status indicators and reconnect logic.
5. **Open-Meteo Integration:** Automated periodic JSON API requests to fetch weather data without requiring an API key.

## References
- [Random Nerd Tutorials - ESP32 CYD LVGL Weather Station](https://randomnerdtutorials.com/esp32-cyd-lvgl-weather-station/)
- [GitHub Example Code](https://github.com/RuiSantosdotme/ESP32-TFT-Touchscreen/tree/main/examples/ESP32_LVGL_Weather_Station)
