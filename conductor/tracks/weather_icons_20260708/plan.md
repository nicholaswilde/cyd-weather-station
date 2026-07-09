# Implementation Plan - Weather Status Icons Integration

## Phase 1: Font Acquisition & CLI Conversion [checkpoint: bc86752]
- [x] Task: Download Erik Flowers' Weather Icons TTF file and convert to LVGL C file (b210df1)
    - [ ] Download TTF font from erikflowers weather-icons GitHub raw URL.
    - [ ] Run `npx lv_font_conv` to generate the 48px C font file (`src/weather_icons_48.c`).
    - [ ] Integrate the font into the PlatformIO build system.
    - [ ] Verify compilation with a mock font declaration in the build.
- [x] Task: Conductor - User Manual Verification 'Phase 1: Font Acquisition & CLI Conversion' (Protocol in workflow.md) bc86752

## Phase 2: UI Icon Mapping and Layout Update
- [ ] Task: Update data model and integrate weather icons in UI
    - [ ] Update `WeatherData` struct in `include/weather_client.h` to include `weatherCode`.
    - [ ] Update `updateWeatherUI` signature in `include/ui.h` and its definition in `src/ui.cpp`.
    - [ ] Implement mapping logic in `src/ui.cpp` to map Open-Meteo codes to glyph unicode strings and Catppuccin hex colors.
    - [ ] Update card layout in `src/ui.cpp` to place the weather icon (48px) on the left, and shift temperature, humidity, and status text to the right.
    - [ ] Run native tests `pio test -e native` to verify all components compile and tests pass.
- [ ] Task: Conductor - User Manual Verification 'Phase 2: UI Icon Mapping and Layout Update' (Protocol in workflow.md)
