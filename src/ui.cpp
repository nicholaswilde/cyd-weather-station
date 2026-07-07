#include "ui.h"
#include <Arduino.h>

static lv_obj_t *wifi_label;
static lv_obj_t *test_label;

static void btn_event_cb(lv_event_t * e) {
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_CLICKED) {
        Serial.println("[UI] Button Clicked!");
        lv_label_set_text(test_label, "Touch OK!");
    }
}

void initUI() {
    // Main screen setup (light grey background)
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xF0F0F0), LV_PART_MAIN);

    // 1. Header Bar Container
    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, 320, 40);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(0x2196F3), LV_PART_MAIN); // Blue header
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Header Title (Time/App Name)
    lv_obj_t * title = lv_label_create(header);
    lv_label_set_text(title, "CYD Weather Station");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_LEFT_MID, 10, 0);

    // Wi-Fi Label in Header
    wifi_label = lv_label_create(header);
    lv_label_set_text(wifi_label, "WiFi: Offline");
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xFFC107), LV_PART_MAIN); // Amber text
    lv_obj_align(wifi_label, LV_ALIGN_RIGHT_MID, -10, 0);

    // 2. Main Content Card
    lv_obj_t * card = lv_obj_create(scr);
    lv_obj_set_size(card, 300, 180);
    lv_obj_align(scr, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_align_to(card, header, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_radius(card, 10, LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 1, LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(0xE0E0E0), LV_PART_MAIN);
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);

    // Weather Placeholders inside card
    lv_obj_t * temp_label = lv_label_create(card);
    lv_label_set_text(temp_label, "--.- *C");
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_align(temp_label, LV_ALIGN_TOP_LEFT, 15, 15);

    lv_obj_t * hum_label = lv_label_create(card);
    lv_label_set_text(hum_label, "Humidity: --%");
    lv_obj_align_to(hum_label, temp_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    lv_obj_t * status_lbl = lv_label_create(card);
    lv_label_set_text(status_lbl, "Waiting for API update...");
    lv_obj_align_to(status_lbl, hum_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);

    // 3. Touch Test Button
    lv_obj_t * btn = lv_btn_create(card);
    lv_obj_set_size(btn, 100, 40);
    lv_obj_align(btn, LV_ALIGN_BOTTOM_RIGHT, -15, -15);
    lv_obj_add_event_cb(btn, btn_event_cb, LV_EVENT_ALL, NULL);

    lv_obj_t * btn_label = lv_label_create(btn);
    lv_label_set_text(btn_label, "Test Touch");
    lv_obj_center(btn_label);

    // Test result feedback label
    test_label = lv_label_create(card);
    lv_label_set_text(test_label, "Touch to test");
    lv_obj_align_to(test_label, btn, LV_ALIGN_OUT_LEFT_MID, -15, 0);
}

void updateWifiStatus(bool connected) {
    if (connected) {
        lv_label_set_text(wifi_label, "WiFi: OK");
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0x4CAF50), LV_PART_MAIN); // Green
    } else {
        lv_label_set_text(wifi_label, "WiFi: Lost");
        lv_obj_set_style_text_color(wifi_label, lv_color_hex(0xF44336), LV_PART_MAIN); // Red
    }
}
