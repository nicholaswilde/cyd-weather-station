#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Location Settings
#define USE_ZIP_CODE true
#define WEATHER_ZIP_CODE "92692"

// Coordinates (Fallback if USE_ZIP_CODE is false)
#define WEATHER_API_LATITUDE "37.7749"
#define WEATHER_API_LONGITUDE "-122.4194"

// Open-Meteo API Settings
#define WEATHER_UPDATE_INTERVAL_MINS 1

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

// SD Card Weather Logging Settings
#define USE_SD_LOGGING true

// Display Performance Settings
// DISPLAY_DRAW_BUF_ROWS: Height of the LVGL draw buffer in pixel rows.
//   Larger = fewer SPI flush calls per frame (smoother animation) but longer
//   CPU blocks per flush (less touch responsiveness). Tune for your hardware:
//     10 rows → ~24 flushes/frame, ~0.9 ms blocked each (choppy)
//     20 rows → ~12 flushes/frame, ~1.85 ms blocked each
//     30 rows → ~8 flushes/frame,  ~2.8 ms blocked each (default, sweet spot)
//     40 rows → ~6 flushes/frame,  ~7 ms blocked each (unresponsive)
#define DISPLAY_DRAW_BUF_ROWS 30

// DISPLAY_REFR_PERIOD_MS: How often LVGL redraws changed areas (milliseconds).
//   Lower = higher frame rate ceiling but more CPU time spent flushing.
//   Recommended range: 10–30 ms. Must be >= loop tick interval (5 ms).
//   NOTE: lv_conf.h is compiled by LVGL before this file is in scope, so
//   LV_DISP_DEF_REFR_PERIOD in include/lv_conf.h must be updated manually
//   to match this value.
#define DISPLAY_REFR_PERIOD_MS 20

// DISPLAY_INDEV_READ_PERIOD_MS: How often LVGL polls the touchscreen (milliseconds).
//   Lower = more responsive touch input. Keep at or below DISPLAY_REFR_PERIOD_MS.
//   NOTE: LV_INDEV_DEF_READ_PERIOD in include/lv_conf.h must also be updated
//   manually to match this value.
#define DISPLAY_INDEV_READ_PERIOD_MS 10

// DISPLAY_SWIPE_ANIM_MS: Duration of the tab-switch swipe animation (milliseconds).
//   Lower = snappier transitions. LVGL default is 300 ms.
#define DISPLAY_SWIPE_ANIM_MS 150

// SD Card SPI Settings
#define SD_CS_PIN 5
#define SD_SCK_PIN 18
#define SD_MISO_PIN 19
#define SD_MOSI_PIN 23

// Physical BOOT Button Settings
#define BOOT_BUTTON_PIN 0

#endif // CONFIG_H
