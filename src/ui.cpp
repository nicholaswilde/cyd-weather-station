#include "ui.h"
#include <Arduino.h>
#include "config/config.h"
#include "settings_manager.h"

extern "C" {
LV_FONT_DECLARE(weather_icons_48);
LV_FONT_DECLARE(weather_icons_24);
}

extern SettingsManager settings;

volatile bool settings_unit_changed = false;
volatile bool settings_brightness_changed = false;
volatile bool settings_timezone_changed = false;
volatile bool settings_theme_changed = false;

static lv_obj_t *wifi_label;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *status_lbl;
static lv_obj_t *time_label;
static lv_obj_t *icon_lbl;
static lv_obj_t *wind_label;
static lv_obj_t *tz_val_label;

// Forecast widgets
static lv_obj_t *fore_day_label[3];
static lv_obj_t *fore_icon_label[3];
static lv_obj_t *fore_temp_label[3];
static lv_obj_t *fore_desc_label[3];

static void theme_dropdown_event_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dropdown);
    int flavor = selected + 1; // 1-based (Mocha=1, Macchiato=2, Frappe=3, Latte=4)
    if (flavor != settings.getThemeFlavor()) {
        settings.setThemeFlavor(flavor);
        settings_theme_changed = true;
    }
}

static void unit_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    int unit = is_checked ? UNIT_IMPERIAL : UNIT_METRIC;
    settings.setUnitSystem(unit);
    settings_unit_changed = true;
}

static void auto_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setAutoBrightness(is_checked);
    settings_brightness_changed = true;
    
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
    if (slider) {
        if (is_checked) {
            lv_obj_add_state(slider, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(slider, LV_STATE_DISABLED);
        }
    }
}

static void brightness_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    settings.setBrightness(val);
    settings_brightness_changed = true;
    
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
    if (label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "Bright: %d%%", val);
        lv_label_set_text(label, buf);
    }
}

