#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>

extern TFT_eSPI tft;
extern XPT2046_Touchscreen ts;

void initDisplayAndTouch();
void initLVGL();
void mapTouchCoordinates(int16_t raw_x, int16_t raw_y, int16_t& out_x, int16_t& out_y, int orientation);

#endif // DISPLAY_H
