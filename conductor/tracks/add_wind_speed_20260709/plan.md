# Implementation Plan - Add Wind Speed to Display (`add_wind_speed_20260709`)

## Phase 1: Font Regeneration & API Integration [checkpoint: e88ce27]
- [x] Task: Regenerate weather icons font to include the windy glyph (34d2cd2)
    - [x] Add `0xf021` to the font generator script in `package.json`
    - [x] Run `task setup:node` and `task font:generate` to rebuild `src/weather_icons_48.c`
- [x] Task: Update WeatherClient data model and parsing (a3e472a)
    - [x] Add `windSpeed` float to `WeatherData` struct in `include/weather_client.h`
    - [x] Add failing unit tests in `tests/test_weather_client` verifying parsing of wind speed
    - [x] Update `src/weather_client.cpp` to parse `windspeed` from the Open-Meteo JSON response
    - [x] Verify all unit tests pass successfully
- [x] Task: Conductor - User Manual Verification 'Phase 1: Font Regeneration & API Integration' (Protocol in workflow.md) (e88ce27)

## Phase 2: UI Integration & Layout
- [~] Task: Implement wind speed UI layout and formatting
    - [ ] Create `wind_label` in `src/ui.cpp` and position it below the humidity reading inside the weather card
    - [ ] Declare a small windy icon inline or next to the label using unicode `0xf021`
    - [ ] Update `updateWeatherUI()` in `include/ui.h` and `src/ui.cpp` to accept `windSpeed` parameter
    - [ ] Format wind speed dynamically based on `UNIT_SYSTEM` (mph vs km/h)
    - [ ] Update main application loop in `src/main.cpp` to pass wind speed to `updateWeatherUI()`
    - [ ] Verify test coverage and run lint checks via `task check`
- [ ] Task: Conductor - User Manual Verification 'Phase 2: UI Integration & Layout' (Protocol in workflow.md)
