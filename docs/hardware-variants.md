# CYD Hardware Variants & Controller Guide

This document tracks hardware differences, controller variations, and troubleshooting findings across different manufacturers of the ESP32 Cheap Yellow Display (CYD) family, particularly boards labeled `ESP32-2432S028`.

---

## Overview

The `ESP32-2432S028` designation is used across multiple manufacturers and clone revisions. While they share the same physical form factor (2.8" display, ESP32-WROOM module, and similar breakout headers), the underlying display controller, touch digitizer, and peripheral pinouts can differ significantly.

| Model / Branding | PCB Markings | Typical Display Controller | Touch Controller | Notes |
| :--- | :--- | :--- | :--- | :--- |
| **Sunton CYD 2.8" (Resistive)** | `ESP32-2432S028R` | ILI9341 | XPT2046 (SPI) | Standard reference CYD board supported by `cyd_28r`. |
| **NM / NodeMiner / Rockbase** | `ESP32-2432S028` + Miner Logo with "NM" | ST7789 or ILI9341 clone (`ILI9341_2`) | XPT2046 or none populated | Marketed for solo lottery mining (NMMiner) or RF expansion (NM-RF-HAT). |
| **CYD 2-USB** | `ESP32-2432S028` (Micro-USB + USB-C) | ST7789 or ILI9341 | XPT2046 (SPI) | Often requires ST7789 driver with BGR color ordering and inverted colors. |
| **CYD 2.8" (Capacitive)** | `JC2432W328C` | ST7789 / ILI9341_2 | CST816 (I2C) | Capacitive touch variant supported by `cyd_28c`. |
| **CYD 3.5" (Capacitive)** | `ESP32-3248S035C` | ST7796 | GT911 / CST820 (I2C) | 3.5" 480x320 display supported by `cyd_35c`. |

---

## Known Variant Issues & Diagnostics

### 1. Partial Screen Rendering / 80px Noise Band (1/4 Screen Glitch)

#### Symptoms
* The device displays the UI across the left **240 columns**, while the remaining **80 columns** on the right show static / uninitialized RAM noise.
* Total screen width is 320 px ($240 + 80 = 320$, exactly 25% or 1/4 of the screen is static).
* UI text and tab titles may appear compressed into portrait dimensions (e.g. `Now`, `Fore`, `Hour`) even when held in landscape orientation.

#### Root Cause
1. **ST7789 Display Controller**: The board uses an ST7789 controller rather than the standard ILI9341. When an ST7789 is initialized using standard ILI9341 driver commands (`ILI9341_DRIVER`), addressing commands only cycle through the controller's 240-pixel base window, leaving the remaining 80 columns unaddressed.
2. **Alternative ILI9341 Clone Controller**: Several manufacturers use clone ILI9341 chips with non-standard initialization timing and register offsets (documented in Bodmer's `TFT_eSPI` [Issue #1172](https://github.com/Bodmer/TFT_eSPI/issues/1172)). Standard ILI9341 drivers produce incomplete rasterization or corrupted bands.

#### Solutions & Workarounds
* **Test `ILI9341_2_DRIVER`**:
  If building from source with PlatformIO, switch the driver flag:
  ```ini
  build_flags =
      ${cyd_base.build_flags}
      -D ILI9341_2_DRIVER=1
      -D TFT_WIDTH=240
      -D TFT_HEIGHT=320
      -D TFT_BL=21
  ```
* **Test `ST7789_DRIVER`**:
  If the display uses an ST7789 controller (common on NM and 2USB models), configure:
  ```ini
  build_flags =
      ${cyd_base.build_flags}
      -D ST7789_DRIVER=1
      -D TFT_WIDTH=240
      -D TFT_HEIGHT=320
      -D TFT_RGB_ORDER=TFT_BGR
      -D TFT_INVERSION_OFF=1
      -D TFT_BL=21
  ```

---

### 2. Inverted Colors

#### Symptoms
* Backgrounds that should be dark appear washed out or white.
* Text colors appear inverted (e.g., Catppuccin Mocha appears light, Latte appears dark).

#### Solution
* Flash the corresponding `_inv` release binary (e.g., `cyd_28r_inv`, `cyd_28c_inv`, `cyd_35c_inv`).
* Or add `-D TFT_INVERSION_ON=1` to your custom build flags.

---

### 3. Red & Blue Color Swapping (RGB vs BGR)

#### Symptoms
* Warm colors appear blue or cyan, while blue elements appear red or orange.

#### Solution
* Add `-D TFT_RGB_ORDER=TFT_BGR` (or toggle to `TFT_RGB`) in `platformio.ini`.

---

## References

* GitHub Issue: [#30 [feat]: Add support for additional CYD board variants](https://github.com/nicholaswilde/cyd-weather-station/issues/30)
* Reference implementation: [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display)
* Alternative driver discussion: [Bodmer/TFT_eSPI #1172](https://github.com/Bodmer/TFT_eSPI/issues/1172)
* Bruce Devices board configurations: [BruceDevices/firmware](https://github.com/pr3y/Bruce)