static void tz_btn_event_cb(lv_event_t * e) {
    intptr_t dir = (intptr_t)lv_event_get_user_data(e);
    int current_offset = settings.getTimezoneOffset();
    int new_offset = current_offset + dir;
    if (new_offset < -12) new_offset = -12;
    if (new_offset > 14) new_offset = 14;
    
    if (new_offset != current_offset) {
        settings.setTimezoneOffset(new_offset);
        settings_timezone_changed = true;
        
        char buf[32];
        if (new_offset >= 0) {
            snprintf(buf, sizeof(buf), "GMT +%d", new_offset);
        } else {
            snprintf(buf, sizeof(buf), "GMT %d", new_offset);
        }
        lv_label_set_text(tz_val_label, buf);
    }
}


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

    // 2. Tabview Setup
    lv_obj_t * tabview = lv_tabview_create(scr, LV_DIR_BOTTOM, 35);
    lv_obj_set_size(tabview, 320, 195);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Style the tabview container and buttons
    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    
    // Style the buttons
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(COLOR_BASE), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(COLOR_PEACH), LV_PART_ITEMS | LV_STATE_CHECKED);

    // Create the tabs
    lv_obj_t * tab_curr = lv_tabview_add_tab(tabview, "Current");
    lv_obj_t * tab_fore = lv_tabview_add_tab(tabview, "Forecast");
    lv_obj_t * tab_settings = lv_tabview_add_tab(tabview, "Settings");

    // Clear scroll on tabs and set base color & padding
    lv_obj_clear_flag(tab_curr, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab_fore, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(tab_settings, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_color(tab_curr, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_fore, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_settings, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_curr, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_fore, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_settings, 5, LV_PART_MAIN);

    // Weather Placeholders inside tab_curr
    icon_lbl = lv_label_create(tab_curr);
    lv_obj_set_style_text_font(icon_lbl, &weather_icons_48, LV_PART_MAIN);
    lv_label_set_text(icon_lbl, "\xef\x81\xbb"); // fallback NA icon (f07b)
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_align(icon_lbl, LV_ALIGN_LEFT_MID, 25, 0);
 
    // Vertical container for details on the right side
    lv_obj_t * details_cnt = lv_obj_create(tab_curr);
    lv_obj_set_size(details_cnt, 200, 150);
    lv_obj_align(details_cnt, LV_ALIGN_RIGHT_MID, -10, 0);
    lv_obj_set_flex_flow(details_cnt, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(details_cnt, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Remove styling constraints and scrollbars
    lv_obj_set_style_bg_opa(details_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(details_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(details_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(details_cnt, 4, LV_PART_MAIN); // vertical space between children
    lv_obj_clear_flag(details_cnt, LV_OBJ_FLAG_SCROLLABLE);

    temp_label = lv_label_create(details_cnt);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        lv_label_set_text(temp_label, "--.- °F");
    } else {
        lv_label_set_text(temp_label, "--.- °C");
    }
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);

    hum_label = lv_label_create(details_cnt);
    lv_label_set_text(hum_label, "Humidity: --%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);

    // Row container for wind icon and label
    lv_obj_t * wind_cnt = lv_obj_create(details_cnt);
    lv_obj_set_size(wind_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(wind_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wind_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(wind_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(wind_cnt, 6, LV_PART_MAIN); // horizontal space between icon and text
    lv_obj_clear_flag(wind_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * wind_icon_lbl = lv_label_create(wind_cnt);
    lv_obj_set_style_text_font(wind_icon_lbl, &weather_icons_24, LV_PART_MAIN);
    lv_label_set_text(wind_icon_lbl, "\xef\x80\xa1"); // U+F021 (wi-windy)
    lv_obj_set_style_text_color(wind_icon_lbl, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);

    wind_label = lv_label_create(wind_cnt);
    lv_label_set_text(wind_label, "Wind: -- km/h");
    lv_obj_set_style_text_color(wind_label, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);

    status_lbl = lv_label_create(details_cnt);
    lv_label_set_text(status_lbl, "Waiting for API update...");
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
    lv_obj_set_width(status_lbl, 190);

    // 3-day Forecast Card Layout inside tab_fore
    for (int i = 0; i < 3; i++) {
        lv_obj_t *day_card = lv_obj_create(tab_fore);
        lv_obj_set_size(day_card, 90, 130);
        if (i == 0) {
            lv_obj_align(day_card, LV_ALIGN_LEFT_MID, 10, 0);
        } else if (i == 1) {
            lv_obj_align(day_card, LV_ALIGN_CENTER, 0, 0);
        } else {
            lv_obj_align(day_card, LV_ALIGN_RIGHT_MID, -10, 0);
        }
        
        // Card styling: Catppuccin Mantle background
        lv_obj_set_style_bg_color(day_card, lv_color_hex(COLOR_MANTLE), LV_PART_MAIN);
        lv_obj_set_style_radius(day_card, 8, LV_PART_MAIN);
        lv_obj_set_style_border_width(day_card, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(day_card, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
        lv_obj_clear_flag(day_card, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_style_pad_all(day_card, 2, LV_PART_MAIN);

        // 1. Day Name Label
        fore_day_label[i] = lv_label_create(day_card);
        lv_label_set_text(fore_day_label[i], i == 0 ? "Today" : (i == 1 ? "Tomorrow" : "Day"));
        lv_obj_set_style_text_color(fore_day_label[i], lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
        lv_obj_align(fore_day_label[i], LV_ALIGN_TOP_MID, 0, 5);

        // 2. Weather Icon Label
        fore_icon_label[i] = lv_label_create(day_card);
        lv_obj_set_style_text_font(fore_icon_label[i], &weather_icons_24, LV_PART_MAIN);
        lv_label_set_text(fore_icon_label[i], "\xef\x81\xbb"); // fallback NA
        lv_obj_set_style_text_color(fore_icon_label[i], lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
        lv_obj_align(fore_icon_label[i], LV_ALIGN_CENTER, 0, -5);

        // 3. Temp Label (High / Low)
        fore_temp_label[i] = lv_label_create(day_card);
        lv_label_set_text(fore_temp_label[i], "--°/--°");
        lv_obj_set_style_text_color(fore_temp_label[i], lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
        lv_obj_align(fore_temp_label[i], LV_ALIGN_BOTTOM_MID, 0, -22);

        // 4. Status Description Label
        fore_desc_label[i] = lv_label_create(day_card);
        lv_label_set_text(fore_desc_label[i], "Loading...");
        lv_obj_set_style_text_color(fore_desc_label[i], lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
        lv_label_set_long_mode(fore_desc_label[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
        lv_obj_set_width(fore_desc_label[i], 80);
        lv_obj_set_style_text_align(fore_desc_label[i], LV_TEXT_ALIGN_CENTER, 0);
        lv_obj_align(fore_desc_label[i], LV_ALIGN_BOTTOM_MID, 0, -4);
    }

    // --- 3. Settings Tab UI Widgets ---
    // Main container inside tab_settings
    lv_obj_t * settings_grid = lv_obj_create(tab_settings);
    lv_obj_set_size(settings_grid, 310, 150);
    lv_obj_align(settings_grid, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_flex_flow(settings_grid, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(settings_grid, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(settings_grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(settings_grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(settings_grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(settings_grid, 0, LV_PART_MAIN);
    lv_obj_clear_flag(settings_grid, LV_OBJ_FLAG_SCROLLABLE);

    // Left Column — Unit switch, Auto Light switch, Theme dropdown
    lv_obj_t * left_col = lv_obj_create(settings_grid);
    lv_obj_set_size(left_col, 148, 148);
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(left_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(left_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(left_col, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(left_col, 8, LV_PART_MAIN);
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_SCROLLABLE);

    // Right Column — Brightness label+slider, Timezone label+buttons
    lv_obj_t * right_col = lv_obj_create(settings_grid);
    lv_obj_set_size(right_col, 155, 148);
    lv_obj_set_flex_flow(right_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(right_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(right_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(right_col, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(right_col, 8, LV_PART_MAIN);
    lv_obj_clear_flag(right_col, LV_OBJ_FLAG_SCROLLABLE);

    // --- Left column items ---

    // Unit (C/F) row
    lv_obj_t * unit_row = lv_obj_create(left_col);
    lv_obj_set_size(unit_row, 144, 28);
    lv_obj_set_flex_flow(unit_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(unit_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(unit_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(unit_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(unit_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(unit_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * unit_label = lv_label_create(unit_row);
    lv_label_set_text(unit_label, "Unit (C/F)");
    lv_obj_set_style_text_color(unit_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * unit_sw = lv_switch_create(unit_row);
    lv_obj_set_size(unit_sw, 40, 20);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        lv_obj_add_state(unit_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(unit_sw, unit_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Auto Light row
    lv_obj_t * auto_row = lv_obj_create(left_col);
    lv_obj_set_size(auto_row, 144, 28);
    lv_obj_set_flex_flow(auto_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(auto_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(auto_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(auto_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(auto_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(auto_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * auto_label = lv_label_create(auto_row);
    lv_label_set_text(auto_label, "Auto Light");
    lv_obj_set_style_text_color(auto_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // Theme selector dropdown
    lv_obj_t * theme_dropdown = lv_dropdown_create(left_col);
    lv_obj_set_size(theme_dropdown, 144, 30);
    lv_dropdown_set_options(theme_dropdown, "Mocha\nMacchiato\nFrappe\nLatte");
    lv_dropdown_set_selected(theme_dropdown, settings.getThemeFlavor() - 1);
    lv_obj_add_event_cb(theme_dropdown, theme_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // --- Right column items ---

    // Brightness label
    lv_obj_t * slider_label = lv_label_create(right_col);
    char slider_buf[32];
    snprintf(slider_buf, sizeof(slider_buf), "Bright: %d%%", settings.getBrightness());
    lv_label_set_text(slider_label, slider_buf);
    lv_obj_set_style_text_color(slider_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // Brightness slider — suppress thumb padding so it doesn't bloat height
    lv_obj_t * brightness_slider = lv_slider_create(right_col);
    lv_obj_set_size(brightness_slider, 120, 14);
    lv_obj_set_style_pad_top(brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(brightness_slider, 4, LV_PART_KNOB);
    lv_slider_set_range(brightness_slider, 10, 100);
    lv_slider_set_value(brightness_slider, settings.getBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);
    if (settings.getAutoBrightness()) {
        lv_obj_add_state(brightness_slider, LV_STATE_DISABLED);
    }

    // Auto switch (created after slider so we can pass slider as user_data)
    lv_obj_t * auto_sw = lv_switch_create(auto_row);
    lv_obj_set_size(auto_sw, 40, 20);
    if (settings.getAutoBrightness()) {
        lv_obj_add_state(auto_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(auto_sw, auto_sw_event_cb, LV_EVENT_VALUE_CHANGED, brightness_slider);

    // Timezone label
    lv_obj_t * tz_label = lv_label_create(right_col);
    lv_label_set_text(tz_label, "Timezone");
    lv_obj_set_style_text_color(tz_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // Timezone +/- row
    lv_obj_t * tz_row = lv_obj_create(right_col);
    lv_obj_set_size(tz_row, 148, 30);
    lv_obj_set_flex_flow(tz_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(tz_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(tz_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(tz_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tz_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(tz_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * tz_minus_btn = lv_btn_create(tz_row);
    lv_obj_set_size(tz_minus_btn, 30, 24);
    lv_obj_t * tz_minus_lbl = lv_label_create(tz_minus_btn);
    lv_label_set_text(tz_minus_lbl, "-");
    lv_obj_align(tz_minus_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(tz_minus_btn, tz_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)-1);

    tz_val_label = lv_label_create(tz_row);
    char tz_buf[16];
    int offset = settings.getTimezoneOffset();
    if (offset >= 0) {
        snprintf(tz_buf, sizeof(tz_buf), "GMT +%d", offset);
    } else {
        snprintf(tz_buf, sizeof(tz_buf), "GMT %d", offset);
    }
    lv_label_set_text(tz_val_label, tz_buf);
    lv_obj_set_style_text_color(tz_val_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);

    lv_obj_t * tz_plus_btn = lv_btn_create(tz_row);
    lv_obj_set_size(tz_plus_btn, 30, 24);
    lv_obj_t * tz_plus_lbl = lv_label_create(tz_plus_btn);
    lv_label_set_text(tz_plus_lbl, "+");
    lv_obj_align(tz_plus_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(tz_plus_btn, tz_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)1);
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
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        snprintf(temp_str, sizeof(temp_str), "%.1f °F", temperature);
    } else {
        snprintf(temp_str, sizeof(temp_str), "%.1f °C", temperature);
    }
    lv_label_set_text(temp_label, temp_str);

    char hum_str[32];
    snprintf(hum_str, sizeof(hum_str), "Humidity: %d%%", humidity);
    lv_label_set_text(hum_label, hum_str);

    char wind_str[32];
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        snprintf(wind_str, sizeof(wind_str), "Wind: %.1f mph", windSpeed);
    } else {
        snprintf(wind_str, sizeof(wind_str), "Wind: %.1f km/h", windSpeed);
    }
    lv_label_set_text(wind_label, wind_str);

    lv_label_set_text(status_lbl, status);

    // Update weather icon and its color
    lv_label_set_text(icon_lbl, getIconGlyph(weatherCode));
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(getIconColor(weatherCode)), LV_PART_MAIN);
}

void updateTimeUI(const char* time_str) {
    lv_label_set_text(time_label, time_str);
}

void updateForecastUI(const WeatherData& data) {
    for (int i = 0; i < 3; i++) {
        // Set Day name
        lv_label_set_text(fore_day_label[i], data.forecast[i].dayName.c_str());

        // Set Icon and its color
        lv_label_set_text(fore_icon_label[i], getIconGlyph(data.forecast[i].weatherCode));
        lv_obj_set_style_text_color(fore_icon_label[i], lv_color_hex(getIconColor(data.forecast[i].weatherCode)), LV_PART_MAIN);

        // Set Temp range
        char temp_range[32];
        snprintf(temp_range, sizeof(temp_range), "%.0f°/%.0f°", data.forecast[i].tempMax, data.forecast[i].tempMin);
        lv_label_set_text(fore_temp_label[i], temp_range);

        // Set Status description
        lv_label_set_text(fore_desc_label[i], data.forecast[i].status.c_str());
    }
}
