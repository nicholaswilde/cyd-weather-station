# CYD Weather Station - Project Critical Information

## Core Architecture
- ESP32 Cheap Yellow Display (CYD) Weather Station running LVGL 8.3 and Arduino framework.
- Main display and touch management: `src/display.cpp`, `src/touch_manager.cpp`, `src/ui.cpp`.
- Hardware configuration and pinouts: `config/config.h`, `platformio.ini`.
- Network and services: `src/wifi_manager.cpp`, `src/mqtt_manager.cpp`, `src/weather_client.cpp`.
- Preferences and configuration: `src/settings_manager.cpp`, `include/settings_manager.h`.

## Development & Build Commands
- Build firmware: `pio run -e cyd_28r` or `pio run -e cyd_35c`
- Run native tests: `pio test -e native`
- Generate compilation database: `pio run -t compiledb`
- Clean build: `pio run -t clean`

## Codebase Standards
- Adding settings: Follow the 5-point checklist in `AGENTS.md` (config.h, web UI, on-device UI, MQTT discovery, /api/config REST).
- Prefix shell/git commands with `rtk` where applicable.
