# Implementation Plan - hourly-forecast-chart_20260711

## Phase 1: API and Data Parsing
- [ ] Task: Update Open-Meteo and OpenWeatherMap APIs to request hourly parameters
- [ ] Task: Parse hourly JSON arrays and implement unit tests in `test_weather_client`

## Phase 2: LVGL Chart UI Setup
- [ ] Task: Create new tab widget inside tabview in `src/ui.cpp`
- [ ] Task: Instantiate `lv_chart` with proper bounds, spacing, and axis divisions
- [ ] Task: Write code to map the data points onto the chart series and dynamically redraw it
- [ ] Task: Conductor - User Manual Verification 'Phase 1 & 2' (Protocol in workflow.md)
