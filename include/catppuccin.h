#ifndef CATPPUCCIN_H
#define CATPPUCCIN_H

#include <CatppuccinRGB565.h>
#include <lvgl.h>

// Catppuccin Flavor Definitions for backwards compatibility
#define CATPPUCCIN_MOCHA      1
#define CATPPUCCIN_MACCHIATO  2
#define CATPPUCCIN_FRAPPE     3
#define CATPPUCCIN_LATTE      4

const Catppuccin::Palette& getCatppuccinFlavor(int flavor);

#endif // CATPPUCCIN_H
