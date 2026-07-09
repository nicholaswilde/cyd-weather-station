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

// Catppuccin Flavor Definitions
#define CATPPUCCIN_MOCHA      1
#define CATPPUCCIN_MACCHIATO  2
#define CATPPUCCIN_FRAPPE     3
#define CATPPUCCIN_LATTE      4

// Choose Catppuccin Flavor (MOCHA, MACCHIATO, FRAPPE, or LATTE)
#define CATPPUCCIN_FLAVOR CATPPUCCIN_MOCHA

#if CATPPUCCIN_FLAVOR == CATPPUCCIN_MOCHA
  #define COLOR_BASE         0x1e1e2e
  #define COLOR_MANTLE       0x181825
  #define COLOR_CRUST        0x11111b
  #define COLOR_TEXT         0xcdd6f4
  #define COLOR_OVERLAY      0x6c7086
  #define COLOR_BLUE         0x89b4fa
  #define COLOR_GREEN        0xa6e3a1
  #define COLOR_RED          0xf38ba8
  #define COLOR_YELLOW       0xf9e2af
  #define COLOR_PEACH        0xfab387
  #define COLOR_MAUVE        0xcba6f7
  #define COLOR_LAVENDER     0xb4befe
  #define COLOR_HEADER_TEXT  0xcdd6f4
#elif CATPPUCCIN_FLAVOR == CATPPUCCIN_MACCHIATO
  #define COLOR_BASE         0x24273a
  #define COLOR_MANTLE       0x1e2030
  #define COLOR_CRUST        0x181926
  #define COLOR_TEXT         0xcad3f5
  #define COLOR_OVERLAY      0x6e738d
  #define COLOR_BLUE         0x8aadf4
  #define COLOR_GREEN        0xa6da95
  #define COLOR_RED          0xed8796
  #define COLOR_YELLOW       0xeed49f
  #define COLOR_PEACH        0xf5a97f
  #define COLOR_MAUVE        0xc6a0f6
  #define COLOR_LAVENDER     0xb7bdf8
  #define COLOR_HEADER_TEXT  0xcad3f5
#elif CATPPUCCIN_FLAVOR == CATPPUCCIN_FRAPPE
  #define COLOR_BASE         0x303446
  #define COLOR_MANTLE       0x292c3c
  #define COLOR_CRUST        0x232634
  #define COLOR_TEXT         0xc6d0f5
  #define COLOR_OVERLAY      0x737994
  #define COLOR_BLUE         0x8caaee
  #define COLOR_GREEN        0xa6d189
  #define COLOR_RED          0xe78284
  #define COLOR_YELLOW       0xe5c890
  #define COLOR_PEACH        0xef9f76
  #define COLOR_MAUVE        0xca9ee6
  #define COLOR_LAVENDER     0xbabbf1
  #define COLOR_HEADER_TEXT  0xc6d0f5
#elif CATPPUCCIN_FLAVOR == CATPPUCCIN_LATTE
  #define COLOR_BASE         0xeff1f5
  #define COLOR_MANTLE       0xe6e9ef
  #define COLOR_CRUST        0xdce0e8
  #define COLOR_TEXT         0x4c4f69
  #define COLOR_OVERLAY      0x9ca0b0
  #define COLOR_BLUE         0x1e66f5
  #define COLOR_GREEN        0x40a02b
  #define COLOR_RED          0xd20f39
  #define COLOR_YELLOW       0xdf8e1d
  #define COLOR_PEACH        0xfe640b
  #define COLOR_MAUVE        0x8839ef
  #define COLOR_LAVENDER     0x7287fd
  #define COLOR_HEADER_TEXT  0x4c4f69
#endif

// Unit System Settings
#define UNIT_METRIC   1
#define UNIT_IMPERIAL 2

// Choose Unit System (UNIT_METRIC or UNIT_IMPERIAL)
// #define UNIT_SYSTEM UNIT_METRIC
#define UNIT_SYSTEM UNIT_IMPERIAL

// Auto Backlight Settings
#define USE_LDR_AUTO_BACKLIGHT true

// RGB LED Status Settings
#define USE_RGB_LED_STATUS true

#endif // CONFIG_H
