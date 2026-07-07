#ifndef DISPLAY_H
#define DISPLAY_H

#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>

extern TFT_eSPI tft;
extern XPT2046_Touchscreen ts;

void initDisplayAndTouch();
void initLVGL();

#endif // DISPLAY_H
