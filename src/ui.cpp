#include "ui.h"
#include <Arduino.h>
#include "config/config.h"
#include "settings_manager.h"
#include "sd_card_manager.h"
#include "screensaver_manager.h"

extern "C" {
LV_FONT_DECLARE(weather_icons_48);
LV_FONT_DECLARE(weather_icons_24);
LV_FONT_DECLARE(weather_icons_16);
}

extern SettingsManager settings;

volatile bool settings_unit_changed = false;
volatile bool settings_brightness_changed = false;
volatile bool settings_timezone_changed = false;
volatile bool settings_theme_changed = false;
volatile bool settings_sd_logging_changed = false;
volatile bool settings_screenshot_server_changed = false;
volatile bool settings_orientation_changed = false;
volatile bool settings_led_changed = false;
volatile bool settings_mqtt_changed = false;

static lv_obj_t *wifi_label;
static lv_obj_t *offline_indicator = nullptr;
static lv_obj_t *header_title = nullptr;
static bool is_offline_mode = false;
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *status_lbl;
static lv_obj_t *status_icon_lbl;
static lv_obj_t *time_label;
static lv_obj_t *icon_lbl;
static lv_obj_t *wind_label;
static lv_obj_t *tz_val_label;
static lv_obj_t *footer_label;
static lv_obj_t *tabview_obj = nullptr;

// Forecast widgets
static lv_obj_t *fore_day_label[3];
static lv_obj_t *fore_icon_label[3];
static lv_obj_t *fore_temp_label[3];
static lv_obj_t *fore_desc_label[3];

// Hourly forecast chart widgets
static lv_obj_t *hourly_chart = nullptr;
static lv_chart_series_t *hourly_temp_series = nullptr;
static lv_chart_series_t *hourly_precip_series = nullptr;

static void theme_dropdown_event_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dropdown);
    int flavor = selected + 1; // 1-based (Mocha=1, Macchiato=2, Frappe=3, Latte=4)
    if (flavor != settings.getThemeFlavor()) {
        settings.setThemeFlavor(flavor);
        settings_theme_changed = true;
    }
}

static void dst_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setDstEnabled(is_checked);
    settings_timezone_changed = true;
}

static void sd_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setSdLoggingEnabled(is_checked);
    settings_sd_logging_changed = true;
}

static void sd_cache_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setSdCacheEnabled(is_checked);
}

static void screenshot_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setScreenshotServerEnabled(is_checked);
    settings_screenshot_server_changed = true;
}

static void mqtt_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setMqttEnabled(is_checked);
    settings_mqtt_changed = true;
}

static void led_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setLedEnabled(is_checked);
    settings_led_changed = true;
    // --- Disable the brightness slider when LED is off ---
    lv_obj_t * slider = (lv_obj_t *)lv_event_get_user_data(e);
    if (slider) {
        if (is_checked) {
            lv_obj_clear_state(slider, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(slider, LV_STATE_DISABLED);
        }
    }
}

static void led_brightness_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int val = lv_slider_get_value(slider);
    settings.setLedBrightness(val);
    settings_led_changed = true;
    // --- Update the accompanying label ---
    lv_obj_t * label = (lv_obj_t *)lv_event_get_user_data(e);
    if (label) {
        char buf[32];
        snprintf(buf, sizeof(buf), "LED: %d%%", (val * 100) / 255);
        lv_label_set_text(label, buf);
    }
}

static const int dropdown_to_rotation[] = {1, 2, 3, 0};
static const int rotation_to_dropdown[] = {3, 0, 1, 2};

static void orientation_dropdown_event_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    uint16_t selected = lv_dropdown_get_selected(dropdown);
    int rotation = 1;
    if (selected < 4) {
        rotation = dropdown_to_rotation[selected];
    }
    settings.setScreenOrientation(rotation);
    settings_orientation_changed = true;
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

static void chart_draw_event_cb(lv_event_t * e) {
    lv_obj_draw_part_dsc_t * dsc = lv_event_get_draw_part_dsc(e);
    if (dsc->part == LV_PART_TICKS || dsc->type == LV_CHART_DRAW_PART_TICK_LABEL) {
        // Style the tick lines
        if (dsc->line_dsc) {
            dsc->line_dsc->color = lv_color_hex(COLOR_OVERLAY);
            dsc->line_dsc->width = 2;
        }
        
        // Style the tick text labels
        if (dsc->label_dsc) {
            dsc->label_dsc->color = lv_color_hex(COLOR_TEXT);
        }

        if (dsc->id == LV_CHART_AXIS_PRIMARY_X && dsc->text) {
            int hour_idx = dsc->value;
            if (hour_idx == 0) {
                lv_snprintf(dsc->text, dsc->text_length, "Now");
            } else if (hour_idx % 4 == 0) {
                lv_snprintf(dsc->text, dsc->text_length, "+%dh", hour_idx);
            } else {
                dsc->text[0] = '\0';
            }
        } else if (dsc->id == LV_CHART_AXIS_PRIMARY_Y && dsc->text) {
            int val = dsc->value;
            lv_snprintf(dsc->text, dsc->text_length, "%d°", val);
        } else if (dsc->id == LV_CHART_AXIS_SECONDARY_Y && dsc->text) {
            int val = dsc->value;
            lv_snprintf(dsc->text, dsc->text_length, "%d%%", val);
        }
    }
}

void initUI() {
    // Main screen setup (light grey background -> Catppuccin Base)
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_hex(COLOR_BASE), LV_PART_MAIN);

    // 1. Header Bar Container
    int rotation = 1;
#ifndef NATIVE_TEST
    rotation = settings.getScreenOrientation();
