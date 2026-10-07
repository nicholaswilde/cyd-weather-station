---
version: alpha
name: CYD Weather Station
colors:
  surface: "#181825"
  surface-dim: "#11111b"
  surface-bright: "#313244"
  surface-container-lowest: "#11111b"
  surface-container-low: "#181825"
  surface-container: "#1e1e2e"
  surface-container-high: "#313244"
  surface-container-highest: "#45475a"
  on-surface: "#cdd6f4"
  on-surface-variant: "#a6adc8"
  outline: "#6c7086"
  outline-variant: "#45475a"
  primary: "#89b4fa"
  on-primary: "#11111b"
  primary-container: "#313244"
  on-primary-container: "#89b4fa"
  secondary: "#b4befe"
  on-secondary: "#11111b"
  secondary-container: "#313244"
  on-secondary-container: "#b4befe"
  tertiary: "#cba6f7"
  on-tertiary: "#11111b"
  tertiary-container: "#313244"
  on-tertiary-container: "#cba6f7"
  error: "#f38ba8"
  on-error: "#11111b"
  error-container: "#313244"
  on-error-container: "#f38ba8"
  background: "#1e1e2e"
  on-background: "#cdd6f4"
typography:
  display-xl:
    fontFamily: Montserrat
    fontSize: 48px
    fontWeight: "700"
    lineHeight: 52px
  headline-lg:
    fontFamily: Montserrat
    fontSize: 28px
    fontWeight: "600"
    lineHeight: 34px
  title-md:
    fontFamily: Montserrat
    fontSize: 20px
    fontWeight: "600"
    lineHeight: 26px
  body-md:
    fontFamily: Montserrat
    fontSize: 14px
    fontWeight: "400"
    lineHeight: 20px
  label-sm:
    fontFamily: Montserrat
    fontSize: 10px
    fontWeight: "500"
    lineHeight: 14px
rounded:
  none: 0px
  sm: 4px
  md: 8px
  lg: 12px
  full: 9999px
spacing:
  none: 0px
  xs: 2px
  sm: 4px
  md: 8px
  lg: 12px
  xl: 16px
  gutter: 8px
components:
  weather-card:
    backgroundColor: "{colors.surface}"
    textColor: "{colors.on-surface}"
    typography: "{typography.body-md}"
    rounded: "{rounded.md}"
    padding: "{spacing.md}"
  header-bar:
    backgroundColor: "{colors.surface-container-lowest}"
    textColor: "{colors.on-surface}"
    typography: "{typography.body-md}"
    padding: "{spacing.sm}"
  button-primary:
    backgroundColor: "{colors.primary}"
    textColor: "{colors.on-primary}"
    typography: "{typography.body-md}"
    rounded: "{rounded.sm}"
    padding: "{spacing.sm}"
  button-secondary:
    backgroundColor: "{colors.secondary}"
    textColor: "{colors.on-secondary}"
    typography: "{typography.body-md}"
    rounded: "{rounded.sm}"
    padding: "{spacing.sm}"
  dialog-modal:
    backgroundColor: "{colors.surface-container-low}"
    textColor: "{colors.on-surface}"
    typography: "{typography.title-md}"
    rounded: "{rounded.lg}"
    padding: "{spacing.lg}"
  setting-switch:
    backgroundColor: "{colors.surface-container-high}"
    rounded: "{rounded.full}"
    padding: "{spacing.xs}"
  setting-slider:
    backgroundColor: "{colors.surface-container-highest}"
    rounded: "{rounded.sm}"
  badge-status:
    backgroundColor: "{colors.tertiary-container}"
    textColor: "{colors.on-tertiary-container}"
    typography: "{typography.label-sm}"
    rounded: "{rounded.full}"
    padding: "{spacing.xs}"
  alert-banner:
    backgroundColor: "{colors.error-container}"
    textColor: "{colors.on-error-container}"
    typography: "{typography.body-md}"
    rounded: "{rounded.sm}"
    padding: "{spacing.sm}"
---

## Brand & Style

The CYD Weather Station design system delivers an embedded dashboard interface tailored for ESP32-powered Cheap Yellow Display (CYD) hardware. The aesthetic blends the dark, pastel-accented Catppuccin palette with clean, legible typography optimized for small TFT displays (2.8" and 3.5" form factors).

The interface prioritizes instant readability from across a room while preserving tactile touch feedback. High contrast ensures clear visibility under varied lighting conditions, while the Catppuccin color harmony eliminates visual harshness during 24/7 desktop operation.

## Colors

The color palette is built around Catppuccin Mocha as the primary dark theme, providing soft contrast against pure black TFT bezels:

- **Primary (#89b4fa - Blue):** Drives interaction, highlights active toggles, and colors primary weather metrics.
- **Secondary (#b4befe - Lavender):** Used for navigation tabs, subheadings, and secondary actions.
- **Tertiary (#cba6f7 - Mauve):** Accent color for modal title headers, badge indicators, and indoor sensor callouts.
- **Surface (#181825 - Mantle):** Elevated panel and card background that gently distinguishes metrics from the base canvas.
- **Background (#1e1e2e - Base):** The root canvas color, providing deep warmth without pure black clipping.
- **Error (#f38ba8 - Red):** Reserved for sensor connection drops, severe weather alerts, and error dialogues.
- **On-Surface (#cdd6f4 - Text):** Crisp light text delivering high WCAG contrast against all dark surfaces.
- **On-Surface-Variant (#a6adc8 - Subtext0):** Soft contrast for unit symbols, subtitles, and timestamp metadata.
- **Outline (#6c7086 - Overlay0):** Subtle borders, chart gridlines, and inactive switch tracks.

## Typography

The interface employs **Montserrat** rendered via LVGL's pre-compiled bitmap font engine (`LV_FONT_MONTSERRAT_*`). Montserrat's wide geometric proportions and clear apertures maintain exceptional legibility on low-DPI SPI LCD screens.

- **Display-XL (48px, Bold):** Reserved exclusively for the large current temperature readout on the primary weather panel.
- **Headline-LG (28px, Semi-Bold):** Used for digital clock time, daily high/low summaries, and prominent indoor sensor readings.
- **Title-MD (20px, Semi-Bold):** Section headers, tab view titles, and modal dialogue headers.
- **Body-MD (14px, Regular):** Standard body text, weather condition descriptions, settings form labels, and button labels.
- **Label-SM (10px, Medium):** Chart axis labels (4-hour intervals), sensor timestamps, and small status indicators.

## Layout & Spacing

Layouts are designed for fixed-resolution displays with touch input:
- **CYD 2.8" Displays:** 320 x 240 pixels (Landscape) or 240 x 320 pixels (Portrait).
- **CYD 3.5" Displays:** 480 x 320 pixels (Landscape) or 320 x 480 pixels (Portrait).

### Grid and Rhythm
- **Fixed Header (24-28px):** Anchors current time, status icons (Wi-Fi, MQTT, SD card, battery), and optional title/version badge.
- **Swipeable Tab View:** Divides content into three primary screens (Current Conditions, 24-Hour Forecast Chart, and Settings).
- **Spacing Scale:** Built on a 4px modular unit (`xs: 2px`, `sm: 4px`, `md: 8px`, `lg: 12px`, `xl: 16px`) to maximize screen real estate while maintaining touch target ergonomics (minimum 32x32px touch targets).

## Elevation & Depth

Because embedded TFT displays lack high dynamic range, depth is achieved through **Tonal Layering** rather than heavy drop shadows:

- **Level 0 (Base / Background - #1e1e2e):** The root canvas under the tab view.
- **Level 1 (Surface / Cards - #181825):** Weather cards and indoor sensor panels, delineated by subtle 1px `#45475a` borders.
- **Level 2 (Containers & Active Controls - #313244):** Active button states, chart plot backgrounds, and form inputs.
- **Level 3 (Modal Overlays - #181825 on dimmed backdrop):** System alerts, Wi-Fi AP provisioning dialogues, and full-screen keyboard overlays with `#6c7086` outlines.

## Shapes

Shapes balance friendly modernity with efficient pixel rendering on embedded microcontrollers:

- **Cards & Containers:** `8px` (`rounded.md`) corner radius provides soft, contemporary panels without excessive anti-aliasing cost.
- **Modals & Dialogs:** `12px` (`rounded.lg`) corner radius for prominent overlay frames.
- **Buttons & Inputs:** `4px` (`rounded.sm`) corner radius maintains crisp button tap boundaries.
- **Switches & Badges:** `full` (`9999px`) pill shapes for toggle switch thumbs, track enclosures, and status dots.

## Components

The design system standardizes the following recurring components:

- **Weather Card (`weather-card`):** A surface container grouping temperature, humidity, pressure, and wind with accompanying weather icons.
- **Forecast Chart (`forecast-chart`):** An `lv_chart` component displaying 24-hour temperature curves (Blue `#89b4fa`) and precipitation probability (Sky `#89dceb`) over an outline grid (`#6c7086`).
- **Header Bar (`header-bar`):** A fixed top bar providing global device state and glanceable time.
- **Setting Controls (`setting-switch`, `setting-slider`):** Touch-friendly controls for configuring display sleep, brightness, Catppuccin theme flavors, and metric/imperial units.
- **Modal Dialog (`dialog-modal`):** Centered popups for Wi-Fi provisioning prompts and OTA update progress.
- **Status Badge (`badge-status`):** Compact pill tags for connection states and sensor health.
