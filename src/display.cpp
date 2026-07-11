#include "display.h"
#include <Arduino.h>
#include <SPI.h>
#include "config/config.h"
#include "screenshot_manager.h"

TFT_eSPI tft = TFT_eSPI();
SPIClass touchscreenSPI(HSPI); // HSPI is used for the touch SPI bus on CYD
XPT2046_Touchscreen ts(XPT2046_CS, XPT2046_IRQ);

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

// Touch read callback for LVGL input
static void my_touchpad_read(lv_indev_drv_t *indev_drv, lv_indev_data_t *data) {
    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        
        // Calibration values for landscape (rotation = 1)
        // Adjust these mapped raw values to coordinate pixels on ESP32-2432S028R
        int16_t x = map(p.x, 200, 3700, 0, 320);
        int16_t y = map(p.y, 200, 3900, 0, 240);
        
        if (x < 0) x = 0;
        if (x >= 320) x = 319;
        if (y < 0) y = 0;
        if (y >= 240) y = 239;

        data->state = LV_INDEV_STATE_PR;
        data->point.x = x;
        data->point.y = y;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}

void initDisplayAndTouch() {
    // NOTE: TFT_BL (GPIO 21) is intentionally NOT set up here via digitalWrite.
    // BacklightManager::begin() configures it exclusively as an LEDC PWM output.
    // Setting it as a digital GPIO output first would prevent LEDC from controlling
    // brightness, causing the screen to be stuck at full brightness.

    // Initialize display
    tft.init();
    tft.setRotation(1); // Landscape
    tft.fillScreen(TFT_BLACK);

    // Initialize touch SPI and controller
    touchscreenSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
    ts.begin(touchscreenSPI);
    ts.setRotation(1); // Set to match display
}

void initLVGL() {
    lv_init();
    lv_disp_draw_buf_init(&draw_buf, buf, NULL, 320 * 10);

    // Initialize display driver
    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = 320;
    disp_drv.ver_res = 240;
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
