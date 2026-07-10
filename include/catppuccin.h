#ifndef CATPPUCCIN_H
#define CATPPUCCIN_H

#include <stdint.h>

// Catppuccin Flavor Definitions
#define CATPPUCCIN_MOCHA      1
#define CATPPUCCIN_MACCHIATO  2
#define CATPPUCCIN_FRAPPE     3
#define CATPPUCCIN_LATTE      4

struct CatppuccinColors {
    uint32_t base;
    uint32_t mantle;
    uint32_t crust;
    uint32_t text;
    uint32_t overlay;
    uint32_t blue;
    uint32_t green;
    uint32_t red;
    uint32_t yellow;
    uint32_t peach;
    uint32_t mauve;
    uint32_t lavender;
    uint32_t header_text;
};

const CatppuccinColors& getCatppuccinFlavor(int flavor);

#endif // CATPPUCCIN_H
