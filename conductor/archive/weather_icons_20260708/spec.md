# Specification - Weather Status Icons Integration

## Overview
Integrate Erik Flowers' Weather Icons font into the ESP32 CYD Weather Station application to display a large (48px) weather symbol next to the temperature and humidity reading. The symbol will be colored dynamically according to the Catppuccin style guide (e.g. Yellow/Peach for clear sky, Blue/Sapphire for rain, Lavender for snow).

## Functional Requirements
1. **Font Conversion**: Convert Erik Flowers' Weather Icons TTF font to an LVGL-compatible C font file (`weather_icons_48.c`) at size 48px, containing Unicode range `0xf000-0xf0ff`.
2. **UI Integration**:
   - Create an `lv_label` widget on the left side of the main content card to display the weather icon.
   - Align the temperature, humidity, and status description text to the right of the weather icon.
   - Map weather codes from the Open-Meteo API to the corresponding Weather Icons glyphs (Private Use Area Unicode values).
   - Style the weather icon dynamically based on the current weather condition using the Catppuccin color palette:
     - Clear/Sunny (Code 0, 1): Yellow/Peach accent (`COLOR_YELLOW` / `COLOR_PEACH`)
     - Clouds/Overcast/Fog (Code 2, 3, 45, 48): Lavender/Subtext accent (`COLOR_LAVENDER` / `COLOR_TEXT`)
     - Drizzle/Rain (Codes 51-65, 80-82): Blue/Sapphire accent (`COLOR_BLUE` / `COLOR_TEXT`)
     - Snow/Freezing Rain/Snow Grains (Codes 56, 57, 66, 67, 71-77, 85, 86): Sky/Lavender accent (`COLOR_BLUE` / `COLOR_LAVENDER`)
     - Thunderstorm (Codes 95-99): Mauve accent (`COLOR_MAUVE`)
3. **Data Model Update**: Update `WeatherData` struct in `weather_client.h` and the UI update function to pass the weather code from the client to the UI.
4. **Fallback Behavior**: If a weather code is unrecognized or invalid, show a fallback icon (e.g., question/NA icon) colored with `COLOR_OVERLAY`.

## Non-Functional Requirements
- Memory footprint must remain low (compressed 48px font containing ~256 characters should be under 15KB).
- Fast rendering on the ESP32 screen without flicker.

## Acceptance Criteria
- Main screen displays a 48px weather symbol next to weather data.
- The symbol changes dynamically based on the Open-Meteo weather code.
- The symbol uses correct Catppuccin colors matching the weather type.
- Unit tests verify the weather code mapping and UI update logic.

## Out of Scope
- Adding animated Lottie/JSON weather icons.
- Fetching historical weather icons.
