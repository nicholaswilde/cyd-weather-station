#ifndef LVGL_MOCK_H
#define LVGL_MOCK_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;
} lv_area_t;

typedef union {
    uint16_t full;
} lv_color_t;

typedef struct _lv_disp_drv_t {
    int32_t hor_res;
    int32_t ver_res;
    void (*flush_cb)(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p);
    void * draw_buf;
} lv_disp_drv_t;

typedef struct {
    int16_t x;
    int16_t y;
} lv_point_t;

typedef struct {
    uint32_t state;
    lv_point_t point;
} lv_indev_data_t;

typedef struct _lv_indev_drv_t {
    uint32_t type;
    void (*read_cb)(struct _lv_indev_drv_t * indev_drv, lv_indev_data_t * data);
} lv_indev_drv_t;

typedef struct {
    void * buf1;
} lv_disp_draw_buf_t;

inline void lv_init() {}
inline void lv_disp_draw_buf_init(lv_disp_draw_buf_t *, void *, void *, uint32_t) {}
inline void lv_disp_drv_init(lv_disp_drv_t *) {}
inline void lv_disp_drv_register(lv_disp_drv_t *) {}
inline void lv_indev_drv_init(lv_indev_drv_t *) {}
inline void lv_indev_drv_register(lv_indev_drv_t *) {}
inline void lv_disp_flush_ready(lv_disp_drv_t *) {}

#define LV_STATE_DEFAULT 0
#define LV_STATE_CHECKED 1
#define LV_INDEV_STATE_PR 1
#define LV_INDEV_STATE_REL 0
#define LV_INDEV_TYPE_POINTER 1
#endif
