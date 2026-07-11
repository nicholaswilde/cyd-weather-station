# Specification: hourly-forecast-chart_20260711

## Overview
Add a dedicated "Hourly" weather forecast view utilizing LVGL's native chart widget (`lv_chart`). This displays a curve of temperature and precipitation probability trends over the next 24 hours.

## Functional Requirements
1. **API Parameter Expansion**: Request `hourly` parameters for temperature and rain/precipitation from weather APIs.
2. **Data Model Updates**: Parse and store a 24-element hourly forecast list inside `WeatherData`.
3. **Hourly Tab**: Insert a third tab between "Forecast" and "Settings" tabs in the UI.
4. **LVGL Chart Widget**: Create a Catppuccin-styled `lv_chart` with x/y axes, grid lines, and a smooth curve series mapping the temperature.

## Acceptance Criteria
- Swipe navigation works smoothly to slide into the new Hourly tab.
- The chart draws a clean line reflecting local temperature variations.
- Adapts color accents automatically when the Catppuccin theme flavor is changed.
