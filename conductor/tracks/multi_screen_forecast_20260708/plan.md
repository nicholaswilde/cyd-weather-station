# Implementation Plan - Multi-Screen Tabview & Weather Forecast (`multi_screen_forecast_20260708`)

## Phase 1: Weather Client Forecast Parsing [checkpoint: 016653d]
- [x] Task: Extend API client and write parser tests (ac0a9aa)
    - [x] Update `WeatherData` struct in `include/weather_client.h` to store forecast entries (3 days)
    - [x] Write unit tests in `tests/test_weather_client` verifying daily forecast parsing from JSON payload
    - [x] Update `WeatherClient::fetchWeather()` in `src/weather_client.cpp` to fetch and parse daily forecast parameters from Open-Meteo
    - [x] Verify all tests pass on native target
- [x] Task: Conductor - User Manual Verification 'Phase 1: Weather Client Forecast Parsing' (Protocol in workflow.md) (016653d)

## Phase 2: LVGL Tabview Integration & Forecast Layout [checkpoint: 003d666]
- [x] Task: Implement Tabview and forecast screen in UI (543a9f7)
    - [x] Refactor `initUI()` in `src/ui.cpp` to create an `lv_tabview` object with two tabs: Current and Forecast
    - [x] Re-add the existing current weather widgets to the first tab
    - [x] Design and add forecast widgets (labels, grids/rows) in the second tab
    - [x] Implement `updateForecastUI(...)` to populate min/max temperatures and status codes
    - [x] Hook the forecast update to the main fetch routine in [main.cpp](file:///home/nicholas/git/nicholaswilde/cyd-weather-station/src/main.cpp)
    - [x] Verify compilation and test suite correctness
- [x] Task: Conductor - User Manual Verification 'Phase 2: LVGL Tabview Integration & Forecast Layout' (Protocol in workflow.md) (003d666)
