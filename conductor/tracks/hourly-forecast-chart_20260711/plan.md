# Implementation Plan - hourly-forecast-chart_20260711

## Phase 1: API and Data Parsing
- [x] Task: Update Open-Meteo and OpenWeatherMap APIs to request hourly parameters (c95c88c)
- [x] Task: Parse hourly JSON arrays and implement unit tests in `test_weather_client` (c95c88c)

## Phase 2: LVGL Chart UI Setup
- [x] Task: Create new tab widget inside tabview in `src/ui.cpp` (e4584f0)
- [x] Task: Instantiate `lv_chart` with proper bounds, spacing, and axis divisions (e4584f0)
- [x] Task: Write code to map the data points onto the chart series and dynamically redraw it (e4584f0)
- [ ] Task: Conductor - User Manual Verification 'Phase 1 & 2' (Protocol in workflow.md)
