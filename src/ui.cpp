#include "ui.h"
#include <Arduino.h>
#include "config/config.h"

static lv_obj_t *wifi_label;
static lv_obj_t *test_label;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *status_lbl;
static lv_obj_t *time_label;

static void btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        Serial.println("[UI] Button Clicked!");
        lv_label_set_text(test_label, "Touch OK!");
    }
}

void initUI() {
    // Main screen setup (light grey background -> Catppuccin Base)
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BASE), LV_PART_MAIN);

    // 1. Header Bar Container
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, 320, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_BLUE), LV_PART_MAIN); // Catppuccin Blue header
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
    temp_label = lv_label_create(card);
#if UNIT_SYSTEM == UNIT_IMPERIAL
    lv_label_set_text(temp_label, "--.- °F");
#else
    lv_label_set_text(temp_label, "--.- °C");
#endif
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 15, 15);

    hum_label = lv_label_create(card);
    lv_label_set_text(hum_label, "Humidity: --%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
    lv_obj_align_to(hum_label, temp_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    status_lbl = lv_label_create(card);
    lv_label_set_text(status_lbl, "Waiting for API update...");
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
    lv_obj_align_to(status_lbl, hum_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    // 3. Touch Test Button
    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -15, -15);
    lv_obj_set_style_bg_color(btn, lv_color_hex(COLOR_BLUE), LV_PART_MAIN); // Blue button background
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Test Touch");
    lv_obj_set_style_text_color(btn_label, lv_color_hex(COLOR_HEADER_TEXT), LV_PART_MAIN); // Contrast text
    lv_obj_center(btn_label);

    // Test result feedback label
    test_label = lv_label_create(card);
    lv_label_set_text(test_label, "Touch to test");
    lv_obj_set_style_text_color(test_label, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_align_to(test_label, btn, LV_ALIGN_OUT_LEFT_MID, -15, 0);
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

void updateWeatherUI(float temperature, int humidity, const char* status) {
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

    lv_label_set_text(status_lbl, status);
}

void updateTimeUI(const char* time_str) {
    lv_label_set_text(time_label, time_str);
}
