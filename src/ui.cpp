#include "ui.h"
#include <Arduino.h>
#include "config/config.h"

extern "C" {
LV_FONT_DECLARE(weather_icons_48);
}

static lv_obj_t *wifi_label;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *status_lbl;
static lv_obj_t *time_label;
static lv_obj_t *icon_lbl;
static lv_obj_t *wind_label;

void initUI() {
    // Main screen setup (light grey background -> Catppuccin Base)
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BASE), LV_PART_MAIN);

    // 1. Header Bar Container
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, 320, 45);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_CRUST), LV_PART_MAIN); // Crust header background
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Header Title (Time/App Name)
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "CYD Weather Station");
    lv_obj_set_style_text_color(title, lv_color_hex(COLOR_HEADER_TEXT), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);

    // Wi-Fi Label in Header
    wifi_label = lv_label_create(header);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN); // Yellow / Amber icon
    lv_obj_align(wifi_label, LV_ALIGN_RIGHT_MID, -5, 0);

    // Time Label in Header
    time_label = lv_label_create(header);
    lv_label_set_text(time_label, "--:--");
    lv_obj_set_style_text_color(time_label, lv_color_hex(COLOR_HEADER_TEXT), LV_PART_MAIN);
    lv_obj_align_to(time_label, wifi_label, LV_ALIGN_OUT_LEFT_MID, -15, 0);

    // 2. Main Content Card
    lv_obj_t * card = lv_obj_create(scr);
    lv_obj_set_size(card, 300, 180);
    lv_obj_align(scr, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_align_to(card, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(card, lv_color_hex(COLOR_MANTLE), LV_PART_MAIN); // Mantle background
    lv_obj_set_style_radius(card, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN); // Overlay border
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Weather Placeholders inside card
    icon_lbl = lv_label_create(card);
    lv_obj_set_style_text_font(icon_lbl, &weather_icons_48, LV_PART_MAIN);
    lv_label_set_text(icon_lbl, "\xef\x81\xbb"); // fallback NA icon (f07b)
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_align(icon_lbl, LV_ALIGN_LEFT_MID, 20, 0);

    temp_label = lv_label_create(card);
#if UNIT_SYSTEM == UNIT_IMPERIAL
    lv_label_set_text(temp_label, "--.- °F");
#else
    lv_label_set_text(temp_label, "--.- °C");
#endif
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_align_to(temp_label, icon_lbl, LV_ALIGN_OUT_RIGHT_TOP, 25, -5);

    hum_label = lv_label_create(card);
    lv_label_set_text(hum_label, "Humidity: --%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
    lv_obj_align_to(hum_label, temp_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    wind_label = lv_label_create(card);
    lv_label_set_text(wind_label, "Wind: -- km/h");
    lv_obj_set_style_text_color(wind_label, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);
    lv_obj_align_to(wind_label, hum_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 22);

    lv_obj_t * wind_icon_lbl = lv_label_create(card);
    lv_obj_set_style_text_font(wind_icon_lbl, &weather_icons_48, LV_PART_MAIN);
    lv_label_set_text(wind_icon_lbl, "\xef\x80\xa1"); // U+F021 (wi-windy)
    lv_obj_set_style_text_color(wind_icon_lbl, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);
    lv_obj_align_to(wind_icon_lbl, wind_label, LV_ALIGN_OUT_LEFT_MID, -25, 0);

    status_lbl = lv_label_create(card);
    lv_label_set_text(status_lbl, "Waiting for API update...");
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
    lv_obj_align_to(status_lbl, hum_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 56);
}

void updateWifiStatus(bool connected) {
    if (connected) {
        lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_GREEN), LV_PART_MAIN); // Catppuccin Green
    } else {
        lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_RED), LV_PART_MAIN); // Catppuccin Red
    }
}

static const char* getIconGlyph(int code) {
    switch (code) {
        case 0:
        case 1:
            return "\xef\x80\x8d"; // wi-day-sunny (f00d)
        case 2:
        case 3:
            return "\xef\x80\x82"; // wi-cloudy (f002)
        case 45:
        case 48:
            return "\xef\x80\x83"; // wi-fog (f003)
        case 51:
        case 53:
        case 55:
        case 61:
        case 63:
        case 65:
        case 80:
        case 81:
        case 82:
            return "\xef\x80\x88"; // wi-rain (f008)
        case 56:
        case 57:
        case 66:
        case 67:
        case 71:
        case 73:
        case 75:
        case 77:
        case 85:
        case 86:
            return "\xef\x80\x8a"; // wi-snow (f00a)
        case 95:
        case 96:
        case 99:
            return "\xef\x80\x90"; // wi-thunderstorm (f010)
        default:
            return "\xef\x81\xbb"; // wi-na (f07b)
    }
}

static uint32_t getIconColor(int code) {
    switch (code) {
        case 0:
        case 1:
            return COLOR_YELLOW;
        case 2:
        case 3:
        case 45:
        case 48:
            return COLOR_LAVENDER;
        case 51:
        case 53:
        case 55:
        case 61:
        case 63:
        case 65:
        case 80:
        case 81:
        case 82:
            return COLOR_BLUE;
        case 56:
        case 57:
        case 66:
        case 67:
        case 71:
        case 73:
        case 75:
        case 77:
        case 85:
        case 86:
            return COLOR_LAVENDER;
        case 95:
        case 96:
        case 99:
            return COLOR_MAUVE;
        default:
            return COLOR_OVERLAY;
    }
}

void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed) {
    char temp_str[32];
#if UNIT_SYSTEM == UNIT_IMPERIAL
    snprintf(temp_str, sizeof(temp_str), "%.1f °F", temperature);
#else
    snprintf(temp_str, sizeof(temp_str), "%.1f °C", temperature);
#endif
    lv_label_set_text(temp_label, temp_str);

    char hum_str[32];
    snprintf(hum_str, sizeof(hum_str), "Humidity: %d%%", humidity);
    lv_label_set_text(hum_label, hum_str);

    char wind_str[32];
#if UNIT_SYSTEM == UNIT_IMPERIAL
    snprintf(wind_str, sizeof(wind_str), "Wind: %.1f mph", windSpeed);
#else
    snprintf(wind_str, sizeof(wind_str), "Wind: %.1f km/h", windSpeed);
#endif
    lv_label_set_text(wind_label, wind_str);

    lv_label_set_text(status_lbl, status);

    // Update weather icon and its color
    lv_label_set_text(icon_lbl, getIconGlyph(weatherCode));
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(getIconColor(weatherCode)), LV_PART_MAIN);
}

void updateTimeUI(const char* time_str) {
    lv_label_set_text(time_label, time_str);
}
