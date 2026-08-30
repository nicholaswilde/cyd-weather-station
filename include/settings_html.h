#ifndef SETTINGS_HTML_H
#define SETTINGS_HTML_H

const char settings_html[] PROGMEM = R"=====(
<!DOCTYPE html>
<html>
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>CYD Weather Station - Settings</title>
<style>
body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: flex-start; min-height: 100vh; box-sizing: border-box; }
.card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 500px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; }
h2 { color: #f5c2e7; margin-top: 0; margin-bottom: 20px; font-weight: 600; text-align: center; }
label { display: block; margin-bottom: 8px; color: #a6adc8; font-size: 14px; }
select, input[type='text'], input[type='password'], input[type='number'] { width: 100%; padding: 12px; margin-bottom: 20px; border-radius: 6px; border: 1px solid #45475a; background: #313244; color: #cdd6f4; font-size: 16px; box-sizing: border-box; }
select:focus, input:focus { outline: none; border-color: #f5c2e7; }
input[type='checkbox'] { width: auto; margin-right: 10px; margin-bottom: 0; transform: scale(1.2); }
.slider-group { display: flex; align-items: center; gap: 15px; margin-bottom: 20px; }
.slider-group input[type='range'] { flex-grow: 1; margin: 0; cursor: pointer; }
.slider-group input[type='number'] { width: 80px; margin-bottom: 0; flex-shrink: 0; }
.checkbox-group { display: flex; align-items: center; margin-bottom: 10px; }
.checkbox-group label { margin-bottom: 0; margin-top: 2px; }
button { width: 100%; padding: 12px; background: #cba6f7; border: none; border-radius: 6px; color: #11111b; font-size: 16px; font-weight: bold; cursor: pointer; transition: background 0.2s; margin-top: 10px; }
button:hover { background: #f5c2e7; }
.section-title { color: #89b4fa; font-size: 18px; margin-top: 20px; margin-bottom: 15px; border-bottom: 1px solid #313244; padding-bottom: 5px; }
.btn-back { display: block; margin-top: 15px; color: #a6adc8; text-decoration: none; font-size: 14px; text-align: center; transition: color 0.2s; }
.btn-back:hover { color: #cdd6f4; }
.password-wrapper { position: relative; display: block; margin-bottom: 20px; }
.password-wrapper input[type='password'], .password-wrapper input[type='text'] { padding-right: 40px; margin-bottom: 0; }
.password-wrapper .toggle-password { position: absolute; right: 12px; top: 12px; cursor: pointer; color: #a6adc8; user-select: none; display: flex; align-items: center; justify-content: center; height: 20px; width: 20px; transition: color 0.2s; }
.password-wrapper .toggle-password:hover { color: #cdd6f4; }
</style>
</head>
<body>
<div class='card'>
<h2 style="margin-bottom: 5px;">CYD Weather Station</h2>
<p style="text-align: center; color: #a6adc8; margin-top: 0; margin-bottom: 20px; font-size: 14px;">Version %APP_VERSION%</p>
<form method='POST' action='/settings/save'>

<div class='section-title' style='margin-top: 0;'>Display & UI</div>
<label for='unit_system' title='Select the measurement units for weather data'>Unit System</label>
<select id='unit_system' name='unit_system'>
    <option value='1' %UNIT_METRIC%>Metric (Celsius, m/s)</option>
    <option value='2' %UNIT_IMPERIAL%>Imperial (Fahrenheit, mph)</option>
</select>

<label for='theme_flavor' title='Select the color palette for the user interface'>Theme (Catppuccin)</label>
<select id='theme_flavor' name='theme_flavor'>
    <option value='1' %THEME_MOCHA%>Mocha</option>
    <option value='2' %THEME_MACCHIATO%>Macchiato</option>
    <option value='3' %THEME_FRAPPE%>Frappe</option>
    <option value='4' %THEME_LATTE%>Latte</option>
</select>

<label for='screen_orientation' title='Set the rotation of the screen'>Screen Orientation</label>
<select id='screen_orientation' name='screen_orientation'>
    <option value='0' %ORIENT_0%>Portrait (0°)</option>
    <option value='1' %ORIENT_1%>Landscape (90°)</option>
    <option value='2' %ORIENT_2%>Portrait Rev (180°)</option>
    <option value='3' %ORIENT_3%>Landscape Rev (270°)</option>
</select>

<label for='brightness' title='Adjust the display backlight brightness'>Screen Brightness (10-100%)</label>
<div class='slider-group'>
    <input type='range' id='brightness_slider' min='10' max='100' value='%BRIGHTNESS%' oninput='document.getElementById("brightness").value = this.value'>
    <input type='number' id='brightness' name='brightness' min='10' max='100' value='%BRIGHTNESS%' oninput='document.getElementById("brightness_slider").value = this.value'>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='auto_brightness' name='auto_brightness' value='1' %AUTO_BRIGHTNESS%>
    <label for='auto_brightness' title='Automatically adjust brightness based on time of day'>Auto Brightness</label>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='screensaver_enabled' name='screensaver_enabled' value='1' %SCREENSAVER_ENABLED% onchange='toggleScreensaverSettings()'>
    <label for='screensaver_enabled' title='Enable a screensaver after a period of inactivity'>Enable Screensaver</label>
</div>

<div id='screensaver_settings' style='display: none; margin-left: 20px; border-left: 2px solid #313244; padding-left: 15px; margin-bottom: 20px;'>
    <label for='screensaver_timeout' title='Time in minutes before the screensaver activates'>Screensaver Timeout (Minutes)</label>
    <input type='number' id='screensaver_timeout' name='screensaver_timeout' min='1' max='60' value='%SCREENSAVER_TIMEOUT%'>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='sleep_schedule_enabled' name='sleep_schedule_enabled' value='1' %SLEEP_SCHEDULE_ENABLED% onchange='toggleSleepSettings()'>
    <label for='sleep_schedule_enabled' title='Enable a schedule to automatically sleep the screen'>Enable Sleep Schedule</label>
</div>

<div id='sleep_settings' style='display: none; margin-left: 20px; border-left: 2px solid #313244; padding-left: 15px; margin-bottom: 20px;'>
    <label for='sleep_start_time' title='Time the screen will go to sleep'>Sleep Start Time</label>
    <input type='time' id='sleep_start_time' name='sleep_start_time' value='%SLEEP_START_TIME%'>
    <label for='sleep_end_time' title='Time the screen will wake up'>Sleep End Time</label>
    <input type='time' id='sleep_end_time' name='sleep_end_time' value='%SLEEP_END_TIME%'>
</div>

<div class='section-title'>Location & Weather</div>
<label for='owm_api' title='Your OpenWeatherMap API key'>OpenWeatherMap API Key (<a href='https://home.openweathermap.org/api_keys' target='_blank' style='color: #89b4fa; text-decoration: none;'>Get Key</a>)</label>
<div class='password-wrapper'>
    <input type='password' id='owm_api' name='owm_api' value='%OWM_API%'>
    <span class='toggle-password' onclick='togglePwd("owm_api", this)'><svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg></span>
</div>
<label for='zip' title='Your US zip code for weather data'>Zip Code (US Only)</label>
<input type='text' id='zip' name='zip' value='%ZIP%'>

<label for='city' title='OpenWeatherMap City ID for accurate location'>City ID (<a href='https://openweathermap.org/find' target='_blank' style='color: #89b4fa; text-decoration: none;'>OpenWeatherMap</a>)</label>
<input type='text' id='city' name='city' value='%CITY%'>

<label for='lat' title='Latitude for weather location'>Latitude</label>
<input type='text' id='lat' name='lat' value='%LAT%'>

<label for='lon' title='Longitude for weather location'>Longitude</label>
<input type='text' id='lon' name='lon' value='%LON%'>

<label for='tz' title='IANA timezone string for accurate local time'>Timezone</label>
<select id='tz' name='tz'></select>

<label for='ntp_server' title='Network Time Protocol server for time synchronization'>NTP Server</label>
<input type='text' id='ntp_server' name='ntp_server' value='%NTP_SERVER%'>

<div class='checkbox-group'>
    <input type='checkbox' id='use_24h' name='use_24h' value='1' %USE_24H_CHECKED%>
    <label for='use_24h' title='Display time in 24-hour military format'>Use 24-Hour Format</label>
</div>


<label for='update_interval' title='How often to fetch new weather data'>Weather Update Interval (Minutes)</label>
<input type='number' id='update_interval' name='update_interval' min='1' max='60' value='%UPDATE_INTERVAL%'>

<hr>
<h3>Local Sensor Settings</h3>
<div class='checkbox-group'>
    <input type='checkbox' id='local_sensor_enabled' name='local_sensor_enabled' value='1' %LOCAL_SENSOR_ENABLED_CHECKED% onchange='toggleLocalSensorSettings()'>
    <label for='local_sensor_enabled' title='Enable the local temperature and humidity sensor'>Local Sensor Enabled</label>
</div>

<div id='local_sensor_settings' style='display: none; margin-left: 20px; border-left: 2px solid #313244; padding-left: 15px; margin-bottom: 20px;'>
    <label for='local_sensor_type' title='Type of local sensor'>Local Sensor Type</label>
    <select id='local_sensor_type' name='local_sensor_type'>
    <option value='1' %LOCAL_SENSOR_TYPE_1_SELECTED%>DHT22</option>
    <option value='2' %LOCAL_SENSOR_TYPE_2_SELECTED%>SHT40</option>
    <option value='3' %LOCAL_SENSOR_TYPE_3_SELECTED%>DHT11</option>
    </select>

    <label for='local_sensor_update_interval' title='How often to read the local sensor'>Local Sensor Update Interval (Seconds)</label>
    <input type='number' id='local_sensor_update_interval' name='local_sensor_update_interval' min='1' max='120' value='%LOCAL_SENSOR_UPDATE_INTERVAL%'>

    <label for='local_sensor_temp_offset' title='Offset to apply to the local temperature sensor'>Temperature Offset</label>
    <input type='number' id='local_sensor_temp_offset' name='local_sensor_temp_offset' step='0.1' value='%LOCAL_SENSOR_TEMP_OFFSET%'>

    <label for='local_sensor_hum_offset' title='Offset to apply to the local humidity sensor'>Humidity Offset (%)</label>
    <input type='number' id='local_sensor_hum_offset' name='local_sensor_hum_offset' step='0.1' value='%LOCAL_SENSOR_HUM_OFFSET%'>
</div>

<div class='section-title'>System Features</div>
<div class='checkbox-group'>
    <input type='checkbox' id='led_enabled' name='led_enabled' value='1' %LED_ENABLED%>
    <label for='led_enabled' title='Enable or disable the onboard RGB LED'>RGB LED Enabled</label>
</div>

<label for='led_brightness' title='Adjust the brightness of the RGB LED'>LED Brightness (0-100%)</label>
<div class='slider-group'>
    <input type='range' id='led_brightness_slider' min='0' max='100' value='%LED_BRIGHTNESS%' oninput='document.getElementById("led_brightness").value = this.value'>
    <input type='number' id='led_brightness' name='led_brightness' min='0' max='100' value='%LED_BRIGHTNESS%' oninput='document.getElementById("led_brightness_slider").value = this.value'>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='mqtt_enabled' name='mqtt_enabled' value='1' %MQTT_ENABLED% onchange='toggleMqttSettings()'>
    <label for='mqtt_enabled' title='Enable MQTT to publish device state and receive commands'>MQTT Enabled</label>
</div>

<div id='mqtt_settings' style='display: none; margin-left: 20px; border-left: 2px solid #313244; padding-left: 15px; margin-bottom: 20px;'>
    <label for='mqtt_server' title='Hostname or IP address of the MQTT broker'>MQTT Server</label>
    <input type='text' id='mqtt_server' name='mqtt_server' value='%MQTT_SERVER%'>
    
    <label for='mqtt_port' title='Port number for the MQTT broker (default: 1883)'>MQTT Port</label>
    <input type='number' id='mqtt_port' name='mqtt_port' value='%MQTT_PORT%'>
    
    <label for='mqtt_user' title='Username for MQTT authentication (leave blank if none)'>MQTT Username</label>
    <input type='text' id='mqtt_user' name='mqtt_user' value='%MQTT_USER%'>
    
    <label for='mqtt_password' title='Password for MQTT authentication (leave blank if none)'>MQTT Password</label>
    <div class='password-wrapper'>
        <input type='password' id='mqtt_password' name='mqtt_password' value='%MQTT_PASSWORD%'>
        <span class='toggle-password' onclick='togglePwd("mqtt_password", this)'><svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg></span>
    </div>

    <label for='mqtt_base' title='Base topic for MQTT messages (e.g., home/weather/)'>MQTT Base Topic</label>
    <input type='text' id='mqtt_base' name='mqtt_base' value='%MQTT_BASE%'>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='screenshot_server_enabled' name='screenshot_server_enabled' value='1' %SCREENSHOT_ENABLED%>
    <label for='screenshot_server_enabled' title='Enable the screenshot server for remote viewing'>Screenshot Server</label>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='api_server_enabled' name='api_server_enabled' value='1' %API_SERVER_ENABLED%>
    <label for='api_server_enabled' title='Enable the REST API server'>API Server</label>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='static_ip_enabled' name='static_ip_enabled' value='1' %STATIC_IP_ENABLED% onchange='toggleStaticIpSettings()'>
    <label for='static_ip_enabled' title='Use a static IP address instead of DHCP'>Static IP Enabled</label>
</div>

<div id='static_ip_settings' style='display: none; margin-left: 20px; border-left: 2px solid #313244; padding-left: 15px; margin-bottom: 20px;'>
    <label for='static_ip' title='Static IP address (e.g. 192.168.1.100)'>IP Address</label>
    <input type='text' id='static_ip' name='static_ip' value='%STATIC_IP%'>
    <label for='static_gw' title='Gateway IP address (e.g. 192.168.1.1)'>Gateway</label>
    <input type='text' id='static_gw' name='static_gw' value='%STATIC_GW%'>
    <label for='static_sn' title='Subnet mask (e.g. 255.255.255.0)'>Subnet Mask</label>
    <input type='text' id='static_sn' name='static_sn' value='%STATIC_SN%'>
    <label for='static_dns' title='DNS server IP address (e.g. 8.8.8.8)'>DNS Server</label>
    <input type='text' id='static_dns' name='static_dns' value='%STATIC_DNS%'>
</div>

<label for='ap_password' title='Password for the device&#39;s configuration Access Point'>Configuration AP Password (min 8 chars)</label>
<div class='password-wrapper'>
    <input type='password' id='ap_password' name='ap_password' value='%AP_PASSWORD%'>
    <span class='toggle-password' onclick='togglePwd("ap_password", this)'><svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg></span>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='sd_logging_enabled' name='sd_logging_enabled' value='1' %SD_LOGGING%>
    <label for='sd_logging_enabled' title='Enable logging to the SD card'>SD Card Logging</label>
</div>

<div class='checkbox-group'>
    <input type='checkbox' id='sd_cache_enabled' name='sd_cache_enabled' value='1' %SD_CACHE%>
    <label for='sd_cache_enabled' title='Enable caching resources on the SD card'>SD Card Cache</label>
</div>

<button type='submit'>Save Settings & Reboot</button>
</form>
<a href="/" class="btn-back">&larr; Back to Dashboard</a>
<p style="margin-top: 25px; margin-bottom: 0; font-size: 13px; color: #6c7086; text-align: center;">Built for %DEVICE_NAME% | <a href="https://github.com/nicholaswilde/cyd-weather-station" target="_blank" style="color: #89b4fa; text-decoration: none;">GitHub</a></p>
</div>
<script>
function toggleLocalSensorSettings() {
    var cb = document.getElementById('local_sensor_enabled');
    var div = document.getElementById('local_sensor_settings');
    if (cb && div) {
        div.style.display = cb.checked ? 'block' : 'none';
    }
}
function toggleMqttSettings() {
    var cb = document.getElementById('mqtt_enabled');
    var div = document.getElementById('mqtt_settings');
    if (cb && div) {
        div.style.display = cb.checked ? 'block' : 'none';
    }
}
function toggleScreensaverSettings() {
    var cb = document.getElementById('screensaver_enabled');
    var div = document.getElementById('screensaver_settings');
    if (cb && div) {
        div.style.display = cb.checked ? 'block' : 'none';
    }
}
function toggleSleepSettings() {
    var cb = document.getElementById('sleep_schedule_enabled');
    var div = document.getElementById('sleep_settings');
    if (cb && div) {
        div.style.display = cb.checked ? 'block' : 'none';
    }
}
function toggleStaticIpSettings() {
    var cb = document.getElementById('static_ip_enabled');
    var div = document.getElementById('static_ip_settings');
    if (cb && div) {
        div.style.display = cb.checked ? 'block' : 'none';
    }
}
function togglePwd(id, el) {
    var eyeSvg = '<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z"></path><circle cx="12" cy="12" r="3"></circle></svg>';
    var eyeOffSvg = '<svg xmlns="http://www.w3.org/2000/svg" width="20" height="20" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round"><path d="M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24"></path><line x1="1" y1="1" x2="23" y2="23"></line></svg>';
    var input = document.getElementById(id);
    if (input.type === "password") {
        input.type = "text";
        el.innerHTML = eyeOffSvg;
    } else {
        input.type = "password";
        el.innerHTML = eyeSvg;
    }
}
window.onload = function() {
    const tzs = ["Africa/Abidjan", "Africa/Accra", "Africa/Addis_Ababa", "Africa/Algiers", "Africa/Asmara", "Africa/Bamako", "Africa/Bangui", "Africa/Banjul", "Africa/Bissau", "Africa/Blantyre", "Africa/Brazzaville", "Africa/Bujumbura", "Africa/Cairo", "Africa/Casablanca", "Africa/Ceuta", "Africa/Conakry", "Africa/Dakar", "Africa/Dar_es_Salaam", "Africa/Djibouti", "Africa/Douala", "Africa/El_Aaiun", "Africa/Freetown", "Africa/Gaborone", "Africa/Harare", "Africa/Johannesburg", "Africa/Juba", "Africa/Kampala", "Africa/Khartoum", "Africa/Kigali", "Africa/Kinshasa", "Africa/Lagos", "Africa/Libreville", "Africa/Lome", "Africa/Luanda", "Africa/Lubumbashi", "Africa/Lusaka", "Africa/Malabo", "Africa/Maputo", "Africa/Maseru", "Africa/Mbabane", "Africa/Mogadishu", "Africa/Monrovia", "Africa/Nairobi", "Africa/Ndjamena", "Africa/Niamey", "Africa/Nouakchott", "Africa/Ouagadougou", "Africa/Porto-Novo", "Africa/Sao_Tome", "Africa/Timbuktu", "Africa/Tripoli", "Africa/Tunis", "Africa/Windhoek", "America/Adak", "America/Anchorage", "America/Anguilla", "America/Antigua", "America/Araguaina", "America/Argentina/Buenos_Aires", "America/Argentina/Catamarca", "America/Argentina/Cordoba", "America/Argentina/Jujuy", "America/Argentina/La_Rioja", "America/Argentina/Mendoza", "America/Argentina/Rio_Gallegos", "America/Argentina/Salta", "America/Argentina/San_Juan", "America/Argentina/San_Luis", "America/Argentina/Tucuman", "America/Argentina/Ushuaia", "America/Aruba", "America/Asuncion", "America/Atikokan", "America/Atka", "America/Bahia", "America/Bahia_Banderas", "America/Barbados", "America/Belem", "America/Belize", "America/Blanc-Sablon", "America/Boa_Vista", "America/Bogota", "America/Boise", "America/Cambridge_Bay", "America/Campo_Grande", "America/Cancun", "America/Caracas", "America/Cayenne", "America/Cayman", "America/Chicago", "America/Chihuahua", "America/Ciudad_Juarez", "America/Coral_Harbour", "America/Costa_Rica", "America/Coyhaique", "America/Creston", "America/Cuiaba", "America/Curacao", "America/Danmarkshavn", "America/Dawson", "America/Dawson_Creek", "America/Denver", "America/Detroit", "America/Dominica", "America/Edmonton", "America/Eirunepe", "America/El_Salvador", "America/Ensenada", "America/Fort_Nelson", "America/Fortaleza", "America/Glace_Bay", "America/Goose_Bay", "America/Grand_Turk", "America/Grenada", "America/Guadeloupe", "America/Guatemala", "America/Guayaquil", "America/Guyana", "America/Halifax", "America/Havana", "America/Hermosillo", "America/Indiana/Indianapolis", "America/Indiana/Knox", "America/Indiana/Marengo", "America/Indiana/Petersburg", "America/Indiana/Tell_City", "America/Indiana/Vevay", "America/Indiana/Vincennes", "America/Indiana/Winamac", "America/Inuvik", "America/Iqaluit", "America/Jamaica", "America/Juneau", "America/Kentucky/Louisville", "America/Kentucky/Monticello", "America/Kralendijk", "America/La_Paz", "America/Lima", "America/Los_Angeles", "America/Lower_Princes", "America/Maceio", "America/Managua", "America/Manaus", "America/Marigot", "America/Martinique", "America/Matamoros", "America/Mazatlan", "America/Menominee", "America/Merida", "America/Metlakatla", "America/Mexico_City", "America/Miquelon", "America/Moncton", "America/Monterrey", "America/Montevideo", "America/Montreal", "America/Montserrat", "America/Nassau", "America/New_York", "America/Nipigon", "America/Nome", "America/Noronha", "America/North_Dakota/Beulah", "America/North_Dakota/Center", "America/North_Dakota/New_Salem", "America/Nuuk", "America/Ojinaga", "America/Panama", "America/Pangnirtung", "America/Paramaribo", "America/Phoenix", "America/Port-au-Prince", "America/Port_of_Spain", "America/Porto_Acre", "America/Porto_Velho", "America/Puerto_Rico", "America/Punta_Arenas", "America/Rainy_River", "America/Rankin_Inlet", "America/Recife", "America/Regina", "America/Resolute", "America/Rio_Branco", "America/Santa_Isabel", "America/Santarem", "America/Santiago", "America/Santo_Domingo", "America/Sao_Paulo", "America/Scoresbysund", "America/Shiprock", "America/Sitka", "America/St_Barthelemy", "America/St_Johns", "America/St_Kitts", "America/St_Lucia", "America/St_Thomas", "America/St_Vincent", "America/Swift_Current", "America/Tegucigalpa", "America/Thule", "America/Thunder_Bay", "America/Tijuana", "America/Toronto", "America/Tortola", "America/Vancouver", "America/Virgin", "America/Whitehorse", "America/Winnipeg", "America/Yakutat", "America/Yellowknife", "Antarctica/Casey", "Antarctica/Davis", "Antarctica/DumontDUrville", "Antarctica/Macquarie", "Antarctica/Mawson", "Antarctica/McMurdo", "Antarctica/Palmer", "Antarctica/Rothera", "Antarctica/Syowa", "Antarctica/Troll", "Antarctica/Vostok", "Arctic/Longyearbyen", "Asia/Aden", "Asia/Almaty", "Asia/Amman", "Asia/Anadyr", "Asia/Aqtau", "Asia/Aqtobe", "Asia/Ashgabat", "Asia/Atyrau", "Asia/Baghdad", "Asia/Bahrain", "Asia/Baku", "Asia/Bangkok", "Asia/Barnaul", "Asia/Beirut", "Asia/Bishkek", "Asia/Brunei", "Asia/Chita", "Asia/Chongqing", "Asia/Colombo", "Asia/Damascus", "Asia/Dhaka", "Asia/Dili", "Asia/Dubai", "Asia/Dushanbe", "Asia/Famagusta", "Asia/Gaza", "Asia/Harbin", "Asia/Hebron", "Asia/Ho_Chi_Minh", "Asia/Hong_Kong", "Asia/Hovd", "Asia/Irkutsk", "Asia/Istanbul", "Asia/Jakarta", "Asia/Jayapura", "Asia/Jerusalem", "Asia/Kabul", "Asia/Kamchatka", "Asia/Karachi", "Asia/Kashgar", "Asia/Kathmandu", "Asia/Khandyga", "Asia/Kolkata", "Asia/Krasnoyarsk", "Asia/Kuala_Lumpur", "Asia/Kuching", "Asia/Kuwait", "Asia/Macau", "Asia/Magadan", "Asia/Makassar", "Asia/Manila", "Asia/Muscat", "Asia/Nicosia", "Asia/Novokuznetsk", "Asia/Novosibirsk", "Asia/Omsk", "Asia/Oral", "Asia/Phnom_Penh", "Asia/Pontianak", "Asia/Pyongyang", "Asia/Qatar", "Asia/Qostanay", "Asia/Qyzylorda", "Asia/Riyadh", "Asia/Sakhalin", "Asia/Samarkand", "Asia/Seoul", "Asia/Shanghai", "Asia/Singapore", "Asia/Srednekolymsk", "Asia/Taipei", "Asia/Tashkent", "Asia/Tbilisi", "Asia/Tehran", "Asia/Tel_Aviv", "Asia/Thimphu", "Asia/Tokyo", "Asia/Tomsk", "Asia/Ulaanbaatar", "Asia/Urumqi", "Asia/Ust-Nera", "Asia/Vientiane", "Asia/Vladivostok", "Asia/Yakutsk", "Asia/Yangon", "Asia/Yekaterinburg", "Asia/Yerevan", "Atlantic/Azores", "Atlantic/Bermuda", "Atlantic/Canary", "Atlantic/Cape_Verde", "Atlantic/Faroe", "Atlantic/Jan_Mayen", "Atlantic/Madeira", "Atlantic/Reykjavik", "Atlantic/South_Georgia", "Atlantic/St_Helena", "Atlantic/Stanley", "Australia/Adelaide", "Australia/Brisbane", "Australia/Broken_Hill", "Australia/Canberra", "Australia/Currie", "Australia/Darwin", "Australia/Eucla", "Australia/Hobart", "Australia/Lindeman", "Australia/Lord_Howe", "Australia/Melbourne", "Australia/Perth", "Australia/Sydney", "Australia/Yancowinna", "Europe/Amsterdam", "Europe/Andorra", "Europe/Astrakhan", "Europe/Athens", "Europe/Belfast", "Europe/Belgrade", "Europe/Berlin", "Europe/Bratislava", "Europe/Brussels", "Europe/Bucharest", "Europe/Budapest", "Europe/Busingen", "Europe/Chisinau", "Europe/Copenhagen", "Europe/Dublin", "Europe/Gibraltar", "Europe/Guernsey", "Europe/Helsinki", "Europe/Isle_of_Man", "Europe/Istanbul", "Europe/Jersey", "Europe/Kaliningrad", "Europe/Kirov", "Europe/Kyiv", "Europe/Lisbon", "Europe/Ljubljana", "Europe/London", "Europe/Luxembourg", "Europe/Madrid", "Europe/Malta", "Europe/Mariehamn", "Europe/Minsk", "Europe/Monaco", "Europe/Moscow", "Europe/Nicosia", "Europe/Oslo", "Europe/Paris", "Europe/Podgorica", "Europe/Prague", "Europe/Riga", "Europe/Rome", "Europe/Samara", "Europe/San_Marino", "Europe/Sarajevo", "Europe/Saratov", "Europe/Simferopol", "Europe/Skopje", "Europe/Sofia", "Europe/Stockholm", "Europe/Tallinn", "Europe/Tirane", "Europe/Tiraspol", "Europe/Ulyanovsk", "Europe/Vaduz", "Europe/Vatican", "Europe/Vienna", "Europe/Vilnius", "Europe/Volgograd", "Europe/Warsaw", "Europe/Zagreb", "Europe/Zurich", "Indian/Antananarivo", "Indian/Chagos", "Indian/Christmas", "Indian/Cocos", "Indian/Comoro", "Indian/Kerguelen", "Indian/Mahe", "Indian/Maldives", "Indian/Mauritius", "Indian/Mayotte", "Indian/Reunion", "Pacific/Apia", "Pacific/Auckland", "Pacific/Bougainville", "Pacific/Chatham", "Pacific/Chuuk", "Pacific/Easter", "Pacific/Efate", "Pacific/Fakaofo", "Pacific/Fiji", "Pacific/Funafuti", "Pacific/Galapagos", "Pacific/Gambier", "Pacific/Guadalcanal", "Pacific/Guam", "Pacific/Honolulu", "Pacific/Johnston", "Pacific/Kanton", "Pacific/Kiritimati", "Pacific/Kosrae", "Pacific/Kwajalein", "Pacific/Majuro", "Pacific/Marquesas", "Pacific/Midway", "Pacific/Nauru", "Pacific/Niue", "Pacific/Norfolk", "Pacific/Noumea", "Pacific/Pago_Pago", "Pacific/Palau", "Pacific/Pitcairn", "Pacific/Pohnpei", "Pacific/Port_Moresby", "Pacific/Rarotonga", "Pacific/Saipan", "Pacific/Samoa", "Pacific/Tahiti", "Pacific/Tarawa", "Pacific/Tongatapu", "Pacific/Wake", "Pacific/Wallis", "Pacific/Yap", "UTC"];
    const tzSelect = document.getElementById('tz');
    const currentTz = '%TZ%';
    tzs.forEach(function(tz) {
        var opt = document.createElement('option');
        opt.value = tz;
        opt.innerHTML = tz;
        if (tz === currentTz) opt.selected = true;
        tzSelect.appendChild(opt);
    });
    toggleLocalSensorSettings();
    toggleMqttSettings();
    toggleScreensaverSettings();
  toggleSleepSettings();
    toggleStaticIpSettings();
};
</script>
</body>
</html>
)=====";

#endif // SETTINGS_HTML_H
