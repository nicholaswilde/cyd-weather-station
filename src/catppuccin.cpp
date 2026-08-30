#include "catppuccin.h"
#include "settings_manager.h"

extern SettingsManager settings;

const Catppuccin::Palette& getCatppuccinFlavor(int flavor) {
    switch (flavor) {
        case CATPPUCCIN_MACCHIATO: return Catppuccin::getPalette(Catppuccin::Flavor::Macchiato);
        case CATPPUCCIN_FRAPPE:    return Catppuccin::getPalette(Catppuccin::Flavor::Frappe);
        case CATPPUCCIN_LATTE:     return Catppuccin::getPalette(Catppuccin::Flavor::Latte);
        case CATPPUCCIN_MOCHA:
        default:                   return Catppuccin::getPalette(Catppuccin::Flavor::Mocha);
    }
}

int getCurrentThemeFlavor() {
    return settings.getThemeFlavor();
}