#endif
    bool isLandscape = (rotation == 1 || rotation == 3);
    int screen_w = isLandscape ? 320 : 240;
    int screen_h = isLandscape ? 240 : 320;
    int header_h = isLandscape ? 45 : 60; // 60px in portrait to fit 2-line title

    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, screen_w, header_h);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_CRUST), LV_PART_MAIN); // Crust header background
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Header Title (Time/App Name)
    header_title = lv_label_create(header);
    lv_label_set_text(header_title, isLandscape ? "CYD Weather Station" : "CYD Weather\nStation");
    lv_obj_set_style_text_color(header_title, lv_color_hex(COLOR_HEADER_TEXT), LV_PART_MAIN);
    lv_obj_align(header_title, LV_ALIGN_LEFT_MID, 10, 0);
    lv_label_set_long_mode(header_title, LV_LABEL_LONG_DOT);
    lv_obj_set_width(header_title, isLandscape ? 220 : 120);

    // Header Right-Side Status Area Container (handles WiFi, clock, offline label)
    lv_obj_t * header_right_area = lv_obj_create(header);
    lv_obj_set_size(header_right_area, LV_SIZE_CONTENT, 30);
    lv_obj_align(header_right_area, LV_ALIGN_RIGHT_MID, -5, 0);
    lv_obj_set_flex_flow(header_right_area, LV_FLEX_FLOW_ROW_REVERSE);
    lv_obj_set_flex_align(header_right_area, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(header_right_area, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(header_right_area, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(header_right_area, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(header_right_area, 8, LV_PART_MAIN);
    lv_obj_clear_flag(header_right_area, LV_OBJ_FLAG_SCROLLABLE);

    // Wi-Fi Label in Header
    wifi_label = lv_label_create(header_right_area);
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_YELLOW), LV_PART_MAIN); // Yellow / Amber icon

    // Time Label in Header
    time_label = lv_label_create(header_right_area);
    lv_label_set_text(time_label, "--:--");
    lv_obj_set_style_text_color(time_label, lv_color_hex(COLOR_HEADER_TEXT), LV_PART_MAIN);

    // Offline Label in Header (initially hidden)
    offline_indicator = lv_label_create(header_right_area);
    lv_label_set_text(offline_indicator, LV_SYMBOL_WARNING " Offline");
    lv_obj_set_style_text_color(offline_indicator, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_add_flag(offline_indicator, LV_OBJ_FLAG_HIDDEN);

    // 2. Tabview Setup
    lv_obj_t * tabview = lv_tabview_create(scr, LV_DIR_BOTTOM, 35);
    tabview_obj = tabview;
    lv_obj_set_size(tabview, screen_w, screen_h - header_h);
    lv_obj_align(tabview, LV_ALIGN_BOTTOM_MID, 0, 0);

    // Style the tabview container and buttons
    lv_obj_t * tab_btns = lv_tabview_get_tab_btns(tabview);
    lv_obj_set_style_bg_color(tabview, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    
    // Style the buttons
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_btns, lv_color_hex(COLOR_BASE), LV_PART_ITEMS | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(tab_btns, lv_color_hex(COLOR_PEACH), LV_PART_ITEMS | LV_STATE_CHECKED);

    // Adjust button padding and gaps to prevent text cramping in portrait mode (where width is only 240px)
    lv_obj_set_style_pad_left(tab_btns, isLandscape ? 8 : 2, LV_PART_ITEMS);
    lv_obj_set_style_pad_right(tab_btns, isLandscape ? 8 : 2, LV_PART_ITEMS);
    lv_obj_set_style_pad_top(tab_btns, 4, LV_PART_ITEMS);
    lv_obj_set_style_pad_bottom(tab_btns, 4, LV_PART_ITEMS);
    lv_obj_set_style_pad_gap(tab_btns, isLandscape ? 4 : 1, LV_PART_MAIN);

    // Tune swipe animation: speed it up and disable momentum throw for crisp
    // tab transitions on the embedded display.
    lv_obj_t * tab_content = lv_tabview_get_content(tabview);
    lv_obj_set_style_anim_time(tab_content, DISPLAY_SWIPE_ANIM_MS, 0); // set in config/config.h
    lv_obj_clear_flag(tab_content, LV_OBJ_FLAG_SCROLL_MOMENTUM);

    // Create the tabs
    lv_obj_t * tab_curr = lv_tabview_add_tab(tabview, "Current");
    lv_obj_t * tab_fore = lv_tabview_add_tab(tabview, "Forecast");
    lv_obj_t * tab_hourly = lv_tabview_add_tab(tabview, "Hourly");
    lv_obj_t * tab_settings = lv_tabview_add_tab(tabview, "Settings");

    // Set base color & padding on tabs
    // NOTE: Do NOT clear LV_OBJ_FLAG_SCROLLABLE on the tab panels —
    // the tabview's internal content object uses horizontal scrolling to
    // animate between tabs when the user swipes left/right.
    lv_obj_set_style_bg_color(tab_curr, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_fore, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_hourly, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_bg_color(tab_settings, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_curr, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_fore, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_hourly, 5, LV_PART_MAIN);
    lv_obj_set_style_pad_all(tab_settings, 5, LV_PART_MAIN);
    // Hide scrollbars so they don't appear visually
    lv_obj_set_scrollbar_mode(tab_curr, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(tab_fore, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(tab_hourly, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_scrollbar_mode(tab_settings, isLandscape ? LV_SCROLLBAR_MODE_OFF : LV_SCROLLBAR_MODE_AUTO);

    // Create Hourly Forecast Chart
    hourly_chart = lv_chart_create(tab_hourly);
    if (isLandscape) {
        lv_obj_set_size(hourly_chart, 230, 130);
    } else {
        lv_obj_set_size(hourly_chart, 160, 190);
    }
    lv_obj_align(hourly_chart, LV_ALIGN_CENTER, 0, -10);
    lv_chart_set_type(hourly_chart, LV_CHART_TYPE_LINE);

    // Styling Catppuccin
    lv_obj_set_style_bg_color(hourly_chart, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_border_width(hourly_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(hourly_chart, lv_color_hex(COLOR_OVERLAY), LV_PART_ITEMS); // grid lines
    lv_obj_set_style_text_color(hourly_chart, lv_color_hex(COLOR_TEXT), LV_PART_TICKS); // axis tick text
    lv_obj_set_style_line_rounded(hourly_chart, true, LV_PART_ITEMS);
    lv_obj_set_style_line_width(hourly_chart, 3, LV_PART_ITEMS);

    // Number of points is 24 (one for each hour)
    lv_chart_set_point_count(hourly_chart, 24);

    // Grid lines count
    lv_chart_set_div_line_count(hourly_chart, 5, 7); // 5 horizontal grid lines, 7 vertical grid lines

    // Enable axes labels and ticks
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_PRIMARY_X, 6, 3, 7, 2, true, 20);
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_PRIMARY_Y, 6, 3, 5, 2, true, 40);
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_SECONDARY_Y, 6, 3, 5, 2, true, 40);

    // Set secondary Y range to 0 - 100
    lv_chart_set_range(hourly_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);

    // Add event callback for tick labeling
    lv_obj_add_event_cb(hourly_chart, chart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    // Add series
    hourly_temp_series = lv_chart_add_series(hourly_chart, lv_color_hex(COLOR_MAUVE), LV_CHART_AXIS_PRIMARY_Y);
    hourly_precip_series = lv_chart_add_series(hourly_chart, lv_color_hex(COLOR_BLUE), LV_CHART_AXIS_SECONDARY_Y);

    // Weather Placeholders inside tab_curr
    icon_lbl = lv_label_create(tab_curr);
    lv_obj_set_style_text_font(icon_lbl, &weather_icons_48, LV_PART_MAIN);
    lv_label_set_text(icon_lbl, "\xef\x81\xbb"); // fallback NA icon (f07b)
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    
    if (isLandscape) {
        lv_obj_align(icon_lbl, LV_ALIGN_LEFT_MID, 12, -10);
    } else {
        lv_obj_align(icon_lbl, LV_ALIGN_TOP_MID, 0, 10);
    }
 
    // Vertical container for details on the right side
    lv_obj_t * details_cnt = lv_obj_create(tab_curr);
    lv_obj_set_size(details_cnt, 200, 115);
    
    if (isLandscape) {
        lv_obj_align(details_cnt, LV_ALIGN_RIGHT_MID, -10, -10);
    } else {
        lv_obj_align(details_cnt, LV_ALIGN_BOTTOM_MID, 0, -25);
    }
    
    lv_obj_set_flex_flow(details_cnt, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(details_cnt, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

    // Remove styling constraints and scrollbars
    lv_obj_set_style_bg_opa(details_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(details_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(details_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_row(details_cnt, 2, LV_PART_MAIN); // vertical space between children (reduced from 4)
    lv_obj_clear_flag(details_cnt, LV_OBJ_FLAG_SCROLLABLE);

    // Helper lambda-style macro for creating an icon+text row in details_cnt
    // Each row: transparent flex-row container → icon label → text label

    // --- Temperature row (wi-thermometer + value) ---
    lv_obj_t * temp_cnt = lv_obj_create(details_cnt);
    lv_obj_set_size(temp_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(temp_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temp_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(temp_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(temp_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(temp_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(temp_cnt, 4, LV_PART_MAIN); // reduced spacing
    lv_obj_clear_flag(temp_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * temp_icon_lbl = lv_label_create(temp_cnt);
    lv_obj_set_style_text_font(temp_icon_lbl, &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(temp_icon_lbl, "\xef\x81\x95"); // U+F055 wi-thermometer
    lv_obj_set_style_text_color(temp_icon_lbl, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_set_width(temp_icon_lbl, 20);
    lv_obj_set_style_text_align(temp_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    temp_label = lv_label_create(temp_cnt);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        lv_label_set_text(temp_label, "--.- °F");
    } else {
        lv_label_set_text(temp_label, "--.- °C");
    }
    lv_obj_set_style_text_font(temp_label, &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);

    // --- Humidity row (wi-humidity + value) ---
    lv_obj_t * hum_cnt = lv_obj_create(details_cnt);
    lv_obj_set_size(hum_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hum_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hum_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(hum_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(hum_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hum_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(hum_cnt, 4, LV_PART_MAIN);
    lv_obj_clear_flag(hum_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * hum_icon_lbl = lv_label_create(hum_cnt);
    lv_obj_set_style_text_font(hum_icon_lbl, &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(hum_icon_lbl, "\xef\x81\xba"); // U+F07A wi-humidity
    lv_obj_set_style_text_color(hum_icon_lbl, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
    lv_obj_set_width(hum_icon_lbl, 20);
    lv_obj_set_style_text_align(hum_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    hum_label = lv_label_create(hum_cnt);
    lv_label_set_text(hum_label, "--%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);

    // --- Wind row (wi-windy + value) ---
    lv_obj_t * wind_cnt = lv_obj_create(details_cnt);
    lv_obj_set_size(wind_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(wind_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wind_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(wind_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(wind_cnt, 4, LV_PART_MAIN); // horizontal space between icon and text
    lv_obj_clear_flag(wind_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * wind_icon_lbl = lv_label_create(wind_cnt);
    lv_obj_set_style_text_font(wind_icon_lbl, &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(wind_icon_lbl, "\xef\x80\xa1"); // U+F021 (wi-windy)
    lv_obj_set_style_text_color(wind_icon_lbl, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);
    lv_obj_set_width(wind_icon_lbl, 20);
    lv_obj_set_style_text_align(wind_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    wind_label = lv_label_create(wind_cnt);
    lv_label_set_text(wind_label, "Wind: -- km/h");
    lv_obj_set_style_text_color(wind_label, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);

    // --- Status row (dynamic weather-code icon + description text) ---
    lv_obj_t * status_cnt = lv_obj_create(details_cnt);
    lv_obj_set_size(status_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(status_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(status_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(status_cnt, 4, LV_PART_MAIN);
    lv_obj_clear_flag(status_cnt, LV_OBJ_FLAG_SCROLLABLE);

    status_icon_lbl = lv_label_create(status_cnt);
    lv_obj_set_style_text_font(status_icon_lbl, &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(status_icon_lbl, "\xef\x81\xbb"); // U+F07B wi-na (updated on fetch)
    lv_obj_set_style_text_color(status_icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_width(status_icon_lbl, 20);
    lv_obj_set_style_text_align(status_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    status_lbl = lv_label_create(status_cnt);
    lv_label_set_text(status_lbl, "Updating...");
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);

    // Footer bar: "Last Update: HH:MM | City Name"
    footer_label = lv_label_create(tab_curr);
    lv_label_set_text(footer_label, "Last Update: -- | --");
    lv_obj_set_style_text_color(footer_label, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_text_font(footer_label, &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_long_mode(footer_label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(footer_label, isLandscape ? 300 : 220);
    lv_obj_set_style_text_align(footer_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(footer_label, LV_ALIGN_BOTTOM_MID, 0, -2);

    // 3-day Forecast Card Layout inside tab_fore
    for (int i = 0; i < 3; i++) {
        lv_obj_t *day_card = lv_obj_create(tab_fore);
        
        // Card styling: Catppuccin Mantle background
        lv_obj_set_style_bg_color(day_card, lv_color_hex(COLOR_MANTLE), LV_PART_MAIN);
        lv_obj_set_style_radius(day_card, 8, LV_PART_MAIN);
        lv_obj_set_style_border_width(day_card, 1, LV_PART_MAIN);
        lv_obj_set_style_border_color(day_card, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
        lv_obj_clear_flag(day_card, LV_OBJ_FLAG_SCROLLABLE);

        if (isLandscape) {
            lv_obj_set_size(day_card, 90, 130);
            if (i == 0) {
                lv_obj_align(day_card, LV_ALIGN_LEFT_MID, 10, 0);
            } else if (i == 1) {
                lv_obj_align(day_card, LV_ALIGN_CENTER, 0, 0);
            } else {
                lv_obj_align(day_card, LV_ALIGN_RIGHT_MID, -10, 0);
            }
            lv_obj_set_style_pad_all(day_card, 2, LV_PART_MAIN);

            // 1. Day Name Label
            fore_day_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_day_label[i], i == 0 ? "Today" : (i == 1 ? "Tmrw" : "Day"));
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
        } else {
            // Portrait layout: row flow, vertical list
            lv_obj_set_size(day_card, 220, 42);
            lv_obj_align(day_card, LV_ALIGN_TOP_MID, 0, 5 + i * 47);
            lv_obj_set_flex_flow(day_card, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(day_card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_all(day_card, 6, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(day_card, 4, LV_PART_MAIN);

            // 1. Day Name Label
            fore_day_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_day_label[i], i == 0 ? "Today" : (i == 1 ? "Tmrw" : "Day"));
            lv_obj_set_style_text_color(fore_day_label[i], lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
            lv_obj_set_width(fore_day_label[i], 50);
            lv_obj_set_style_text_align(fore_day_label[i], LV_TEXT_ALIGN_LEFT, 0);

            // 2. Weather Icon Label
            fore_icon_label[i] = lv_label_create(day_card);
            lv_obj_set_style_text_font(fore_icon_label[i], &weather_icons_24, LV_PART_MAIN);
            lv_label_set_text(fore_icon_label[i], "\xef\x81\xbb"); // fallback NA
            lv_obj_set_style_text_color(fore_icon_label[i], lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
            lv_obj_set_width(fore_icon_label[i], 24);
            lv_obj_set_style_text_align(fore_icon_label[i], LV_TEXT_ALIGN_CENTER, 0);

            // 3. Temp Label (High / Low)
            fore_temp_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_temp_label[i], "--°/--°");
            lv_obj_set_style_text_color(fore_temp_label[i], lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
            lv_obj_set_width(fore_temp_label[i], 65);
            lv_obj_set_style_text_align(fore_temp_label[i], LV_TEXT_ALIGN_CENTER, 0);

            // 4. Status Description Label
            fore_desc_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_desc_label[i], "Loading...");
            lv_obj_set_style_text_color(fore_desc_label[i], lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
            lv_label_set_long_mode(fore_desc_label[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_width(fore_desc_label[i], 55);
            lv_obj_set_style_text_align(fore_desc_label[i], LV_TEXT_ALIGN_RIGHT, 0);
        }
    }

    // --- 3. Settings Tab UI Widgets ---
    // Main container inside tab_settings
    lv_obj_t * settings_grid = lv_obj_create(tab_settings);
    lv_obj_set_size(settings_grid, isLandscape ? 310 : 230, LV_SIZE_CONTENT);
    lv_obj_align(settings_grid, LV_ALIGN_TOP_MID, 0, 5);
    lv_obj_set_flex_flow(settings_grid, isLandscape ? LV_FLEX_FLOW_ROW : LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(settings_grid, isLandscape ? LV_FLEX_ALIGN_SPACE_BETWEEN : LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(settings_grid, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(settings_grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(settings_grid, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(settings_grid, isLandscape ? 0 : 4, LV_PART_MAIN);
    lv_obj_clear_flag(settings_grid, LV_OBJ_FLAG_SCROLLABLE);

    // Left Column — Unit switch, Auto Light switch, Theme dropdown
    lv_obj_t * left_col = lv_obj_create(settings_grid);
    lv_obj_set_size(left_col, 148, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(left_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(left_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(left_col, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(left_col, 4, LV_PART_MAIN);
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_SCROLLABLE);

    // Right Column — Brightness label+slider, Timezone label+buttons
    lv_obj_t * right_col = lv_obj_create(settings_grid);
    lv_obj_set_size(right_col, 155, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(right_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(right_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(right_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(right_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(right_col, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(right_col, isLandscape ? 4 : 8, LV_PART_MAIN);
    lv_obj_clear_flag(right_col, LV_OBJ_FLAG_SCROLLABLE);

    // --- Left column items ---

    // Unit (C/F) row
    lv_obj_t * unit_row = lv_obj_create(left_col);
    lv_obj_set_size(unit_row, 144, 22);
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
    // Switch track — off state
    lv_obj_set_style_bg_color(unit_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    // Switch track — on/checked state
    lv_obj_set_style_bg_color(unit_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    // Switch knob
    lv_obj_set_style_bg_color(unit_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        lv_obj_add_state(unit_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(unit_sw, unit_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Auto Light row
    lv_obj_t * auto_row = lv_obj_create(left_col);
    lv_obj_set_size(auto_row, 144, 22);
    lv_obj_set_flex_flow(auto_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(auto_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(auto_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(auto_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(auto_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(auto_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * auto_label = lv_label_create(auto_row);
    lv_label_set_text(auto_label, "Auto Light");
    lv_obj_set_style_text_color(auto_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // DST switch row
    lv_obj_t * dst_row = lv_obj_create(left_col);
    lv_obj_set_size(dst_row, 144, 22);
    lv_obj_set_flex_flow(dst_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(dst_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(dst_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(dst_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(dst_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(dst_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * dst_label = lv_label_create(dst_row);
    lv_label_set_text(dst_label, "DST");
    lv_obj_set_style_text_color(dst_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * dst_sw = lv_switch_create(dst_row);
    lv_obj_set_size(dst_sw, 40, 20);
    lv_obj_set_style_bg_color(dst_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(dst_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(dst_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getDstEnabled()) {
        lv_obj_add_state(dst_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(dst_sw, dst_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // SD Log row
    lv_obj_t * sd_row = lv_obj_create(left_col);
    lv_obj_set_size(sd_row, 144, 22);
    lv_obj_set_flex_flow(sd_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sd_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sd_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sd_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sd_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sd_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sd_label = lv_label_create(sd_row);
    lv_label_set_text(sd_label, "SD Log");
    lv_obj_set_style_text_color(sd_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * sd_sw = lv_switch_create(sd_row);
    lv_obj_set_size(sd_sw, 40, 20);
    lv_obj_set_style_bg_color(sd_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sd_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sd_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (SdCardManager::isCardPresent()) {
        if (settings.getSdLoggingEnabled()) {
            lv_obj_add_state(sd_sw, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(sd_sw, sd_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        lv_obj_add_state(sd_sw, LV_STATE_DISABLED);
    }

    // SD Cache row
    lv_obj_t * sd_cache_row = lv_obj_create(left_col);
    lv_obj_set_size(sd_cache_row, 144, 22);
    lv_obj_set_flex_flow(sd_cache_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sd_cache_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sd_cache_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sd_cache_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sd_cache_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sd_cache_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sd_cache_label = lv_label_create(sd_cache_row);
    lv_label_set_text(sd_cache_label, "SD Cache");
    lv_obj_set_style_text_color(sd_cache_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * sd_cache_sw = lv_switch_create(sd_cache_row);
    lv_obj_set_size(sd_cache_sw, 40, 20);
    lv_obj_set_style_bg_color(sd_cache_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(sd_cache_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sd_cache_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (SdCardManager::isCardPresent()) {
        if (settings.getSdCacheEnabled()) {
            lv_obj_add_state(sd_cache_sw, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(sd_cache_sw, sd_cache_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        lv_obj_add_state(sd_cache_sw, LV_STATE_DISABLED);
    }

    // Screenshot Server row
    lv_obj_t * scr_row = lv_obj_create(left_col);
    lv_obj_set_size(scr_row, 144, 22);
    lv_obj_set_flex_flow(scr_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scr_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(scr_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(scr_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * scr_label = lv_label_create(scr_row);
    lv_label_set_text(scr_label, "Scr Srv");
    lv_obj_set_style_text_color(scr_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * scr_sw = lv_switch_create(scr_row);
    lv_obj_set_size(scr_sw, 40, 20);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getScreenshotServerEnabled()) {
        lv_obj_add_state(scr_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(scr_sw, screenshot_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // MQTT row
    lv_obj_t * mqtt_row = lv_obj_create(left_col);
    lv_obj_set_size(mqtt_row, 144, 22);
    lv_obj_set_flex_flow(mqtt_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mqtt_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(mqtt_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(mqtt_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(mqtt_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(mqtt_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * mqtt_label = lv_label_create(mqtt_row);
    lv_label_set_text(mqtt_label, "MQTT");
    lv_obj_set_style_text_color(mqtt_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * mqtt_sw = lv_switch_create(mqtt_row);
    lv_obj_set_size(mqtt_sw, 40, 20);
    lv_obj_set_style_bg_color(mqtt_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(mqtt_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(mqtt_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getMqttEnabled()) {
        lv_obj_add_state(mqtt_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(mqtt_sw, mqtt_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // --- Right column items ---

    // Brightness label
    lv_obj_t * slider_label = lv_label_create(right_col);
    char slider_buf[32];
    snprintf(slider_buf, sizeof(slider_buf), "Bright: %d%%", settings.getBrightness());
    lv_label_set_text(slider_label, slider_buf);
    lv_obj_set_style_text_color(slider_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // Brightness slider — suppress thumb padding so it doesn't bloat height
    lv_obj_t * brightness_slider = lv_slider_create(right_col);
    lv_obj_set_size(brightness_slider, 136, 14);
    lv_obj_set_style_pad_top(brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(brightness_slider, 4, LV_PART_KNOB);
    // Slider track (unfilled)
    lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    // Slider filled indicator
    lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    // Slider knob
    lv_obj_set_style_bg_color(brightness_slider, lv_color_hex(COLOR_TEXT), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_range(brightness_slider, 10, 100);
    lv_slider_set_value(brightness_slider, settings.getBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);
    if (settings.getAutoBrightness()) {
        lv_obj_add_state(brightness_slider, LV_STATE_DISABLED);
    }

    // LED row
    lv_obj_t * led_row = lv_obj_create(left_col);
    lv_obj_set_size(led_row, 144, 22);
    lv_obj_set_flex_flow(led_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(led_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(led_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(led_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(led_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(led_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * led_label_txt = lv_label_create(led_row);
    lv_label_set_text(led_label_txt, "LED");
    lv_obj_set_style_text_color(led_label_txt, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    // --- right column LED Brightness label + slider (created first so we can pass as user_data) ---
    lv_obj_t * led_slider_label = lv_label_create(right_col);
    char led_slider_buf[32];
    int led_bright_pct = (settings.getLedBrightness() * 100) / 255;
    snprintf(led_slider_buf, sizeof(led_slider_buf), "LED: %d%%", led_bright_pct);
    lv_label_set_text(led_slider_label, led_slider_buf);
    lv_obj_set_style_text_color(led_slider_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);

    lv_obj_t * led_brightness_slider = lv_slider_create(right_col);
    lv_obj_set_size(led_brightness_slider, 136, 14);
    lv_obj_set_style_pad_top(led_brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(led_brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_bg_color(led_brightness_slider, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(led_brightness_slider, lv_color_hex(COLOR_MAUVE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(led_brightness_slider, lv_color_hex(COLOR_TEXT), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_range(led_brightness_slider, 0, 255);
    lv_slider_set_value(led_brightness_slider, settings.getLedBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(led_brightness_slider, led_brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, led_slider_label);
    if (!settings.getLedEnabled()) {
        lv_obj_add_state(led_brightness_slider, LV_STATE_DISABLED);
    }

    // --- LED enable switch (created after slider so slider can be passed as user_data) ---
    lv_obj_t * led_sw = lv_switch_create(led_row);
    lv_obj_set_size(led_sw, 40, 20);
    lv_obj_set_style_bg_color(led_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(led_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(led_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getLedEnabled()) {
        lv_obj_add_state(led_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(led_sw, led_sw_event_cb, LV_EVENT_VALUE_CHANGED, led_brightness_slider);

    // Auto switch (created after slider so we can pass slider as user_data)
    lv_obj_t * auto_sw = lv_switch_create(auto_row);
    lv_obj_set_size(auto_sw, 40, 20);
    lv_obj_set_style_bg_color(auto_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(auto_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(auto_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
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
    lv_obj_set_style_bg_color(tz_minus_btn, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(tz_minus_btn, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t * tz_minus_lbl = lv_label_create(tz_minus_btn);
    lv_label_set_text(tz_minus_lbl, "-");
    lv_obj_set_style_text_color(tz_minus_lbl, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
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
    lv_obj_set_style_bg_color(tz_plus_btn, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(tz_plus_btn, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN | LV_STATE_PRESSED);
    lv_obj_t * tz_plus_lbl = lv_label_create(tz_plus_btn);
    lv_label_set_text(tz_plus_lbl, "+");
    lv_obj_set_style_text_color(tz_plus_lbl, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_align(tz_plus_lbl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(tz_plus_btn, tz_btn_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)1);

    // Theme selector dropdown
    lv_obj_t * theme_dropdown = lv_dropdown_create(right_col);
    lv_obj_set_size(theme_dropdown, 148, 26);
    lv_dropdown_set_options(theme_dropdown, "Mocha\nMacchiato\nFrappe\nLatte");
    lv_dropdown_set_selected(theme_dropdown, settings.getThemeFlavor() - 1);
    lv_obj_add_event_cb(theme_dropdown, theme_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // Dropdown button body
    lv_obj_set_style_bg_color(theme_dropdown, lv_color_hex(COLOR_CRUST), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(theme_dropdown, lv_color_hex(COLOR_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(theme_dropdown, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(theme_dropdown, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // Dropdown list
    lv_obj_t * dropdown_list = lv_dropdown_get_list(theme_dropdown);
    lv_obj_set_style_bg_color(dropdown_list, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_set_style_text_color(dropdown_list, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_border_color(dropdown_list, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(dropdown_list, lv_color_hex(COLOR_BLUE), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(dropdown_list, lv_color_hex(COLOR_CRUST), LV_PART_SELECTED | LV_STATE_CHECKED);

    // Orientation selector dropdown
    lv_obj_t * orientation_dropdown = lv_dropdown_create(right_col);
    lv_obj_set_size(orientation_dropdown, 148, 26);
    lv_dropdown_set_options(orientation_dropdown, "Landscape\nPortrait\nLandscape Rev\nPortrait Rev");
    int initial_rot = settings.getScreenOrientation();
    if (initial_rot >= 0 && initial_rot < 4) {
        lv_dropdown_set_selected(orientation_dropdown, rotation_to_dropdown[initial_rot]);
    }
    lv_obj_add_event_cb(orientation_dropdown, orientation_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    // Dropdown button body
    lv_obj_set_style_bg_color(orientation_dropdown, lv_color_hex(COLOR_CRUST), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(orientation_dropdown, lv_color_hex(COLOR_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(orientation_dropdown, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(orientation_dropdown, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    // Dropdown list
    lv_obj_t * orient_list = lv_dropdown_get_list(orientation_dropdown);
    lv_obj_set_style_bg_color(orient_list, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_set_style_text_color(orient_list, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_border_color(orient_list, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(orient_list, lv_color_hex(COLOR_BLUE), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(orient_list, lv_color_hex(COLOR_CRUST), LV_PART_SELECTED | LV_STATE_CHECKED);

    // Re-apply offline indicator state if active
    if (is_offline_mode) {
        updateOfflineIndicator(true);
    }
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

void updateWifiAPMode(const char* apSSID) {
    lv_label_set_text(wifi_label, LV_SYMBOL_WIFI);
    lv_obj_set_style_text_color(wifi_label, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN); // Catppuccin Mauve for AP Mode

    lv_label_set_text(status_lbl, "AP Active");
    lv_label_set_text(temp_label, "Setup");
    lv_label_set_text(hum_label, "192.168.4.1");
    lv_label_set_text(wind_label, apSSID);
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

const char* getCardinalDirection(int degrees) {
    degrees = (degrees % 360 + 360) % 360;
    static const char* directions[] = {"N", "NNE", "NE", "ENE", "E", "ESE", "SE", "SSE",
                                       "S", "SSW", "SW", "WSW", "W", "WNW", "NW", "NNW"};
    int index = (int)((degrees + 11.25) / 22.5) % 16;
    return directions[index];
}

void updateWeatherUI(float temperature, int humidity, const char* status, int weatherCode, float windSpeed, int windDirection) {
    char temp_str[32];
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        snprintf(temp_str, sizeof(temp_str), "%.1f °F", temperature);
    } else {
        snprintf(temp_str, sizeof(temp_str), "%.1f °C", temperature);
    }
    lv_label_set_text(temp_label, temp_str);

    char hum_str[32];
    snprintf(hum_str, sizeof(hum_str), "%d%%", humidity);
    lv_label_set_text(hum_label, hum_str);

    char wind_str[48];
    const char* cardinal = getCardinalDirection(windDirection);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        snprintf(wind_str, sizeof(wind_str), "%.1f mph %s", windSpeed, cardinal);
    } else {
        snprintf(wind_str, sizeof(wind_str), "%.1f km/h %s", windSpeed, cardinal);
    }
    lv_label_set_text(wind_label, wind_str);

    lv_label_set_text(status_lbl, status);

    // Update large weather icon (left side) and small status row icon (right side)
    lv_label_set_text(icon_lbl, getIconGlyph(weatherCode));
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(getIconColor(weatherCode)), LV_PART_MAIN);
    lv_label_set_text(status_icon_lbl, getIconGlyph(weatherCode));
    lv_obj_set_style_text_color(status_icon_lbl, lv_color_hex(getIconColor(weatherCode)), LV_PART_MAIN);
}

void updateTimeUI(const char* time_str) {
    lv_label_set_text(time_label, time_str);
    updateScreenSaverTime(time_str);
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

void updateFooterUI(const char* update_time, const char* city) {
    char buf[64];
    if (city && city[0] != '\0') {
        snprintf(buf, sizeof(buf), "Last Update: %s | %s", update_time, city);
    } else {
        snprintf(buf, sizeof(buf), "Last Update: %s", update_time);
    }
    lv_label_set_text(footer_label, buf);
}

void setUIActiveTab(int index) {
    if (tabview_obj != nullptr && index >= 0 && index < 4) {
        lv_tabview_set_act(tabview_obj, index, LV_ANIM_OFF);
    }
}

void setUIOrientation(int rotation) {
    if (rotation >= 0 && rotation < 4) {
        if (rotation != settings.getScreenOrientation()) {
            settings.setScreenOrientation(rotation);
            settings_orientation_changed = true;
        }
    }
}

void updateOfflineIndicator(bool isOffline) {
    is_offline_mode = isOffline;
    if (offline_indicator != nullptr) {
        if (isOffline) {
            lv_obj_clear_flag(offline_indicator, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(offline_indicator, LV_OBJ_FLAG_HIDDEN);
        }

        // Adjust title width to prevent overlap with the offline indicator
        if (header_title != nullptr) {
            int rotation = settings.getScreenOrientation();
            bool isLandscape = (rotation == 1 || rotation == 3);
            if (isOffline) {
                // If offline indicator is shown, restrict title width
                lv_obj_set_width(header_title, isLandscape ? 130 : 90);
            } else {
                // If offline indicator is hidden, allow title to take more space
                lv_obj_set_width(header_title, isLandscape ? 220 : 120);
            }
        }
    }
}

static lv_obj_t *screensaver_overlay = nullptr;
static lv_obj_t *screensaver_clock = nullptr;

static void screensaver_click_event_cb(lv_event_t * e) {
    uint8_t target = 80;
#ifndef NATIVE_TEST
    target = settings.getBrightness();
#endif
    if (target < 10) target = 10;
    screensaver.wake(target);
}

void showScreenSaver() {
    if (screensaver_overlay != nullptr) return;

    // Create full screen overlay on active screen
    screensaver_overlay = lv_obj_create(lv_scr_act());
    lv_obj_set_size(screensaver_overlay, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(screensaver_overlay, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(screensaver_overlay, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(screensaver_overlay, 0, 0);
    lv_obj_set_style_radius(screensaver_overlay, 0, 0);
    lv_obj_set_style_pad_all(screensaver_overlay, 0, 0);
    lv_obj_align(screensaver_overlay, LV_ALIGN_TOP_LEFT, 0, 0);

    // Create clock label
    screensaver_clock = lv_label_create(screensaver_overlay);
    lv_obj_set_style_text_color(screensaver_clock, lv_color_make(180, 180, 180), 0);
    lv_obj_set_style_text_font(screensaver_clock, &lv_font_montserrat_48, 0);
    
    if (time_label != nullptr) {
        lv_label_set_text(screensaver_clock, lv_label_get_text(time_label));
    } else {
        lv_label_set_text(screensaver_clock, "--:--");
    }

    lv_obj_align(screensaver_clock, LV_ALIGN_CENTER, 0, 0);

    // Make overlay clickable and add callback
    lv_obj_add_flag(screensaver_overlay, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(screensaver_overlay, screensaver_click_event_cb, LV_EVENT_CLICKED, nullptr);
}

void hideScreenSaver() {
    if (screensaver_overlay != nullptr) {
        lv_obj_del(screensaver_overlay);
        screensaver_overlay = nullptr;
        screensaver_clock = nullptr;
    }
}

void updateScreenSaverTime(const char* time_str) {
    if (screensaver_overlay == nullptr || screensaver_clock == nullptr) return;
    
    lv_label_set_text(screensaver_clock, time_str);

    // Drift position periodically to prevent screen burn-in
    static int update_counter = 0;
    update_counter++;
    if (update_counter >= 15) {
        update_counter = 0;
        
        int max_x = 100;
        int max_y = 100;
#ifndef NATIVE_TEST
        int rotation = settings.getScreenOrientation();
        if (rotation == 1 || rotation == 3) {
            max_x = 320 - 150;
            max_y = 240 - 55;
        } else {
            max_x = 240 - 150;
            max_y = 320 - 55;
        }
#endif
        if (max_x < 10) max_x = 10;
        if (max_y < 10) max_y = 10;

#ifndef NATIVE_TEST
        int new_x = random(5, max_x);
        int new_y = random(5, max_y);
#else
        int new_x = 20;
        int new_y = 20;
#endif

        lv_obj_align(screensaver_clock, LV_ALIGN_TOP_LEFT, new_x, new_y);
    }
}

void showUIStatusMessage(const char* message) {
    lv_obj_t * scr = lv_scr_act();
    // Create a container for the status message
    lv_obj_t * card = lv_obj_create(scr);
    if (!card) return;
    
    // Clear scrollable flag
    lv_obj_clear_flag(card, LV_OBJ_FLAG_SCROLLABLE);
    
    // Style card
    lv_obj_set_style_bg_color(card, lv_color_hex(COLOR_MANTLE), LV_PART_MAIN);
    lv_obj_set_style_border_color(card, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
    lv_obj_set_style_border_width(card, 2, LV_PART_MAIN);
    lv_obj_set_style_border_opa(card, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(card, 8, LV_PART_MAIN);
    
    // Size and align
    lv_obj_set_size(card, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_style_pad_all(card, 15, LV_PART_MAIN);
    lv_obj_align(card, LV_ALIGN_CENTER, 0, 0);
    
    // Create label
    lv_obj_t * label = lv_label_create(card);
    if (label) {
        lv_label_set_text(label, message);
        lv_obj_set_style_text_color(label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
        lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
    }
    
    // Auto-delete after 2 seconds
    lv_obj_del_delayed(card, 2000);
}

void updateHourlyUI(const WeatherData& data) {
    if (hourly_chart == nullptr || hourly_temp_series == nullptr || hourly_precip_series == nullptr) return;

    // Dynamically adjust Y range based on actual temperature min/max to maximize visual precision
    float temp_min = 999.0f;
    float temp_max = -999.0f;
    for (int i = 0; i < 24; i++) {
        float temp = data.hourly[i].temperature;
        if (temp < temp_min) temp_min = temp;
        if (temp > temp_max) temp_max = temp;
    }

    // Safety margin of 3 degrees on min and max
    int y_min = (int)(temp_min - 3.0f);
    int y_max = (int)(temp_max + 3.0f);
    
    // Ensure we don't have min >= max
    if (y_min >= y_max) {
        y_min = (int)temp_min - 5;
        y_max = (int)temp_min + 5;
    }
    
    lv_chart_set_range(hourly_chart, LV_CHART_AXIS_PRIMARY_Y, y_min, y_max);

    // Populate data using direct array access
    for (int i = 0; i < 24; i++) {
        hourly_temp_series->y_points[i] = (lv_coord_t)data.hourly[i].temperature;
        hourly_precip_series->y_points[i] = (lv_coord_t)data.hourly[i].precipitationProbability;
    }

    lv_chart_refresh(hourly_chart);
}

