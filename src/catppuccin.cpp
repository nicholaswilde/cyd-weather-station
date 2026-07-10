#include "catppuccin.h"
#include "settings_manager.h"

extern SettingsManager settings;

static const CatppuccinColors mocha = {
    0x1e1e2e, 0x181825, 0x11111b, 0xcdd6f4, 0x6c7086,
    0x89b4fa, 0xa6e3a1, 0xf38ba8, 0xf9e2af, 0xfab387,
    0xcba6f7, 0xb4befe, 0xcdd6f4
};

static const CatppuccinColors macchiato = {
    0x24273a, 0x1e2030, 0x181926, 0xcad3f5, 0x6e738d,
    0x8aadf4, 0xa6da95, 0xed8796, 0xeed49f, 0xf5a97f,
    0xc6a0f6, 0xb7bdf8, 0xcad3f5
};

static const CatppuccinColors frappe = {
    0x303446, 0x292c3c, 0x232634, 0xc6d0f5, 0x737994,
    0x8caaee, 0xa6d189, 0xe78284, 0xe5c890, 0xef9f76,
    0xca9ee6, 0xbabbf1, 0xc6d0f5
};

static const CatppuccinColors latte = {
    0xeff1f5, 0xe6e9ef, 0xdce0e8, 0x4c4f69, 0x9ca0b0,
    0x1e66f5, 0x40a02b, 0xd20f39, 0xdf8e1d, 0xfe640b,
    0x8839ef, 0x7287fd, 0x4c4f69
};

const CatppuccinColors& getCatppuccinFlavor(int flavor) {
    switch (flavor) {
        case CATPPUCCIN_MACCHIATO: return macchiato;
        case CATPPUCCIN_FRAPPE:    return frappe;
        case CATPPUCCIN_LATTE:     return latte;
        case CATPPUCCIN_MOCHA:
        default:                   return mocha;
    }
}

int getCurrentThemeFlavor() {
    return settings.getThemeFlavor();
}
