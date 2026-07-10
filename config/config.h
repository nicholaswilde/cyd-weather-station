#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Location Settings
#define USE_ZIP_CODE true
#define WEATHER_ZIP_CODE "90210"

// Coordinates (Fallback if USE_ZIP_CODE is false)
#define WEATHER_API_LATITUDE "37.7749"
#define WEATHER_API_LONGITUDE "-122.4194"

// Open-Meteo API Settings
#define WEATHER_UPDATE_INTERVAL_MINS 15

// NTP and Timezone Settings
#define NTP_SERVER "pool.ntp.org"
#define GMT_OFFSET_SEC (-8 * 3600)  // GMT -8 (PST)
#define DST_OFFSET_SEC 3600         // Daylight Savings offset (1 hour)

#include "catppuccin.h"

extern int getCurrentThemeFlavor();

#define COLOR_BASE         (getCatppuccinFlavor(getCurrentThemeFlavor()).base)
#define COLOR_MANTLE       (getCatppuccinFlavor(getCurrentThemeFlavor()).mantle)
#define COLOR_CRUST        (getCatppuccinFlavor(getCurrentThemeFlavor()).crust)
#define COLOR_TEXT         (getCatppuccinFlavor(getCurrentThemeFlavor()).text)
#define COLOR_OVERLAY      (getCatppuccinFlavor(getCurrentThemeFlavor()).overlay)
#define COLOR_BLUE         (getCatppuccinFlavor(getCurrentThemeFlavor()).blue)
#define COLOR_GREEN        (getCatppuccinFlavor(getCurrentThemeFlavor()).green)
#define COLOR_RED          (getCatppuccinFlavor(getCurrentThemeFlavor()).red)
#define COLOR_YELLOW       (getCatppuccinFlavor(getCurrentThemeFlavor()).yellow)
#define COLOR_PEACH        (getCatppuccinFlavor(getCurrentThemeFlavor()).peach)
#define COLOR_MAUVE        (getCatppuccinFlavor(getCurrentThemeFlavor()).mauve)
#define COLOR_LAVENDER     (getCatppuccinFlavor(getCurrentThemeFlavor()).lavender)
#define COLOR_HEADER_TEXT  (getCatppuccinFlavor(getCurrentThemeFlavor()).header_text)

// Unit System Settings
#define UNIT_METRIC   1
#define UNIT_IMPERIAL 2

// Choose Unit System (UNIT_METRIC or UNIT_IMPERIAL)
// #define UNIT_SYSTEM UNIT_METRIC
#define UNIT_SYSTEM UNIT_IMPERIAL

// Auto Backlight Settings
#define USE_LDR_AUTO_BACKLIGHT false

// RGB LED Status Settings
#define USE_RGB_LED_STATUS true

#endif // CONFIG_H
