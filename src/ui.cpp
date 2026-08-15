#include "ui.h"
#include <Arduino.h>
#include "config/config.h"
#include "settings_manager.h"
#include "sd_card_manager.h"
#include "screensaver_manager.h"
#include "version.h"
#include <WiFi.h>

extern "C" {
LV_FONT_DECLARE(weather_icons_72);
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
volatile bool settings_local_sensor_changed = false;

static lv_obj_t *wifi_label;
static lv_obj_t *offline_indicator = nullptr;
static lv_obj_t *header_title = nullptr;
static bool is_offline_mode = false;

static lv_obj_t *wifi_info_dialog = nullptr;

static lv_obj_t *ui_auto_bright_sw = nullptr;
static lv_obj_t *ui_screensaver_sw = nullptr;
static lv_obj_t *ui_sleep_sw = nullptr;
static lv_obj_t *ui_led_sw = nullptr;
static lv_obj_t *ui_sd_log_sw = nullptr;
static lv_obj_t *ui_sd_cache_sw = nullptr;
static lv_obj_t *ui_brightness_slider = nullptr;
static lv_obj_t *ui_led_brightness_slider = nullptr;
static lv_obj_t *ui_sw_local_sensor = nullptr;
static lv_obj_t *ui_dd_local_sensor_type = nullptr;
static lv_obj_t *ui_slider_local_sensor_update_interval = nullptr;
static lv_obj_t *ui_slider_temp_offset = nullptr;
static lv_obj_t *ui_slider_hum_offset = nullptr;

static void close_wifi_info_cb(lv_event_t * e) {
    if (wifi_info_dialog != nullptr) {
        lv_obj_del(wifi_info_dialog);
        wifi_info_dialog = nullptr;
    }
}

static void wifi_icon_click_cb(lv_event_t * e) {
    if (WiFi.status() != WL_CONNECTED && WiFi.getMode() != WIFI_AP) return;
    if (wifi_info_dialog != nullptr) return;

    bool isLargeScreen = (lv_disp_get_hor_res(NULL) >= 480 || lv_disp_get_ver_res(NULL) >= 480);

    wifi_info_dialog = lv_obj_create(lv_scr_act());
    lv_obj_set_size(wifi_info_dialog, lv_pct(85), lv_pct(85));
    lv_obj_align(wifi_info_dialog, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_bg_color(wifi_info_dialog, lv_color_hex(COLOR_MANTLE), 0);
    lv_obj_set_style_border_color(wifi_info_dialog, lv_color_hex(COLOR_OVERLAY), 0);
    lv_obj_set_style_border_width(wifi_info_dialog, 2, 0);
    lv_obj_set_style_radius(wifi_info_dialog, 10, 0);
    lv_obj_clear_flag(wifi_info_dialog, LV_OBJ_FLAG_SCROLLABLE);

    // Title label
    lv_obj_t * lbl_title = lv_label_create(wifi_info_dialog);
    lv_label_set_text(lbl_title, "WiFi Info");
    lv_obj_set_style_text_font(lbl_title, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, isLargeScreen ? 10 : 5);

    // Status label
    lv_obj_t * lbl_status = lv_label_create(wifi_info_dialog);
    if (WiFi.getMode() == WIFI_AP) {
        lv_label_set_text(lbl_status, "AP Mode Active");
        lv_obj_set_style_text_color(lbl_status, lv_color_hex(COLOR_MAUVE), 0);
    } else {
        lv_label_set_text(lbl_status, "Connected");
        lv_obj_set_style_text_color(lbl_status, lv_color_hex(COLOR_GREEN), 0);
    }
    lv_obj_set_style_text_font(lbl_status, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_status, LV_ALIGN_TOP_MID, 0, isLargeScreen ? 40 : 25);

    // Info details
    lv_obj_t * lbl_info = lv_label_create(wifi_info_dialog);
    char infoBuf[256];
    if (WiFi.getMode() == WIFI_AP) {
        snprintf(infoBuf, sizeof(infoBuf), 
                 "SSID: %s\nIP: %s\nMAC: %s", 
                 WiFi.softAPSSID().c_str(), 
                 WiFi.softAPIP().toString().c_str(), 
                 WiFi.softAPmacAddress().c_str());
    } else {
        snprintf(infoBuf, sizeof(infoBuf), 
                 "SSID: %s\nIP: %s\nMAC: %s\nRSSI: %d dBm", 
                 WiFi.SSID().c_str(), 
                 WiFi.localIP().toString().c_str(), 
                 WiFi.macAddress().c_str(), 
                 WiFi.RSSI());
    }
    lv_label_set_text(lbl_info, infoBuf);
    lv_obj_set_style_text_font(lbl_info, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_align(lbl_info, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(lbl_info, lv_color_hex(COLOR_TEXT), 0);
    lv_obj_align(lbl_info, LV_ALIGN_CENTER, 0, isLargeScreen ? 10 : 15);

    // Close Button
    lv_obj_t * btn_close = lv_btn_create(wifi_info_dialog);
    lv_obj_set_size(btn_close, isLargeScreen ? 120 : 80, isLargeScreen ? 40 : 30);
    lv_obj_align(btn_close, LV_ALIGN_BOTTOM_MID, 0, isLargeScreen ? -15 : -5);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(COLOR_OVERLAY), 0);
    lv_obj_add_event_cb(btn_close, close_wifi_info_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t * lbl_close = lv_label_create(btn_close);
    lv_label_set_text(lbl_close, "Close");
    lv_obj_set_style_text_color(lbl_close, lv_color_hex(COLOR_CRUST), 0);
    lv_obj_set_style_text_font(lbl_close, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, 0);
    lv_obj_align(lbl_close, LV_ALIGN_CENTER, 0, 0);
}
static lv_obj_t *temp_label;
static lv_obj_t *hum_label;
static lv_obj_t *wind_label;
static lv_obj_t *status_lbl;
static lv_obj_t *status_icon_lbl;
static lv_obj_t *time_label;
static lv_obj_t *icon_lbl;
static lv_obj_t *tz_val_label;
static lv_obj_t *footer_label;
static lv_obj_t *local_sensor_cnt = nullptr;
static lv_obj_t *local_sensor_lbl = nullptr;
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

static void local_sensor_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setLocalSensorEnabled(is_checked);
    settings_local_sensor_changed = true;
}

static void local_sensor_type_dropdown_event_cb(lv_event_t * e) {
    lv_obj_t * dropdown = lv_event_get_target(e);
    int selected = lv_dropdown_get_selected(dropdown);
    int type = selected + 1;
    settings.setLocalSensorType(type);
    settings_local_sensor_changed = true;
}

static void local_sensor_interval_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    int value = lv_slider_get_value(slider);
    settings.setLocalSensorUpdateInterval(value);
    settings_local_sensor_changed = true;
}

static void temp_offset_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    float value = (float)lv_slider_get_value(slider) / 10.0f;
    settings.setLocalSensorTempOffset(value);
    settings_local_sensor_changed = true;
}

static void hum_offset_slider_event_cb(lv_event_t * e) {
    lv_obj_t * slider = lv_event_get_target(e);
    float value = (float)lv_slider_get_value(slider);
    settings.setLocalSensorHumOffset(value);
    settings_local_sensor_changed = true;
}


static void sleep_sw_event_cb(lv_event_t * e) {
    lv_obj_t * obj = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(obj, LV_STATE_CHECKED);
    settings.setSleepScheduleEnabled(is_checked);
}

static void screensaver_sw_event_cb(lv_event_t * e) {
    lv_obj_t * sw = lv_event_get_target(e);
    bool is_checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    settings.setScreensaverEnabled(is_checked);
#ifndef NATIVE_TEST
    if (!is_checked && screensaver.isActive()) {
        screensaver.wake(settings.getBrightness());
    }
#endif
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

struct TimezonePreset {
    const char* label;
    const char* value;
};
static const TimezonePreset tz_presets[] = {
    {"UTC", "UTC0"},
    {"London", "GMT0BST,M3.5.0/1,M10.5.0"},
    {"CET", "CET-1CEST,M3.5.0,M10.5.0/3"},
    {"EET", "EET-2EEST,M3.5.0/3,M10.5.0/4"},
    {"US East", "EST5EDT,M3.2.0,M11.1.0"},
    {"US Central", "CST6CDT,M3.2.0,M11.1.0"},
    {"US Mount.", "MST7MDT,M3.2.0,M11.1.0"},
    {"US Pacific", "PST8PDT,M3.2.0,M11.1.0"},
    {"US Alaska", "AKST9AKDT,M3.2.0,M11.1.0"},
    {"US Hawaii", "HST10"},
    {"AU East", "AEST-10AEDT,M10.1.0,M4.1.0/3"},
    {"AU Central", "ACST-9:30ACDT,M10.1.0,M4.1.0/3"},
    {"AU West", "AWST-8"}
};
static const int num_tz_presets = sizeof(tz_presets)/sizeof(tz_presets[0]);

static void tz_btn_event_cb(lv_event_t * e) {
    intptr_t dir = (intptr_t)lv_event_get_user_data(e);
    
    int current_idx = 0;
    String current_tz = settings.getTimezone();
    for (int i = 0; i < num_tz_presets; i++) {
        if (current_tz == tz_presets[i].value) {
            current_idx = i;
            break;
        }
    }
    
    current_idx += dir;
    if (current_idx < 0) current_idx = num_tz_presets - 1;
    if (current_idx >= num_tz_presets) current_idx = 0;
    
    settings.setTimezone(tz_presets[current_idx].value);
    settings_timezone_changed = true;
    lv_label_set_text(tz_val_label, tz_presets[current_idx].label);
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
            } else {
                lv_snprintf(dsc->text, dsc->text_length, "+%dh", hour_idx);
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
    int screen_w = lv_disp_get_hor_res(NULL);
    int screen_h = lv_disp_get_ver_res(NULL);
    int header_h = isLandscape ? 45 : 60;
    bool isLargeScreen = (lv_disp_get_hor_res(NULL) >= 480 || lv_disp_get_ver_res(NULL) >= 480); // 60px in portrait to fit 2-line title

    lv_obj_t * header = lv_obj_create(scr);
    lv_obj_set_size(header, screen_w, header_h);
    lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(header, lv_color_hex(COLOR_CRUST), LV_PART_MAIN); // Crust header background
    lv_obj_set_style_border_width(header, 0, LV_PART_MAIN);
    lv_obj_set_style_radius(header, 0, LV_PART_MAIN);
    lv_obj_clear_flag(header, LV_OBJ_FLAG_SCROLLABLE);

    // Header Title (Time/App Name)
    header_title = lv_label_create(header);
#ifdef APP_VERSION
    lv_label_set_text(header_title, isLandscape ? "CYD Weather Station\n" APP_VERSION : "CYD Weather\nStation\n" APP_VERSION);
#else
    lv_label_set_text(header_title, isLandscape ? "CYD Weather Station" : "CYD Weather\nStation");
#endif
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
    lv_obj_add_flag(wifi_label, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_ext_click_area(wifi_label, 15);
    lv_obj_add_event_cb(wifi_label, wifi_icon_click_cb, LV_EVENT_CLICKED, NULL);

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
    lv_obj_t * tab_curr = lv_tabview_add_tab(tabview, isLandscape ? "Current" : "Now");
    lv_obj_t * tab_fore = lv_tabview_add_tab(tabview, isLandscape ? "Forecast" : "Fore");
    lv_obj_t * tab_hourly = lv_tabview_add_tab(tabview, isLandscape ? "Hourly" : "Hour");
    lv_obj_t * tab_settings = lv_tabview_add_tab(tabview, isLandscape ? "Settings" : "Setup");

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
    lv_obj_set_scrollbar_mode(tab_settings, LV_SCROLLBAR_MODE_AUTO);

    // Create Hourly Forecast Chart
    hourly_chart = lv_chart_create(tab_hourly);
    if (isLandscape) {
        lv_obj_set_size(hourly_chart, lv_pct(85), lv_pct(90));
    } else {
        lv_obj_set_size(hourly_chart, lv_pct(80), lv_pct(90));
    }
    lv_obj_align(hourly_chart, LV_ALIGN_CENTER, 0, -10);
    lv_chart_set_type(hourly_chart, LV_CHART_TYPE_LINE);

    // Styling Catppuccin
    lv_obj_set_style_bg_color(hourly_chart, lv_color_hex(COLOR_BASE), LV_PART_MAIN);
    lv_obj_set_style_border_width(hourly_chart, 0, LV_PART_MAIN);
    lv_obj_set_style_line_color(hourly_chart, lv_color_hex(COLOR_OVERLAY), LV_PART_ITEMS); // grid lines
    lv_obj_set_style_text_color(hourly_chart, lv_color_hex(COLOR_TEXT), LV_PART_TICKS); // axis tick text
    lv_obj_set_style_text_font(hourly_chart, isLargeScreen ? &lv_font_montserrat_14 : &lv_font_montserrat_10, LV_PART_TICKS);
    lv_obj_set_style_line_rounded(hourly_chart, true, LV_PART_ITEMS);
    lv_obj_set_style_line_width(hourly_chart, 3, LV_PART_ITEMS);

    // Number of points is 24 (one for each hour)
    lv_chart_set_point_count(hourly_chart, 24);

    // Grid lines count
    lv_chart_set_div_line_count(hourly_chart, 5, 7); // 5 horizontal grid lines, 7 vertical grid lines

    // Enable axes labels and ticks
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_PRIMARY_X, 6, 3, 7, 2, true, isLargeScreen ? 40 : 20);
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_PRIMARY_Y, 6, 3, 5, 2, true, isLargeScreen ? 80 : 50);
    lv_chart_set_axis_tick(hourly_chart, LV_CHART_AXIS_SECONDARY_Y, 6, 3, 5, 2, true, isLargeScreen ? 85 : 55);

    // Set secondary Y range to 0 - 100
    lv_chart_set_range(hourly_chart, LV_CHART_AXIS_SECONDARY_Y, 0, 100);

    // Add event callback for tick labeling
    lv_obj_add_event_cb(hourly_chart, chart_draw_event_cb, LV_EVENT_DRAW_PART_BEGIN, NULL);

    // Add series
    hourly_temp_series = lv_chart_add_series(hourly_chart, lv_color_hex(COLOR_MAUVE), LV_CHART_AXIS_PRIMARY_Y);
    hourly_precip_series = lv_chart_add_series(hourly_chart, lv_color_hex(COLOR_BLUE), LV_CHART_AXIS_SECONDARY_Y);

    // Container for the icon to accurately center it in the remaining space
    lv_obj_t * icon_cnt = lv_obj_create(tab_curr);
    lv_obj_set_style_bg_opa(icon_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(icon_cnt, 0, LV_PART_MAIN);
    lv_obj_clear_flag(icon_cnt, LV_OBJ_FLAG_SCROLLABLE | LV_OBJ_FLAG_CLICKABLE);

    if (isLandscape) {
        lv_obj_set_size(icon_cnt, lv_pct(40), lv_pct(85));
        lv_obj_align(icon_cnt, LV_ALIGN_TOP_LEFT, 0, 0);
    } else {
        lv_obj_set_size(icon_cnt, lv_pct(100), lv_pct(30));
        lv_obj_align(icon_cnt, LV_ALIGN_TOP_MID, 0, -10);
    }

    // Weather Placeholders inside icon_cnt
    icon_lbl = lv_label_create(icon_cnt);
    if (isLargeScreen) {
        lv_obj_set_style_text_font(icon_lbl, &weather_icons_72, LV_PART_MAIN);
    } else {
        lv_obj_set_style_text_font(icon_lbl, &weather_icons_48, LV_PART_MAIN);
    }
    lv_label_set_text(icon_lbl, "\xef\x81\xbb"); // fallback NA icon (f07b)
    lv_obj_set_style_text_color(icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_align(icon_lbl, LV_ALIGN_CENTER, 0, 0);
 
    // Vertical container for details on the right side
    lv_obj_t * details_cnt = lv_obj_create(tab_curr);
    lv_obj_clear_flag(details_cnt, LV_OBJ_FLAG_CLICKABLE);
    if (isLandscape) {
        lv_obj_set_size(details_cnt, lv_pct(60), lv_pct(85));
        lv_obj_align(details_cnt, LV_ALIGN_TOP_RIGHT, 0, 0);
    } else {
        lv_obj_set_size(details_cnt, lv_pct(100), lv_pct(55));
        lv_obj_align(details_cnt, LV_ALIGN_BOTTOM_MID, 0, -20);
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
    lv_obj_clear_flag(temp_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(temp_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(temp_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(temp_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(temp_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(temp_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(temp_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(temp_cnt, 4, LV_PART_MAIN); // reduced spacing
    lv_obj_clear_flag(temp_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * temp_icon_lbl = lv_label_create(temp_cnt);
    lv_obj_set_style_text_font(temp_icon_lbl, isLargeScreen ? &weather_icons_24 : &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(temp_icon_lbl, "\xef\x81\x95"); // U+F055 wi-thermometer
    lv_obj_set_style_text_color(temp_icon_lbl, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_set_width(temp_icon_lbl, isLargeScreen ? 30 : 20);
    lv_obj_set_style_text_align(temp_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    temp_label = lv_label_create(temp_cnt);
    if (settings.getUnitSystem() == UNIT_IMPERIAL) {
        lv_label_set_text(temp_label, "--.- °F");
    } else {
        lv_label_set_text(temp_label, "--.- °C");
    }
    lv_obj_set_style_text_font(temp_label, isLargeScreen ? &lv_font_montserrat_48 : &lv_font_montserrat_28, LV_PART_MAIN);
    lv_obj_set_style_text_color(temp_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);

    // --- Humidity row (wi-humidity + value) ---
    lv_obj_t * hum_cnt = lv_obj_create(details_cnt);
    lv_obj_clear_flag(hum_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(hum_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(hum_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(hum_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(hum_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(hum_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(hum_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(hum_cnt, 4, LV_PART_MAIN);
    lv_obj_clear_flag(hum_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * hum_icon_lbl = lv_label_create(hum_cnt);
    lv_obj_set_style_text_font(hum_icon_lbl, isLargeScreen ? &weather_icons_24 : &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(hum_icon_lbl, "\xef\x81\xba"); // U+F07A wi-humidity
    lv_obj_set_style_text_color(hum_icon_lbl, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);
    lv_obj_set_width(hum_icon_lbl, isLargeScreen ? 30 : 20);
    lv_obj_set_style_text_align(hum_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    hum_label = lv_label_create(hum_cnt);
    lv_obj_set_style_text_font(hum_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(hum_label, "--%");
    lv_obj_set_style_text_color(hum_label, lv_color_hex(COLOR_BLUE), LV_PART_MAIN);

    // --- Wind row (wi-windy + value) ---
    lv_obj_t * wind_cnt = lv_obj_create(details_cnt);
    lv_obj_clear_flag(wind_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(wind_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(wind_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(wind_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(wind_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(wind_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(wind_cnt, 4, LV_PART_MAIN); // horizontal space between icon and text
    lv_obj_clear_flag(wind_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * wind_icon_lbl = lv_label_create(wind_cnt);
    lv_obj_set_style_text_font(wind_icon_lbl, isLargeScreen ? &weather_icons_24 : &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(wind_icon_lbl, "\xef\x80\xa1"); // U+F021 (wi-windy)
    lv_obj_set_style_text_color(wind_icon_lbl, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);
    lv_obj_set_width(wind_icon_lbl, isLargeScreen ? 30 : 20);
    lv_obj_set_style_text_align(wind_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    wind_label = lv_label_create(wind_cnt);
    lv_obj_set_style_text_font(wind_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(wind_label, "Wind: -- km/h");
    lv_obj_set_style_text_color(wind_label, lv_color_hex(COLOR_LAVENDER), LV_PART_MAIN);

    // --- Status row (dynamic weather-code icon + description text) ---
    lv_obj_t * status_cnt = lv_obj_create(details_cnt);
    lv_obj_clear_flag(status_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(status_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(status_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(status_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(status_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(status_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(status_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_column(status_cnt, 4, LV_PART_MAIN);
    lv_obj_clear_flag(status_cnt, LV_OBJ_FLAG_SCROLLABLE);

    status_icon_lbl = lv_label_create(status_cnt);
    lv_obj_set_style_text_font(status_icon_lbl, isLargeScreen ? &weather_icons_24 : &weather_icons_16, LV_PART_MAIN); // 16px icon
    lv_label_set_text(status_icon_lbl, "\xef\x81\xbb"); // U+F07B wi-na (updated on fetch)
    lv_obj_set_style_text_color(status_icon_lbl, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_width(status_icon_lbl, isLargeScreen ? 30 : 20);
    lv_obj_set_style_text_align(status_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    status_lbl = lv_label_create(status_cnt);
    lv_obj_set_style_text_font(status_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(status_lbl, "Updating...");
    lv_obj_set_style_text_color(status_lbl, lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);

    // --- Local Sensor row (wi-thermometer + text) ---
    local_sensor_cnt = lv_obj_create(details_cnt);
    lv_obj_clear_flag(local_sensor_cnt, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(local_sensor_cnt, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(local_sensor_cnt, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(local_sensor_cnt, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(local_sensor_cnt, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(local_sensor_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(local_sensor_cnt, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_top(local_sensor_cnt, 4, LV_PART_MAIN);
    lv_obj_set_style_pad_column(local_sensor_cnt, 4, LV_PART_MAIN);
    lv_obj_clear_flag(local_sensor_cnt, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * loc_icon_lbl = lv_label_create(local_sensor_cnt);
    lv_obj_set_style_text_font(loc_icon_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(loc_icon_lbl, LV_SYMBOL_HOME);
    lv_obj_set_style_text_color(loc_icon_lbl, lv_color_hex(COLOR_GREEN), LV_PART_MAIN);
    lv_obj_set_width(loc_icon_lbl, isLargeScreen ? 30 : 20);
    lv_obj_set_style_text_align(loc_icon_lbl, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);

    local_sensor_lbl = lv_label_create(local_sensor_cnt);
    lv_obj_set_style_text_font(local_sensor_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
    lv_label_set_text(local_sensor_lbl, "Local: --.-° / --%");
    lv_obj_set_style_text_color(local_sensor_lbl, lv_color_hex(COLOR_GREEN), LV_PART_MAIN);
    
    if (!settings.getLocalSensorEnabled()) {
        lv_obj_add_flag(local_sensor_cnt, LV_OBJ_FLAG_HIDDEN);
    }

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
            lv_obj_set_size(day_card, lv_pct(31), lv_pct(90));
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
            lv_obj_set_style_text_font(fore_day_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_day_label[i], lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
            lv_obj_align(fore_day_label[i], LV_ALIGN_TOP_MID, 0, 5);

            // 2. Weather Icon Label
            fore_icon_label[i] = lv_label_create(day_card);
            lv_obj_set_style_text_font(fore_icon_label[i], isLargeScreen ? &weather_icons_48 : &weather_icons_24, LV_PART_MAIN);
            lv_label_set_text(fore_icon_label[i], "\xef\x81\xbb"); // fallback NA
            lv_obj_set_style_text_color(fore_icon_label[i], lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
            lv_obj_align(fore_icon_label[i], LV_ALIGN_CENTER, 0, -5);

            // 3. Temp Label (High / Low)
            fore_temp_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_temp_label[i], "--°/--°");
            lv_obj_set_style_text_font(fore_temp_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_temp_label[i], lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
            lv_obj_align(fore_temp_label[i], LV_ALIGN_BOTTOM_MID, 0, isLargeScreen ? -40 : -22);

            // 4. Status Description Label
            fore_desc_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_desc_label[i], "Loading...");
            lv_obj_set_style_text_font(fore_desc_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_desc_label[i], lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
            lv_label_set_long_mode(fore_desc_label[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_width(fore_desc_label[i], lv_pct(95));
            lv_obj_set_style_text_align(fore_desc_label[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_align(fore_desc_label[i], LV_ALIGN_BOTTOM_MID, 0, -4);
        } else {
            // Portrait layout: row flow, vertical list
            int card_w = isLargeScreen ? 300 : 220;
            int card_h = isLargeScreen ? 70 : 42;
            int card_step = isLargeScreen ? 80 : 47;
            
            lv_obj_set_size(day_card, card_w, card_h);
            lv_obj_align(day_card, LV_ALIGN_TOP_MID, 0, 10 + i * card_step);
            lv_obj_set_flex_flow(day_card, LV_FLEX_FLOW_ROW);
            lv_obj_set_flex_align(day_card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
            lv_obj_set_style_pad_all(day_card, 6, LV_PART_MAIN);
            lv_obj_set_style_pad_gap(day_card, isLargeScreen ? 8 : 4, LV_PART_MAIN);

            // 1. Day Name Label
            fore_day_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_day_label[i], i == 0 ? "Today" : (i == 1 ? "Tmrw" : "Day"));
            lv_obj_set_style_text_font(fore_day_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_day_label[i], lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
            lv_obj_set_width(fore_day_label[i], isLargeScreen ? 65 : 50); // Increased from 60
            lv_obj_set_style_text_align(fore_day_label[i], LV_TEXT_ALIGN_LEFT, 0);

            // 2. Weather Icon Label
            fore_icon_label[i] = lv_label_create(day_card);
            lv_obj_set_style_text_font(fore_icon_label[i], isLargeScreen ? &weather_icons_48 : &weather_icons_24, LV_PART_MAIN);
            lv_label_set_text(fore_icon_label[i], "\xef\x81\xbb"); // fallback NA
            lv_obj_set_style_text_color(fore_icon_label[i], lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
            lv_obj_set_width(fore_icon_label[i], isLargeScreen ? 48 : 24);
            lv_obj_set_style_text_align(fore_icon_label[i], LV_TEXT_ALIGN_CENTER, 0);

            // 3. Temp Label (High / Low)
            fore_temp_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_temp_label[i], "--°/--°");
            lv_obj_set_style_text_font(fore_temp_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_temp_label[i], lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
            lv_obj_set_width(fore_temp_label[i], isLargeScreen ? 85 : 65); // Increased from 70
            lv_obj_set_style_text_align(fore_temp_label[i], LV_TEXT_ALIGN_CENTER, 0);

            // 4. Status Description Label
            fore_desc_label[i] = lv_label_create(day_card);
            lv_label_set_text(fore_desc_label[i], "Loading...");
            lv_obj_set_style_text_font(fore_desc_label[i], isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);
            lv_obj_set_style_text_color(fore_desc_label[i], lv_color_hex(COLOR_MAUVE), LV_PART_MAIN);
            lv_label_set_long_mode(fore_desc_label[i], LV_LABEL_LONG_SCROLL_CIRCULAR);
            lv_obj_set_flex_grow(fore_desc_label[i], 1);
            lv_obj_set_style_text_align(fore_desc_label[i], LV_TEXT_ALIGN_RIGHT, 0);
        }
    }

    // --- 3. Settings Tab UI Widgets ---
    // Main container inside tab_settings
    lv_obj_t * settings_grid = lv_obj_create(tab_settings);
    lv_obj_clear_flag(settings_grid, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(settings_grid, lv_pct(100), LV_SIZE_CONTENT);
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
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(left_col, isLandscape ? lv_pct(48) : lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(left_col, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(left_col, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_style_bg_opa(left_col, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(left_col, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(left_col, 2, LV_PART_MAIN);
    lv_obj_set_style_pad_gap(left_col, 4, LV_PART_MAIN);
    lv_obj_clear_flag(left_col, LV_OBJ_FLAG_SCROLLABLE);

    // Right Column — Brightness label+slider, Timezone label+buttons
    lv_obj_t * right_col = lv_obj_create(settings_grid);
    lv_obj_clear_flag(right_col, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(right_col, isLandscape ? lv_pct(48) : lv_pct(100), LV_SIZE_CONTENT);
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
    lv_obj_clear_flag(unit_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(unit_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(unit_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(unit_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(unit_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(unit_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(unit_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(unit_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * unit_label = lv_label_create(unit_row);
    lv_label_set_text(unit_label, "Unit (C/F)");
    lv_obj_set_style_text_color(unit_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(unit_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    lv_obj_t * unit_sw = lv_switch_create(unit_row);
    lv_obj_set_size(unit_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
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
    lv_obj_clear_flag(auto_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(auto_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(auto_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(auto_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(auto_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(auto_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(auto_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(auto_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * auto_label = lv_label_create(auto_row);
    lv_label_set_text(auto_label, "Auto Light");
    lv_obj_set_style_text_color(auto_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(auto_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);



    // SD Log row
    lv_obj_t * sd_row = lv_obj_create(left_col);
    lv_obj_clear_flag(sd_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(sd_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(sd_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sd_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sd_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sd_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sd_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sd_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sd_label = lv_label_create(sd_row);
    lv_label_set_text(sd_label, "SD Log");
    lv_obj_set_style_text_color(sd_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(sd_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_sd_log_sw = lv_switch_create(sd_row);
    lv_obj_set_size(ui_sd_log_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_sd_log_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sd_log_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_sd_log_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (SdCardManager::isCardPresent()) {
        if (settings.getSdLoggingEnabled()) {
            lv_obj_add_state(ui_sd_log_sw, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(ui_sd_log_sw, sd_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        lv_obj_add_state(ui_sd_log_sw, LV_STATE_DISABLED);
    }

    // SD Cache row
    lv_obj_t * sd_cache_row = lv_obj_create(left_col);
    lv_obj_clear_flag(sd_cache_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(sd_cache_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(sd_cache_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sd_cache_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sd_cache_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sd_cache_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sd_cache_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sd_cache_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * sd_cache_label = lv_label_create(sd_cache_row);
    lv_label_set_text(sd_cache_label, "SD Cache");
    lv_obj_set_style_text_color(sd_cache_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(sd_cache_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_sd_cache_sw = lv_switch_create(sd_cache_row);
    lv_obj_set_size(ui_sd_cache_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_sd_cache_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sd_cache_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_sd_cache_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (SdCardManager::isCardPresent()) {
        if (settings.getSdCacheEnabled()) {
            lv_obj_add_state(ui_sd_cache_sw, LV_STATE_CHECKED);
        }
        lv_obj_add_event_cb(ui_sd_cache_sw, sd_cache_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    } else {
        lv_obj_add_state(ui_sd_cache_sw, LV_STATE_DISABLED);
    }

    // Screenshot Server row
    lv_obj_t * scr_row = lv_obj_create(left_col);
    lv_obj_clear_flag(scr_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(scr_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(scr_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(scr_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(scr_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(scr_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(scr_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(scr_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * scr_label = lv_label_create(scr_row);
    lv_label_set_text(scr_label, "API Srv");
    lv_obj_set_style_text_color(scr_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(scr_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    lv_obj_t * scr_sw = lv_switch_create(scr_row);
    lv_obj_set_size(scr_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(scr_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getScreenshotServerEnabled()) {
        lv_obj_add_state(scr_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(scr_sw, screenshot_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Local Sensor row
    lv_obj_t * loc_sens_row = lv_obj_create(left_col);
    lv_obj_clear_flag(loc_sens_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(loc_sens_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(loc_sens_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(loc_sens_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(loc_sens_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(loc_sens_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(loc_sens_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(loc_sens_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * loc_sens_label = lv_label_create(loc_sens_row);
    lv_label_set_text(loc_sens_label, "Local Sensor");
    lv_obj_set_style_text_color(loc_sens_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(loc_sens_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_sw_local_sensor = lv_switch_create(loc_sens_row);
    lv_obj_set_size(ui_sw_local_sensor, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_sw_local_sensor, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sw_local_sensor, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_sw_local_sensor, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getLocalSensorEnabled()) {
        lv_obj_add_state(ui_sw_local_sensor, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_sw_local_sensor, local_sensor_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Screensaver row
    lv_obj_t * saver_row = lv_obj_create(left_col);
    lv_obj_clear_flag(saver_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(saver_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(saver_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(saver_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(saver_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(saver_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(saver_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(saver_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * saver_label = lv_label_create(saver_row);
    lv_label_set_text(saver_label, "Scr Saver");
    lv_obj_set_style_text_color(saver_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(saver_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_screensaver_sw = lv_switch_create(saver_row);
    lv_obj_set_size(ui_screensaver_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_screensaver_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_screensaver_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_screensaver_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getScreensaverEnabled()) {
        lv_obj_add_state(ui_screensaver_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_screensaver_sw, screensaver_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // Sleep Schedule Row
    lv_obj_t *sleep_row = lv_obj_create(left_col);
    lv_obj_clear_flag(sleep_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_width(sleep_row, LV_PCT(100));
    lv_obj_set_height(sleep_row, LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(sleep_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(sleep_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(sleep_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(sleep_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(sleep_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(sleep_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *sleep_label = lv_label_create(sleep_row);
    lv_label_set_text(sleep_label, "Sleep Schedule");
    lv_obj_set_style_text_color(sleep_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(sleep_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_sleep_sw = lv_switch_create(sleep_row);
    lv_obj_set_size(ui_sleep_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_sleep_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_sleep_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_sleep_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getSleepScheduleEnabled()) {
        lv_obj_add_state(ui_sleep_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_sleep_sw, sleep_sw_event_cb, LV_EVENT_VALUE_CHANGED, NULL);


    // MQTT row
    lv_obj_t * mqtt_row = lv_obj_create(left_col);
    lv_obj_clear_flag(mqtt_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(mqtt_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(mqtt_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(mqtt_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(mqtt_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(mqtt_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(mqtt_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(mqtt_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * mqtt_label = lv_label_create(mqtt_row);
    lv_label_set_text(mqtt_label, "MQTT");
    lv_obj_set_style_text_color(mqtt_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(mqtt_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    lv_obj_t * mqtt_sw = lv_switch_create(mqtt_row);
    lv_obj_set_size(mqtt_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
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
    lv_obj_set_style_text_font(slider_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    // Brightness slider — suppress thumb padding so it doesn't bloat height
    ui_brightness_slider = lv_slider_create(right_col);
    lv_obj_set_size(ui_brightness_slider, lv_pct(92), 14);
    lv_obj_set_style_pad_top(ui_brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(ui_brightness_slider, 4, LV_PART_KNOB);
    // Slider track (unfilled)
    lv_obj_set_style_bg_color(ui_brightness_slider, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    // Slider filled indicator
    lv_obj_set_style_bg_color(ui_brightness_slider, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    // Slider knob
    lv_obj_set_style_bg_color(ui_brightness_slider, lv_color_hex(COLOR_TEXT), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_range(ui_brightness_slider, 10, 100);
    lv_slider_set_value(ui_brightness_slider, settings.getBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_brightness_slider, brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, slider_label);
    if (settings.getAutoBrightness()) {
        lv_obj_add_state(ui_brightness_slider, LV_STATE_DISABLED);
    }

    // LED row
    lv_obj_t * led_row = lv_obj_create(left_col);
    lv_obj_clear_flag(led_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(led_row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_flex_flow(led_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(led_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(led_row, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_border_width(led_row, 0, LV_PART_MAIN);
    lv_obj_set_style_pad_all(led_row, 0, LV_PART_MAIN);
    lv_obj_clear_flag(led_row, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t * led_label_txt = lv_label_create(led_row);
    lv_label_set_text(led_label_txt, "LED");
    lv_obj_set_style_text_color(led_label_txt, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(led_label_txt, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    // --- right column LED Brightness label + slider (created first so we can pass as user_data) ---
    lv_obj_t * led_slider_label = lv_label_create(right_col);
    char led_slider_buf[32];
    int led_bright_pct = (settings.getLedBrightness() * 100) / 255;
    snprintf(led_slider_buf, sizeof(led_slider_buf), "LED: %d%%", led_bright_pct);
    lv_label_set_text(led_slider_label, led_slider_buf);
    lv_obj_set_style_text_color(led_slider_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(led_slider_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_led_brightness_slider = lv_slider_create(right_col);
    lv_obj_set_size(ui_led_brightness_slider, lv_pct(92), 14);
    lv_obj_set_style_pad_top(ui_led_brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_pad_bottom(ui_led_brightness_slider, 4, LV_PART_KNOB);
    lv_obj_set_style_bg_color(ui_led_brightness_slider, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_led_brightness_slider, lv_color_hex(COLOR_MAUVE), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_led_brightness_slider, lv_color_hex(COLOR_TEXT), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_slider_set_range(ui_led_brightness_slider, 0, 255);
    lv_slider_set_value(ui_led_brightness_slider, settings.getLedBrightness(), LV_ANIM_OFF);
    lv_obj_add_event_cb(ui_led_brightness_slider, led_brightness_slider_event_cb, LV_EVENT_VALUE_CHANGED, led_slider_label);
    if (!settings.getLedEnabled()) {
        lv_obj_add_state(ui_led_brightness_slider, LV_STATE_DISABLED);
    }

    // --- LED enable switch (created after slider so slider can be passed as user_data) ---
    ui_led_sw = lv_switch_create(led_row);
    lv_obj_set_size(ui_led_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_led_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_led_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_led_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getLedEnabled()) {
        lv_obj_add_state(ui_led_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_led_sw, led_sw_event_cb, LV_EVENT_VALUE_CHANGED, ui_led_brightness_slider);

    // Auto switch (created after slider so we can pass slider as user_data)
    ui_auto_bright_sw = lv_switch_create(auto_row);
    lv_obj_set_size(ui_auto_bright_sw, isLargeScreen ? 60 : 40, isLargeScreen ? 30 : 20);
    lv_obj_set_style_bg_color(ui_auto_bright_sw, lv_color_hex(COLOR_OVERLAY), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui_auto_bright_sw, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(ui_auto_bright_sw, lv_color_hex(COLOR_CRUST), LV_PART_KNOB | LV_STATE_DEFAULT);
    if (settings.getAutoBrightness()) {
        lv_obj_add_state(ui_auto_bright_sw, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(ui_auto_bright_sw, auto_sw_event_cb, LV_EVENT_VALUE_CHANGED, ui_brightness_slider);

    // Timezone label
    lv_obj_t * tz_label = lv_label_create(right_col);
    lv_label_set_text(tz_label, "Timezone");
    lv_obj_set_style_text_color(tz_label, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(tz_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    // Timezone +/- row
    lv_obj_t * tz_row = lv_obj_create(right_col);
    lv_obj_clear_flag(tz_row, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(tz_row, lv_pct(100), LV_SIZE_CONTENT);
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
    const char* init_tz_label = "Custom";
    String current_tz = settings.getTimezone();
    for (int i = 0; i < num_tz_presets; i++) {
        if (current_tz == tz_presets[i].value) {
            init_tz_label = tz_presets[i].label;
            break;
        }
    }
    lv_label_set_text(tz_val_label, init_tz_label);
    lv_obj_set_style_text_color(tz_val_label, lv_color_hex(COLOR_PEACH), LV_PART_MAIN);
    lv_obj_set_style_text_font(tz_val_label, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

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
    lv_obj_set_size(theme_dropdown, lv_pct(92), 26);
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
    lv_obj_set_size(orientation_dropdown, lv_pct(92), 26);
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

    // Local Sensor Type Dropdown
    ui_dd_local_sensor_type = lv_dropdown_create(right_col);
    lv_obj_set_size(ui_dd_local_sensor_type, lv_pct(92), 26);
    lv_dropdown_set_options(ui_dd_local_sensor_type, "DHT22\nSHT40\nDHT11");
    lv_dropdown_set_selected(ui_dd_local_sensor_type, settings.getLocalSensorType() - 1);
    lv_obj_add_event_cb(ui_dd_local_sensor_type, local_sensor_type_dropdown_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(ui_dd_local_sensor_type, lv_color_hex(COLOR_CRUST), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui_dd_local_sensor_type, lv_color_hex(COLOR_TEXT), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui_dd_local_sensor_type, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui_dd_local_sensor_type, 1, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_t * loc_type_list = lv_dropdown_get_list(ui_dd_local_sensor_type);
    lv_obj_set_style_bg_color(loc_type_list, lv_color_hex(COLOR_CRUST), LV_PART_MAIN);
    lv_obj_set_style_text_color(loc_type_list, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_border_color(loc_type_list, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(loc_type_list, lv_color_hex(COLOR_BLUE), LV_PART_SELECTED | LV_STATE_CHECKED);
    lv_obj_set_style_text_color(loc_type_list, lv_color_hex(COLOR_CRUST), LV_PART_SELECTED | LV_STATE_CHECKED);

    // Local Sensor Update Interval Slider
    lv_obj_t * loc_sens_slider_lbl = lv_label_create(right_col);
    lv_label_set_text(loc_sens_slider_lbl, "Sensor Update Interval (s)");
    lv_obj_set_style_text_color(loc_sens_slider_lbl, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(loc_sens_slider_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_slider_local_sensor_update_interval = lv_slider_create(right_col);
    lv_slider_set_range(ui_slider_local_sensor_update_interval, 1, 120);
    lv_slider_set_value(ui_slider_local_sensor_update_interval, settings.getLocalSensorUpdateInterval(), LV_ANIM_OFF);
    lv_obj_set_size(ui_slider_local_sensor_update_interval, lv_pct(92), isLargeScreen ? 20 : 10);
    lv_obj_add_event_cb(ui_slider_local_sensor_update_interval, local_sensor_interval_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(ui_slider_local_sensor_update_interval, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_slider_local_sensor_update_interval, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui_slider_local_sensor_update_interval, lv_color_hex(COLOR_CRUST), LV_PART_KNOB);

    // Temperature Offset Slider
    lv_obj_t * temp_offset_lbl = lv_label_create(right_col);
    lv_label_set_text(temp_offset_lbl, "Temperature Offset");
    lv_obj_set_style_text_color(temp_offset_lbl, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(temp_offset_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_slider_temp_offset = lv_slider_create(right_col);
    lv_slider_set_range(ui_slider_temp_offset, -100, 100); // -10.0 to 10.0
    lv_slider_set_value(ui_slider_temp_offset, (int)(settings.getLocalSensorTempOffset() * 10), LV_ANIM_OFF);
    lv_obj_set_size(ui_slider_temp_offset, lv_pct(92), isLargeScreen ? 20 : 10);
    lv_obj_add_event_cb(ui_slider_temp_offset, temp_offset_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(ui_slider_temp_offset, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_slider_temp_offset, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui_slider_temp_offset, lv_color_hex(COLOR_CRUST), LV_PART_KNOB);

    // Humidity Offset Slider
    lv_obj_t * hum_offset_lbl = lv_label_create(right_col);
    lv_label_set_text(hum_offset_lbl, "Humidity Offset");
    lv_obj_set_style_text_color(hum_offset_lbl, lv_color_hex(COLOR_TEXT), LV_PART_MAIN);
    lv_obj_set_style_text_font(hum_offset_lbl, isLargeScreen ? &lv_font_montserrat_20 : &lv_font_montserrat_14, LV_PART_MAIN);

    ui_slider_hum_offset = lv_slider_create(right_col);
    lv_slider_set_range(ui_slider_hum_offset, -20, 20); // -20 to 20
    lv_slider_set_value(ui_slider_hum_offset, (int)settings.getLocalSensorHumOffset(), LV_ANIM_OFF);
    lv_obj_set_size(ui_slider_hum_offset, lv_pct(92), isLargeScreen ? 20 : 10);
    lv_obj_add_event_cb(ui_slider_hum_offset, hum_offset_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_set_style_bg_color(ui_slider_hum_offset, lv_color_hex(COLOR_OVERLAY), LV_PART_MAIN);
    lv_obj_set_style_bg_color(ui_slider_hum_offset, lv_color_hex(COLOR_BLUE), LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(ui_slider_hum_offset, lv_color_hex(COLOR_CRUST), LV_PART_KNOB);

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

void updateLocalSensorUI(float temperature, float humidity) {
    if (local_sensor_lbl != nullptr) {
        char local_str[64];
        if (settings.getUnitSystem() == UNIT_IMPERIAL) {
            snprintf(local_str, sizeof(local_str), "Local: %.1f °F / %.0f%%", temperature, humidity);
        } else {
            snprintf(local_str, sizeof(local_str), "Local: %.1f °C / %.0f%%", temperature, humidity);
        }
        lv_label_set_text(local_sensor_lbl, local_str);
    }
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

static std::string sanitizeCityName(const char* input) {
    if (!input) return "";
    std::string result = "";
    int i = 0;
    while (input[i] != '\0') {
        unsigned char c = input[i];
        if (c < 128) {
            result += (char)c;
            i++;
        } else if (c == 0xC3) {
            unsigned char next = input[i+1];
            if (next == '\0') break;
            if (next == 0xA1 || next == 0xA2 || next == 0xA4 || next == 0xA5) result += 'a';
            else if (next == 0x81 || next == 0x84) result += 'A';
            else if (next == 0xA6) result += "ae";
            else if (next == 0xA7) result += 'c';
            else if (next == 0xA9 || next == 0xAA || next == 0xAB) result += 'e';
            else if (next == 0x89) result += 'E';
            else if (next == 0xAD) result += 'i';
            else if (next == 0xB1) result += 'n';
            else if (next == 0xB2 || next == 0xB3 || next == 0xB4 || next == 0xB6 || next == 0xB8) result += 'o';
            else if (next == 0x93 || next == 0x96) result += 'O';
            else if (next == 0xBA || next == 0xBC) result += 'u';
            else if (next == 0x9A || next == 0x9C) result += 'U';
            else if (next == 0x9F) result += "ss";
            else result += '?';
            i += 2;
        } else if (c == 0xC4) {
            unsigned char next = input[i+1];
            if (next == '\0') break;
            if (next == 0x80) result += 'A';
            else if (next == 0x81) result += 'a';
            else if (next == 0x8C) result += 'C';
            else if (next == 0x8D) result += 'c';
            else if (next == 0x92) result += 'E';
            else if (next == 0x93) result += 'e';
            else if (next == 0xA2) result += 'G';
            else if (next == 0xA3) result += 'g';
            else if (next == 0xAA) result += 'I';
            else if (next == 0xAB) result += 'i';
            else result += '?';
            i += 2;
        } else if (c == 0xC5) {
            unsigned char next = input[i+1];
            if (next == '\0') break;
            if (next == 0x8A) result += 'K';
            else if (next == 0x8B) result += 'k';
            else if (next == 0x92) result += 'L';
            else if (next == 0x93) result += 'l';
            else if (next == 0xA0) result += 'N';
            else if (next == 0xA1) result += 'n';
            else if (next == 0xA6) result += 'S';
            else if (next == 0xA7) result += 's';
            else if (next == 0xAA) result += 'U';
            else if (next == 0xAB) result += 'u';
            else if (next == 0xBD) result += 'Z';
            else if (next == 0xBE) result += 'z';
            else result += '?';
            i += 2;
        } else {
            result += '?';
            i++;
            while (input[i] != '\0' && (input[i] & 0xC0) == 0x80) {
                i++;
            }
        }
    }
    return result;
}

void updateFooterUI(const char* update_time, const char* city) {
    char buf[64];
    if (city && city[0] != '\0') {
        std::string clean_city = sanitizeCityName(city);
        snprintf(buf, sizeof(buf), "Last Update: %s | %s", update_time, clean_city.c_str());
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

void ui_sync_toggles() {
    if (ui_auto_bright_sw != nullptr) {
        if (settings.getAutoBrightness()) {
            lv_obj_add_state(ui_auto_bright_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_auto_bright_sw, LV_STATE_CHECKED);
        }
    }
    
    if (ui_sleep_sw != nullptr) {
        if (settings.getSleepScheduleEnabled()) {
            lv_obj_add_state(ui_sleep_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_sleep_sw, LV_STATE_CHECKED);
        }
    }

    if (ui_screensaver_sw != nullptr) {
        if (settings.getScreensaverEnabled()) {
            lv_obj_add_state(ui_screensaver_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_screensaver_sw, LV_STATE_CHECKED);
        }
    }
    
    if (ui_led_sw != nullptr) {
        if (settings.getLedEnabled()) {
            lv_obj_add_state(ui_led_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_led_sw, LV_STATE_CHECKED);
        }
    }
    
    if (ui_sd_log_sw != nullptr) {
        if (settings.getSdLoggingEnabled()) {
            lv_obj_add_state(ui_sd_log_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_sd_log_sw, LV_STATE_CHECKED);
        }
    }
    
    if (ui_sd_cache_sw != nullptr) {
        if (settings.getSdCacheEnabled()) {
            lv_obj_add_state(ui_sd_cache_sw, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_sd_cache_sw, LV_STATE_CHECKED);
        }
    }
    
    if (ui_sw_local_sensor != nullptr) {
        if (settings.getLocalSensorEnabled()) {
            lv_obj_add_state(ui_sw_local_sensor, LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(ui_sw_local_sensor, LV_STATE_CHECKED);
        }
    }

    if (ui_dd_local_sensor_type != nullptr) {
        lv_dropdown_set_selected(ui_dd_local_sensor_type, settings.getLocalSensorType() - 1);
    }
    
    if (ui_slider_local_sensor_update_interval != nullptr) {
        lv_slider_set_value(ui_slider_local_sensor_update_interval, settings.getLocalSensorUpdateInterval(), LV_ANIM_OFF);
    }

    if (ui_slider_temp_offset != nullptr) {
        lv_slider_set_value(ui_slider_temp_offset, (int)(settings.getLocalSensorTempOffset() * 10), LV_ANIM_OFF);
    }

    if (ui_slider_hum_offset != nullptr) {
        lv_slider_set_value(ui_slider_hum_offset, (int)settings.getLocalSensorHumOffset(), LV_ANIM_OFF);
    }
    
    if (ui_brightness_slider != nullptr) {
        lv_slider_set_value(ui_brightness_slider, settings.getBrightness(), LV_ANIM_OFF);
        if (settings.getAutoBrightness()) {
            lv_obj_add_state(ui_brightness_slider, LV_STATE_DISABLED);
        } else {
            lv_obj_clear_state(ui_brightness_slider, LV_STATE_DISABLED);
        }
    }
    
    if (ui_led_brightness_slider != nullptr) {
        lv_slider_set_value(ui_led_brightness_slider, settings.getLedBrightness(), LV_ANIM_OFF);
        if (settings.getLedEnabled()) {
            lv_obj_clear_state(ui_led_brightness_slider, LV_STATE_DISABLED);
        } else {
            lv_obj_add_state(ui_led_brightness_slider, LV_STATE_DISABLED);
        }
    }

    if (local_sensor_cnt != nullptr) {
        if (settings.getLocalSensorEnabled()) {
            lv_obj_clear_flag(local_sensor_cnt, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(local_sensor_cnt, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

