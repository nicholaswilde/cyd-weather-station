# Specification: Add Wind Speed to Display (`add_wind_speed_20260709`)

## Overview
Add a wind speed metric reading onto the CYD display screen, complete with a dedicated weather icon and automatic formatting based on the selected unit system.

## Functional Requirements
1. **API Integration**:
   - Fetch wind speed from the Open-Meteo current weather API. In the JSON response, this corresponds to `current_weather.windspeed`.
   - Update `WeatherData` struct in `include/weather_client.h` to hold a `windSpeed` float variable.
   - Update `weather_client.cpp` parsing to extract `windspeed`.
2. **Display Placement & UI**:
   - Create a new LVGL label `wind_label` inside the weather card.
   - Position the wind speed reading below the humidity reading inside the main weather card.
   - Display a 48px wind icon next to the wind speed text or display it inline using the `wi-windy` glyph (Unicode `0xf021`).
3. **Unit System Formatting**:
   - If `UNIT_SYSTEM` is `UNIT_METRIC`, display wind speed in `km/h` (e.g. "Wind: 14.2 km/h").
   - If `UNIT_SYSTEM` is `UNIT_IMPERIAL`, display wind speed in `mph` (e.g. "Wind: 8.8 mph").
4. **Font Asset Regeneration**:
   - Extend the list of character ranges in the `package.json` font generator script to include `0xf021` (`wi-windy`).
   - Run the generator to update `src/weather_icons_48.c`.

## Non-Functional Requirements
- Ensure responsive rendering and proper spacing on the 320x240 display card.
- Native tests must mock the updated weather client response containing the wind speed field.

## Acceptance Criteria
- [ ] Wind speed is parsed correctly from the Open-Meteo API response.
- [ ] Unit formatting adjusts dynamically to km/h or mph based on config.h.
- [ ] The custom wind icon (Unicode 0xf021) is visible on the screen.
- [ ] Layout is clean, responsive, and does not overlap other widgets.
- [ ] Unit tests are updated to verify parsing of wind speed.
