#ifndef LANDING_HTML_H
#define LANDING_HTML_H

#ifndef PROGMEM
#define PROGMEM
#endif

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
.btn-reboot-ap { background: #cba6f7; width: 100%; border: none; font-family: inherit; }
.btn-reboot-ap:hover { background: #b4befe; }
.btn-reset { background: #f38ba8; width: 100%; border: none; font-family: inherit; }
.btn-reset:hover { background: #eba0ac; }
</style>
</head>
<body>
<div class='card'>
<h1>CYD Weather Station</h1>
<p class='subtitle'>Version %APP_VERSION%</p>

<a href="/settings" class="btn btn-settings">⚙️ Device Settings</a>
<a href="/update" class="btn btn-update">🔄 Firmware Update</a>
<a href="/screenshot" class="btn btn-screenshot %SCREENSHOT_DISABLED%" target="_blank">📸 View Screenshot</a>
%SCREENSHOT_HELP_TEXT%
<a href="/clear_logs" class="btn btn-update" style="background: #fab387; color: #11111b;" onclick="return confirm('Are you sure you want to clear the SD card logs?');">🗑️ Clear SD Logs</a>
<a href="/clear_cache" class="btn btn-update" style="background: #fab387; color: #11111b;" onclick="return confirm('Are you sure you want to clear the SD card cache?');">🗑️ Clear SD Cache</a>
<form method="POST" action="/reboot_ap" style="margin-top: 15px; margin-bottom: 0;" onsubmit="return submitAction(event, this, 'Reboot device into Access Point (AP) setup mode? Stored settings will NOT be erased.');">
    <button type="submit" class="btn btn-reboot-ap">📶 Reboot to AP Setup Mode</button>
</form>
<form method="POST" action="/reset" style="margin-top: 15px; margin-bottom: 0;" onsubmit="return submitAction(event, this, 'Are you sure you want to factory reset this device? All settings and Wi-Fi credentials will be erased and the device will restart in AP Setup mode. This cannot be undone.');">
    <button type="submit" class="btn btn-reset">⚠️ Factory Reset</button>
</form>

<p style="margin-top: 25px; font-size: 13px; color: #6c7086;">Built for %DEVICE_NAME% | <a href="https://github.com/nicholaswilde/cyd-weather-station" target="_blank" style="color: #89b4fa; text-decoration: none;">GitHub</a></p>
</div>
<script>
function submitAction(e, form, msg) {
    if (!confirm(msg)) {
        if (e) e.preventDefault();
        return false;
    }
    if (!window.fetch) return true;
    if (e) e.preventDefault();
    var btn = form.querySelector('button');
    if (btn) btn.disabled = true;
    fetch(form.action, { method: 'POST' })
        .then(function(r) { return r.text(); })
        .then(function(html) {
            if (window.history && window.history.replaceState) {
                window.history.replaceState(null, '', '/');
            }
            document.open();
            document.write(html);
            document.close();
        })
        .catch(function() {
            var card = document.querySelector('.card');
            if (card) {
                card.innerHTML = '<h2>Device Restarting</h2><p>The device is restarting in Access Point (AP) setup mode. Connect to the <strong>cyd-weather-station-XXXX</strong> Wi-Fi network and navigate to <strong>http://192.168.4.1</strong>.</p>';
            }
        });
    return false;
}
</script>
</body>
</html>
)=====";

#endif
