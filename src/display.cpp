#include "display.h"
#include <Arduino.h>
#include <SPI.h>
#include "config/config.h"
#include "screenshot_manager.h"
#include "settings_manager.h"
#include "screensaver_manager.h"
#include "touch_manager.h"

extern SettingsManager settings;

TFT_eSPI tft = TFT_eSPI();

// LVGL buffers — size controlled by DISPLAY_DRAW_BUF_ROWS in config/config.h
static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf[320 * DISPLAY_DRAW_BUF_ROWS];

// Flush callback for LVGL display
static void my_disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p) {
    uint32_t w = (area->x2 - area->x1 + 1);
    uint32_t h = (area->y2 - area->y1 + 1);
    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)&color_p->full, w * h, true);
    tft.endWrite();

    // --- If a screenshot capture is in progress, forward each tile ---
    if (ScreenshotManager::isCaptureInProgress()) {
        ScreenshotManager::onFlushTile(
            area->x1, area->y1, area->x2, area->y2,
            (const uint16_t *)&color_p->full);
    }

    lv_disp_flush_ready(disp_drv);
}

void mapTouchCoordinates(int16_t raw_x, int16_t raw_y, int16_t& out_x, int16_t& out_y, int orientation) {
    int16_t lx, ly;
    int w_land = TFT_HEIGHT;
    int h_land = TFT_WIDTH;

    if (TouchManager::isCapacitive()) {
        bool rawIsPortrait = (raw_x <= h_land && raw_y <= w_land);
        
        if (rawIsPortrait) {
            switch (orientation) {
                case 0: // Portrait Rev / Native Portrait
                    lx = raw_x;
                    ly = raw_y;
                    break;
                case 1: // Landscape
                    lx = raw_y;
                    ly = h_land - raw_x;
                    break;
                case 2: // Portrait
                    lx = h_land - raw_x;
                    ly = w_land - raw_y;
                    break;
                case 3: // Landscape Rev
                    lx = w_land - raw_y;
                    ly = raw_x;
                    break;
                default:
                    lx = raw_y;
                    ly = raw_x;
                    break;
            }
        } else {
            switch (orientation) {
                case 0: // Portrait
                    lx = h_land - raw_y;
                    ly = w_land - raw_x;
                    break;
                case 1: // Landscape
                    lx = raw_x;
                    ly = raw_y;
                    break;
                case 2: // Portrait Rev
                    lx = raw_y;
                    ly = raw_x;
                    break;
                case 3: // Landscape Rev
                    lx = w_land - raw_x;
                    ly = h_land - raw_y;
                    break;
                default:
                    lx = raw_x;
                    ly = raw_y;
                    break;
            }
        }
        out_x = lx;
        out_y = ly;
    } else {
        // --- map to base dimensions ---
        lx = map(raw_x, 200, 3700, 0, w_land);
        ly = map(raw_y, 200, 3900, 0, h_land);
        
        // --- adjust based on orientation ---
        switch (orientation) {
            case 0: // Portrait Rev
                out_x = h_land - ly;
                out_y = lx;
                break;
            case 1: // Landscape
                out_x = lx;
                out_y = ly;
                break;
            case 2: // Portrait
                out_x = ly;
                out_y = w_land - lx;
                break;
            case 3: // Landscape Rev
                out_x = w_land - lx;
                out_y = h_land - ly;
                break;
            default:
                out_x = lx;
                out_y = ly;
                break;
        }
    }
    
    // --- bounds clamping ---
    int16_t max_x = (orientation == 1 || orientation == 3) ? w_land - 1 : h_land - 1;
    int16_t max_y = (orientation == 1 || orientation == 3) ? h_land - 1 : w_land - 1;
    
    if (out_x < 0) out_x = 0;
    if (out_x > max_x) out_x = max_x;
    if (out_y < 0) out_y = 0;
    if (out_y > max_y) out_y = max_y;
}

// Touch read callback for LVGL input
static void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    if (TouchManager::isTouched()) {
#ifndef NATIVE_TEST
        screensaver.feedActivity();
#endif
        int tx, ty;
        TouchManager::getTouchPoint(tx, ty);
        
        int16_t x = 0;
        int16_t y = 0;
        int orientation = 1;
#ifndef NATIVE_TEST
        orientation = settings.getScreenOrientation();
#endif
        mapTouchCoordinates(tx, ty, x, y, orientation);

        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void initDisplayAndTouch() {
    // NOTE: TFT_BL is intentionally NOT set up here via digitalWrite.
    // BacklightManager::begin() configures it exclusively as an LEDC PWM output.

    int rotation = 1;
#ifndef NATIVE_TEST
    rotation = settings.getScreenOrientation();
#endif

    // Initialize display
    tft.init();
    tft.setRotation(rotation);
    tft.fillScreen(TFT_BLACK);

    // Initialize TouchManager
    TouchManager::begin();
}

void initLVGL() {
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * DISPLAY_DRAW_BUF_ROWS);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    
    int rotation = 1;
#ifndef NATIVE_TEST
    rotation = settings.getScreenOrientation();
#endif
    if (rotation == 1 || rotation == 3) {
        disp_drv.hor_res = TFT_HEIGHT;
        disp_drv.ver_res = TFT_WIDTH;
    } else {
        disp_drv.hor_res = TFT_WIDTH;
        disp_drv.ver_res = TFT_HEIGHT;
    }
    
    disp_drv.flush_cb = my_disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    // Initialize input device driver
    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = my_touchpad_read;
    lv_indev_drv_register(&indev_drv);
}
