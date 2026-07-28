#ifndef CONFIG_H
#define CONFIG_H

#include "secrets.h"

// Location Settings
// #define WEATHER_ZIP_CODE "90210"
#define WEATHER_ZIP_CODE ""

// Coordinates (Fallback if USE_ZIP_CODE is false)
// #define WEATHER_API_LATITUDE "34.10313"
#define WEATHER_API_LATITUDE ""
// #define WEATHER_API_LONGITUDE "-118.41625"
#define WEATHER_API_LONGITUDE ""

// Open-Meteo API Settings
#define WEATHER_UPDATE_INTERVAL_MINS 1

// NTP and Timezone Settings
// Note: We use POSIX strings for efficiency to avoid bundling a giant tzdata lookup table.
// Find your region's POSIX string here: https://gist.github.com/alwynallan/24d96091655391107939
#define NTP_SERVER "pool.ntp.org"
// #define TIMEZONE_DEFAULT "UTC0"       // POSIX Timezone string

// US Eastern
#define TIMEZONE_DEFAULT "EST5EDT,M3.2.0,M11.1.0"
// US Central
// #define TIMEZONE_DEFAULT "CST6CDT,M3.2.0,M11.1.0"
// US Mountain
// #define TIMEZONE_DEFAULT "MST7MDT,M3.2.0,M11.1.0"
// US Pacific
// #define TIMEZONE_DEFAULT "PST8PDT,M3.2.0,M11.1.0"
// London
// #define TIMEZONE_DEFAULT "GMT0BST,M3.5.0/1,M10.5.0"
// CET
// #define TIMEZONE_DEFAULT "CET-1CEST,M3.5.0,M10.5.0/3"

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

// Screensaver Settings
#define SCREENSAVER_ENABLED false
#define SCREENSAVER_TIMEOUT_MS 300000 // 5 minutes (in milliseconds)

// RGB LED Status Settings
#define USE_RGB_LED_STATUS true

// LED Enable / Brightness Defaults
// LED_ENABLED: Set to false to disable the RGB status LED by default.
#define LED_ENABLED true
// LED_BRIGHTNESS: Default LED brightness (0–255). Full brightness = 255.
#define LED_BRIGHTNESS 255

// MQTT default status setting
#define MQTT_ENABLED false

// API Server default status setting
#define API_SERVER_ENABLED false

// SD Card Weather Logging Settings
#define USE_SD_LOGGING false
#define USE_SD_CACHE false

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

// Static IP Settings
// Uncomment the lines below to configure a static IP address.
// If commented out, the device will use DHCP.
// #define STATIC_IP          "192.168.1.100"
// #define STATIC_GATEWAY     "192.168.1.1"
// #define STATIC_SUBNET      "255.255.255.0"
// #define STATIC_DNS         "1.1.1.1"

#endif // CONFIG_H
