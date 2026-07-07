# Product Guidelines: ESP32 CYD LVGL Weather Station

## 1. User Interface (UI) & User Experience (UX) Design
* **Resolution Constraint:** All screens must fit a fixed 320x240 pixel portrait/landscape screen (ESP32-2432S028R default is landscape 320x240).
* **Typography:**
  - Font sizes must be large enough to read from a distance of 1–2 meters (desk display).
  - Use clear, sans-serif fonts compatible with LVGL (e.g., standard Montserrat fonts built into LVGL).
* **Touch Targets:** Interactive elements (buttons, sliders, tabs) must have a minimum touch area of 40x40 pixels to facilitate easy control via resistive touchscreen.
* **Layout and Navigation:**
  - Keep the screen layout clean and un-cluttered.
  - Use simple tab views or swipe gestures to navigate between the "Current Weather" view and the "Forecast" view.
  - Provide immediate visual feedback for touch actions (e.g., button pressed state).

## 2. Technical and Performance Guidelines
* **Resource Optimization:**
  - Minimize unnecessary screen refreshes. Redraw only the UI components whose values have changed.
  - Compile custom icons or images as static C arrays to optimize flash/RAM memory usage.
* **Network & API Polling:**
  - Poll the Open-Meteo API at reasonable intervals (e.g., every 15–30 minutes) to avoid rate limits and save network bandwidth.
  - Perform HTTP requests asynchronously (if supported) or handle them in a way that doesn't block the main LVGL GUI loop or cause noticeable UI freezing.

## 3. Reliability & Error Recovery
* **Network Status Indication:** Always show a clear Wi-Fi connection indicator (e.g., signal bars or status icon) on the screen.
* **Graceful Degradation:**
  - If Wi-Fi is disconnected, show a visual warning (e.g., "Disconnected" status or icon). Do not crash the application.
  - If an API request fails, show the last successfully retrieved weather data along with a "Stale Data" indicator or timestamp.
* **Sensor Integration (Future-proofing):** Design the data model to allow local sensors (e.g., DHT22/BME280) to easily supplement external weather data.
