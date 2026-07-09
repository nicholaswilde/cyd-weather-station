# Specification: Multi-Screen Tabview & Weather Forecast (`multi_screen_forecast_20260708`)

## Overview
Enhance the user interface using an LVGL Tabview to easily swipe/navigate between multiple screens. Implement a forecast screen that fetches and displays a 3-day weather forecast (including temperature highs/lows and weather descriptions) from the Open-Meteo API.

## Functional Requirements
1. **LVGL UI Tabview Integration**:
   - Replace the static single-screen layout with an `lv_tabview` component.
   - Configure tabs at the top or bottom of the screen (sized appropriately for the 320x240 display).
   - Create two tabs:
     - **Current**: Shows the current weather details (transferred from the current layout).
     - **Forecast**: Displays a 3-day weather forecast (today, tomorrow, day after).
2. **Forecast Data Fetching**:
   - Modify `WeatherClient` to request daily forecast data from the Open-Meteo API:
     - API parameter additions: `&daily=weather_code,temperature_2m_max,temperature_2m_min`
   - Extend `WeatherData` struct or create a separate forecast data structure to store:
     - Max/min temperatures for 3 days.
     - Weather codes (or mapped descriptions) for 3 days.
3. **Forecast Screen Layout**:
   - Layout the 3-day forecast clearly. A grid-like or list-like layout is recommended.
   - Each forecast entry should show:
     - Day name (e.g., "Today", "Thu", "Fri" or dates).
     - Min/Max temperature (formatted with correct units).
     - Brief weather condition text (e.g., "Rainy", "Sunny").

## Non-Functional Requirements
- **Display Constraint**: Keep elements clean and spaced nicely to fit on the 320x240 screen without clipping.
- **Parsing Memory**: Ensure ArduinoJson buffer sizes are large enough to parse the larger response body including forecast data, without causing heap exhaustion.

## Acceptance Criteria
- [ ] UI is successfully refactored to use a Tabview; swipe gestures or tab selections successfully change views.
- [ ] The Open-Meteo API request fetches 3-day daily forecast data.
- [ ] Forecast tab displays max/min temperatures and descriptions correctly.
- [ ] Unit tests are written/updated for `WeatherClient` to verify parsing of the extended JSON payload.
- [ ] Compilation is clean and check/linter passes (`task check`).

## Out of Scope
- Interactive customization of forecast period length (fixed to 3 days).
- Custom icons or graphics files (reserving standard symbols or text descriptions for simplicity and flash constraints).
