#ifndef LANDING_HTML_H
#define LANDING_HTML_H

const char landing_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CYD Weather Station</title>
<style>
body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }
.card { background: #181825; border-radius: 12px; padding: 40px 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }
h1 { color: #cba6f7; margin-top: 0; margin-bottom: 5px; font-weight: 700; }
p.subtitle { color: #a6adc8; margin-top: 0; margin-bottom: 30px; font-size: 15px; }
.btn { display: flex; align-items: center; justify-content: center; text-decoration: none; width: 100%; padding: 14px; border-radius: 8px; color: #11111b; font-size: 16px; font-weight: bold; cursor: pointer; transition: transform 0.2s, background 0.2s; margin-bottom: 15px; box-sizing: border-box; }
.btn:active { transform: scale(0.98); }
.btn-settings { background: #89b4fa; }
.btn-settings:hover { background: #b4befe; }
.btn-update { background: #a6e3a1; }
.btn-update:hover { background: #94e2d5; }
.btn-screenshot { background: #f5c2e7; }
.btn-screenshot:hover { background: #f5e0dc; }
.btn-screenshot.disabled { background: #45475a; color: #a6adc8; cursor: not-allowed; pointer-events: none; }
</style>
</head>
<body>
<div class='card'>
<h1>CYD Weather Station</h1>
<p class='subtitle'>Version %APP_VERSION%</p>

<a href="/settings" class="btn btn-settings">⚙️ Device Settings</a>
<a href="/update" class="btn btn-update">🔄 Firmware Update</a>
<a href="/screenshot" class="btn btn-screenshot %SCREENSHOT_DISABLED%" target="_blank">📸 View Screenshot</a>

<p style="margin-top: 25px; font-size: 13px; color: #6c7086;">Built for ESP32-2432S028R</p>
</div>
</body>
</html>
)=====";

#endif
