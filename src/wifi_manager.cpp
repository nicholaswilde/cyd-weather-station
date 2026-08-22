#include "wifi_manager.h"
#include <Arduino.h>
#include "settings_manager.h"
#include "config/config.h"

#ifndef NATIVE_TEST
#include "screenshot_manager.h"
#include "sd_card_manager.h"
#include <lvgl.h>
#include <SD.h>
#include <Update.h>
#include "ui.h"
#include "ota_html.h"
#include "settings_html.h"
#include "landing_html.h"
#include "version.h"
#include <ArduinoJson.h>
#include "weather_cache.h"
#include "weather_logger.h"
#endif

extern SettingsManager settings;

#if defined(ST7796_DRIVER)
    #define DEVICE_NAME "ESP32-3248S035C"
#elif defined(ST7789_DRIVER)
    #if defined(HAS_CAPACITIVE_TOUCH)
        #define DEVICE_NAME "ESP32-2432W328C"
    #else
        #define DEVICE_NAME "Unknown ST7789 CYD"
    #endif
#elif defined(ILI9341_DRIVER)
    #define DEVICE_NAME "ESP32-2432S028R"
#else
    #define DEVICE_NAME "Unknown CYD Device"
#endif

static void configureStaticIP() {
#ifndef NATIVE_TEST
    if (!settings.getStaticIpEnabled()) return;
    String staticIpStr = settings.getStaticIp();
    if (staticIpStr.length() > 0) {
        IPAddress local_ip;
        if (local_ip.fromString(staticIpStr)) {
            IPAddress gateway_ip;
            IPAddress subnet_ip;
            IPAddress dns_ip;
            
            if (settings.getStaticGateway().length() > 0) gateway_ip.fromString(settings.getStaticGateway());
            if (settings.getStaticSubnet().length() > 0) subnet_ip.fromString(settings.getStaticSubnet());
            if (settings.getStaticDns().length() > 0) dns_ip.fromString(settings.getStaticDns());

            if (WiFi.config(local_ip, gateway_ip, subnet_ip, dns_ip)) {
                Serial.println("[WiFi] Static IP configured successfully.");
            } else {
                Serial.println("[WiFi] Failed to configure Static IP.");
            }
        }
    }
#endif
}

WifiManager::WifiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password), _state(WIFI_STATE_DISCONNECTED), _lastReconnectAttempt(0), _connectionStartTime(0) {}

void WifiManager::begin() {
    Serial.println("[WiFi] Starting Wi-Fi Manager...");
#ifndef NATIVE_TEST
    WiFi.setAutoReconnect(true);
    WiFi.setTxPower(WIFI_POWER_11dBm);

    _improv = new ImprovWiFi(&Serial);
    _improv->setDeviceInfo(ImprovTypes::ChipFamily::CF_ESP32, "CYD-Weather-Station", "1.0", "CYD Weather Station", "http://{LOCAL_IPV4}");
    
    _improv->setCustomConnectWiFi([](const char *ssid, const char *password) {
        Serial.printf("\n[WiFi] Improv connecting to %s...\n", ssid);
        // Turn off AP mode to speed up STA connection and avoid channel conflicts
        WiFi.softAPdisconnect(true);
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);
        
        WiFi.begin(ssid, password);
        int attempts = 0;
        // Wait up to 8 seconds (16 * 500ms) to prevent browser RPC timeout (usually 10s)
        while (WiFi.status() != WL_CONNECTED && attempts < 16) { 
            delay(500);
            attempts++;
        }
        return WiFi.status() == WL_CONNECTED;
    });

    _improv->onImprovConnected([](const char *ssid, const char *password) {
        Serial.printf("\n[WiFi] Improv provisioned successfully!\n");
        settings.setWifiSSID(String(ssid));
        settings.setWifiPassword(String(password));
        // No need to restart; WifiManager::update() handles the state transition to WIFI_STATE_CONNECTED
    });
#endif
    WiFi.mode(WIFI_STA);
    
    if (_ssid.length() == 0) {
        Serial.println("[WiFi] No credentials configured. Launching AP mode directly...");
        startAPMode();
    } else {
        configureStaticIP();
        WiFi.begin(_ssid.c_str(), _password.c_str());
        _state = WIFI_STATE_CONNECTING;
        _connectionStartTime = millis();
        Serial.printf("[WiFi] Connecting to %s...\n", _ssid.c_str());
    }
}

void WifiManager::update() {
#ifndef NATIVE_TEST
    if (_improv) {
        _improv->handleSerial();
    }
#endif
    wl_status_t status = WiFi.status();

    switch (_state) {
        case WIFI_STATE_DISCONNECTED:
            if (millis() - _lastReconnectAttempt > _reconnectInterval) {
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Reconnecting...");
                configureStaticIP();
                WiFi.begin(_ssid.c_str(), _password.c_str());
                _state = WIFI_STATE_CONNECTING;
                _connectionStartTime = millis();
            }
            break;

        case WIFI_STATE_CONNECTING:
            if (status == WL_CONNECTED) {
                _state = WIFI_STATE_CONNECTED;
                Serial.print("[WiFi] Connected! IP address: ");
                Serial.println(WiFi.localIP());
#ifndef NATIVE_TEST
                startWebServer();
#endif
            } else if (status == WL_CONNECT_FAILED || status == WL_NO_SSID_AVAIL || (millis() - _connectionStartTime > _connectionTimeout)) {
                Serial.println("[WiFi] Connection failed or timed out. Transitioning to AP Mode...");
                startAPMode();
            }
            break;

        case WIFI_STATE_CONNECTED:
            if (status != WL_CONNECTED) {
                _state = WIFI_STATE_DISCONNECTED;
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Connection lost.");
#ifndef NATIVE_TEST
                stopWebServer();
#endif
            } else {
#ifndef NATIVE_TEST
                if (_webServer) {
                    _webServer->handleClient();
                }
#endif
            }
            break;

        case WIFI_STATE_AP_MODE:
            if (WiFi.status() == WL_CONNECTED) {
                Serial.println("[WiFi] Wi-Fi connected in background. Stopping AP Mode...");
#ifndef NATIVE_TEST
                if (_dnsServer) {
                    _dnsServer->stop();
                    delete _dnsServer;
                    _dnsServer = nullptr;
                }
                if (_webServer) {
                    _webServer->stop();
                    delete _webServer;
                    _webServer = nullptr;
                }
                WiFi.softAPdisconnect(true);
                WiFi.mode(WIFI_STA);
                startWebServer();
#endif
                _state = WIFI_STATE_CONNECTED;
                Serial.print("[WiFi] Connected! IP address: ");
                Serial.println(WiFi.localIP());
            } else {
#ifndef NATIVE_TEST
                if (_dnsServer) _dnsServer->processNextRequest();
                if (_webServer) _webServer->handleClient();
#endif
            }
            break;
    }
}

WifiState WifiManager::getState() const {
    return _state;
}

String WifiManager::getIPAddress() const {
    if (_state == WIFI_STATE_CONNECTED) {
        return WiFi.localIP().toString();
    } else if (_state == WIFI_STATE_AP_MODE) {
        return "192.168.4.1";
    }
    return "0.0.0.0";
}

int8_t WifiManager::getRSSI() const {
    if (_state == WIFI_STATE_CONNECTED) {
        return WiFi.RSSI();
    }
    return -100;
}

void WifiManager::setCredentials(const String& ssid, const String& password) {
    _ssid = ssid;
    _password = password;
}

String WifiManager::getAPSSID() {
    String mac = WiFi.macAddress();
    String cleanMac = "";
    for (size_t i = 0; i < mac.length(); i++) {
        if (mac[i] != ':') {
            cleanMac += mac[i];
        }
    }
    String suffix = "";
    if (cleanMac.length() >= 4) {
        suffix = String(cleanMac.c_str() + cleanMac.length() - 4);
    } else {
        suffix = "ESP32";
    }
    for (size_t i = 0; i < suffix.length(); i++) {
        suffix[i] = toupper(suffix[i]);
    }
    return "cyd-weather-station-" + suffix;
}

void WifiManager::startAPMode() {
    _state = WIFI_STATE_AP_MODE;
    String apSSID = getAPSSID();
    Serial.printf("[WiFi] Entering AP Mode. SSID: %s\n", apSSID.c_str());

#ifndef NATIVE_TEST
    WiFi.persistent(false);
    WiFi.setAutoReconnect(false);
    WiFi.disconnect();
    delay(200);

    WiFi.mode(WIFI_AP_STA);
    WiFi.setTxPower(WIFI_POWER_11dBm);
    delay(100);

    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    delay(100);
    
    const char* apPass = nullptr;
    String apPasswordStr = settings.getApPassword();
    if (apPasswordStr.length() >= 8) {
        apPass = apPasswordStr.c_str();
    }
    
    WiFi.softAP(apSSID.c_str(), apPass);
    delay(200);

    // Wait for the captive portal to trigger the scan
    _cachedNetworksHTML = "<div class='net-item' style='color: #a6adc8;'>Scanning in progress... Please refresh.</div>";

    _dnsServer = new DNSServer();
    _dnsServer->setErrorReplyCode(DNSReplyCode::NoError);
    _dnsServer->start(53, "*", apIP);

    _webServer = new WebServer(80);
    _webServer->on("/", [this]() { handleRoot(); });
    _webServer->on("/save", [this]() { handleSave(); });
    _webServer->on("/scan", [this]() {
        WiFi.scanNetworks(true, false, false, 150);
        String html = "<!DOCTYPE html><html><head>";
        html += "<meta http-equiv='refresh' content='3;url=/'>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
        html += "<title>Scanning...</title>";
        html += "<style>";
        html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; }";
        html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }";
        html += "h2 { color: #f5c2e7; margin-top: 0; }";
        html += "p { color: #a6adc8; }";
        html += "</style></head><body>";
        html += "<div class='card'><h2>Scanning for Wi-Fi...</h2><p>Please wait while we refresh the network list.</p></div>";
        html += "</body></html>";
        _webServer->send(200, "text/html", html);
    });
    _webServer->onNotFound([this]() { handleNotFound(); });
    registerOTARoutes();
    _webServer->begin();

    Serial.println("[WiFi] AP Mode Web Server and DNS Server started.");
#endif
}

void WifiManager::handleRoot() {
#ifndef NATIVE_TEST
    int16_t scanStatus = WiFi.scanComplete();
    if (scanStatus >= 0) {
        _cachedNetworksHTML = "";
        for (int i = 0; i < scanStatus; ++i) {
            String ssidName = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            _cachedNetworksHTML += "<div class='net-item' onclick='selectSSID(\"" + ssidName + "\")'>";
            _cachedNetworksHTML += "<span>" + ssidName + "</span>";
            _cachedNetworksHTML += "<span style='color: #a6adc8; font-size: 12px;'>" + String(rssi) + " dBm</span>";
            _cachedNetworksHTML += "</div>";
        }
        WiFi.scanDelete();
    } else if (scanStatus == WIFI_SCAN_FAILED) {
        WiFi.scanNetworks(true, false, false, 150);
        if (_cachedNetworksHTML.length() == 0 || _cachedNetworksHTML.indexOf("Scanning in progress") != -1) {
            _cachedNetworksHTML = "<div class='net-item' style='color: #a6adc8;'>Scanning in progress... Please refresh.</div>";
        }
    }

    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    if (scanStatus == WIFI_SCAN_RUNNING || scanStatus == WIFI_SCAN_FAILED) {
        html += "<meta http-equiv='refresh' content='3'>";
    }
    html += "<title>CYD Weather Station Setup</title>";
    html += "<style>";
    html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
    html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; }";
    html += "h2 { color: #f5c2e7; margin-top: 0; margin-bottom: 20px; font-weight: 600; text-align: center; }";
    html += "label { display: block; margin-bottom: 8px; color: #a6adc8; font-size: 14px; }";
    html += "select, input[type='text'], input[type='password'] { width: 100%; padding: 12px; margin-bottom: 20px; border-radius: 6px; border: 1px solid #45475a; background: #313244; color: #cdd6f4; font-size: 16px; box-sizing: border-box; }";
    html += "select:focus, input:focus { outline: none; border-color: #f5c2e7; }";
    html += "button { width: 100%; padding: 12px; background: #cba6f7; border: none; border-radius: 6px; color: #11111b; font-size: 16px; font-weight: bold; cursor: pointer; transition: background 0.2s; }";
    html += "button:hover { background: #f5c2e7; }";
    html += ".section-title { color: #89b4fa; font-size: 18px; margin-top: 20px; margin-bottom: 15px; border-bottom: 1px solid #313244; padding-bottom: 5px; }";
    html += ".net-list { margin-bottom: 20px; max-height: 150px; overflow-y: auto; border: 1px solid #313244; border-radius: 6px; padding: 10px; background: #11111b; }";
    html += ".net-item { display: flex; justify-content: space-between; padding: 8px; cursor: pointer; border-bottom: 1px solid #1e1e2e; }";
    html += ".net-item:last-child { border-bottom: none; }";
    html += ".net-item:hover { background: #313244; color: #f5c2e7; }";
    html += ".password-wrapper { position: relative; display: block; margin-bottom: 20px; }";
    html += ".password-wrapper input[type='password'], .password-wrapper input[type='text'] { padding-right: 40px; margin-bottom: 0; }";
    html += ".password-wrapper .toggle-password { position: absolute; right: 12px; top: 12px; cursor: pointer; color: #a6adc8; user-select: none; display: flex; align-items: center; justify-content: center; height: 20px; width: 20px; transition: color 0.2s; }";
    html += ".password-wrapper .toggle-password:hover { color: #cdd6f4; }";
    html += "</style>";
    html += "<script>";
    html += "function selectSSID(ssid) { document.getElementById('ssid').value = ssid; }";
    html += "function togglePwd(id, el) {";
    html += "    var eyeSvg = '<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"></path><circle cx=\"12\" cy=\"12\" r=\"3\"></circle></svg>';";
    html += "    var eyeOffSvg = '<svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M17.94 17.94A10.07 10.07 0 0 1 12 20c-7 0-11-8-11-8a18.45 18.45 0 0 1 5.06-5.94M9.9 4.24A9.12 9.12 0 0 1 12 4c7 0 11 8 11 8a18.5 18.5 0 0 1-2.16 3.19m-6.72-1.07a3 3 0 1 1-4.24-4.24\"></path><line x1=\"1\" y1=\"1\" x2=\"23\" y2=\"23\"></line></svg>';";
    html += "    var input = document.getElementById(id);";
    html += "    if (input.type === \"password\") {";
    html += "        input.type = \"text\";";
    html += "        el.innerHTML = eyeOffSvg;";
    html += "    } else {";
    html += "        input.type = \"password\";";
    html += "        el.innerHTML = eyeSvg;";
    html += "    }";
    html += "}";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='card'>";
    html += "<h2 style=\"margin-bottom: 5px;\">CYD Weather Station</h2>";
#ifdef APP_VERSION
    html += "<p style=\"text-align: center; color: #a6adc8; margin-top: 0; margin-bottom: 20px; font-size: 14px;\">Version " + String(APP_VERSION) + "</p>";
#else
    html += "<p style=\"text-align: center; color: #a6adc8; margin-top: 0; margin-bottom: 20px; font-size: 14px;\">Version unknown</p>";
#endif
    html += "<form method='POST' action='/save'>";
    
    html += "<div class='section-title' style='margin-top: 0;'>Wi-Fi Connection</div>";
    
    html += "<div style='display: flex; justify-content: space-between; align-items: center;'>";
    html += "<label style='margin-bottom: 0;'>Select Network</label>";
    html += "<a href='/scan' style='color: #cba6f7; font-size: 12px; text-decoration: none;'>&#x21bb; Refresh List</a>";
    html += "</div>";
    html += "<div style='height: 8px;'></div>";
    
    html += "<div class='net-list'>";
    html += _cachedNetworksHTML;
    html += "</div>";
    
    html += "<label for='ssid'>SSID</label>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='SSID name' required>";
    
    html += "<label for='pass'>Password</label>";
    html += "<div class='password-wrapper'>";
    html += "<input type='password' id='pass' name='pass' placeholder='Password'>";
    html += "<span class='toggle-password' onclick='togglePwd(\"pass\", this)'><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"></path><circle cx=\"12\" cy=\"12\" r=\"3\"></circle></svg></span>";
    html += "</div>";
    
    html += "<div class='section-title'>Weather Service</div>";
    
    html += "<label for='owm_api'>OpenWeatherMap API Key</label>";
    html += "<div class='password-wrapper'>";
    html += "<input type='password' id='owm_api' name='owm_api' placeholder='API Key' value='" + settings.getOwmApiKey() + "'>";
    html += "<span class='toggle-password' onclick='togglePwd(\"owm_api\", this)'><svg xmlns=\"http://www.w3.org/2000/svg\" width=\"20\" height=\"20\" viewBox=\"0 0 24 24\" fill=\"none\" stroke=\"currentColor\" stroke-width=\"2\" stroke-linecap=\"round\" stroke-linejoin=\"round\"><path d=\"M1 12s4-8 11-8 11 8 11 8-4 8-11 8-11-8-11-8z\"></path><circle cx=\"12\" cy=\"12\" r=\"3\"></circle></svg></span>";
    html += "</div>";
    
    html += "<div class='section-title'>Location & Time</div>";
    
    html += "<label for='zip'>Zip Code (US Only)</label>";
    html += "<input type='text' id='zip' name='zip' placeholder='e.g. 90210' value='" + settings.getZipCode() + "'>";
    
    html += "<label for='city'>City ID (<a href='https://openweathermap.org/find' target='_blank' style='color: #89b4fa; text-decoration: none;'>OpenWeatherMap</a>)</label>";
    html += "<input type='text' id='city' name='city' placeholder='e.g. 2643743' value='" + settings.getCityCode() + "'>";
    
    html += "<label for='lat'>Latitude</label>";
    html += "<input type='text' id='lat' name='lat' placeholder='e.g. 34.1031' value='" + settings.getLatitude() + "'>";
    
    html += "<label for='lon'>Longitude</label>";
    html += "<input type='text' id='lon' name='lon' placeholder='e.g. -118.416' value='" + settings.getLongitude() + "'>";
    
    html += "<label for='tz'>Timezone (<a href='https://gist.github.com/alwynallan/24d96091655391107939' target='_blank' style='color: #89b4fa; text-decoration: none;'>POSIX format</a>)</label>";
    html += "<input type='text' id='tz' name='tz' placeholder='e.g. PST8PDT,M3.2.0,M11.1.0' value='" + settings.getTimezone() + "'>";
    
    html += "<label for='ntp_server'>NTP Server</label>";
    html += "<input type='text' id='ntp_server' name='ntp_server' placeholder='e.g. pool.ntp.org' value='" + settings.getNtpServer() + "'>";
    
    html += "<p style='color: #a6adc8; font-size: 12px; margin-top: -10px; margin-bottom: 20px; text-align: center;'><em>Leave location fields blank to auto-detect your location via IP address.</em></p>";
    
    html += "<button type='submit'>Save & Connect</button>";
    html += "</form>";
    html += "<p style=\"margin-top: 25px; margin-bottom: 0; font-size: 13px; color: #6c7086; text-align: center;\">Built for " + String(DEVICE_NAME) + " | <a href=\"https://github.com/nicholaswilde/cyd-weather-station\" target=\"_blank\" style=\"color: #89b4fa; text-decoration: none;\">GitHub</a></p>";
    html += "</div>";
    html += "</body></html>";

    _webServer->send(200, "text/html", html);
#endif
}

void WifiManager::handleSave() {
#ifndef NATIVE_TEST
    String ssid = _webServer->arg("ssid");
    String pass = _webServer->arg("pass");
    String zip = _webServer->arg("zip");
    String city = _webServer->arg("city");
    String lat = _webServer->arg("lat");
    String lon = _webServer->arg("lon");
    String tz = _webServer->arg("tz");
    String owmApi = _webServer->arg("owm_api");
    String ntpServer = _webServer->arg("ntp_server");

    Serial.printf("[WiFi] Saved new credentials via captive portal: %s\n", ssid.c_str());

    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<title>Credentials Saved</title>";
    html += "<style>";
    html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
    html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }";
    html += "h2 { color: #a6e3a1; margin-top: 0; margin-bottom: 20px; }";
    html += "p { color: #cdd6f4; margin-bottom: 20px; line-height: 1.5; }";
    html += "</style></head><body>";
    html += "<div class='card'>";
    html += "<h2>Configuration Saved</h2>";
    html += "<p>Connecting to <strong>" + ssid + "</strong>...</p>";
    html += "<p>The device will now reboot to apply the new settings. You can close this page.</p>";
    html += "</div>";
    html += "</body></html>";

    settings.setWifiSSID(ssid);
    settings.setWifiPassword(pass);
    settings.setZipCode(zip);
    settings.setCityCode(city);
    if (lat.length() > 0) settings.setLatitude(lat);
    if (lon.length() > 0) settings.setLongitude(lon);
    if (tz.length() > 0) settings.setTimezone(tz);
    if (owmApi.length() > 0) settings.setOwmApiKey(owmApi);
    if (ntpServer.length() > 0) settings.setNtpServer(ntpServer);

    _webServer->send(200, "text/html", html);
    delay(1000);

    ESP.restart();
#endif
}

void WifiManager::handleNotFound() {
#ifndef NATIVE_TEST
    _webServer->sendHeader("Location", "http://192.168.4.1/", true);
    _webServer->send(302, "text/plain", "");
#endif
}

/**
 * @brief Starts or stops the screenshot server based on the given flag.
 */
void WifiManager::applyScreenshotServerSetting(bool enabled) {
    // Web server is always running in STA mode for settings.
    // Screenshot endpoint checks this setting dynamically.
}

#ifndef NATIVE_TEST
void WifiManager::startWebServer() {
    if (_webServer) {
        stopWebServer();
    }
    _webServer = new WebServer(80);
    _webServer->on("/", [this]() { handleLanding(); });
    _webServer->on("/reset", [this]() {
        settings.factoryReset();
        WiFi.disconnect(true, true);
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html += "<title>CYD Weather Station</title>";
        html += "<style>body { font-family: 'Inter', sans-serif; background: #1e1e2e; color: #cdd6f4; text-align: center; padding: 50px; }</style>";
        html += "<meta http-equiv=\"refresh\" content=\"5;url=/\">";
        html += "</head><body><h2 style='color: #f5c2e7;'>Factory Reset Complete</h2><p>The device is restarting in AP Setup mode...</p></body></html>";
        _webServer->send(200, "text/html", html);
        delay(1000);
        ESP.restart();
    });
    _webServer->on("/clear_cache", [this]() {
        bool success = WeatherCache::clearCache();
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html += "<title>Cache Cleared</title>";
        html += "<style>";
        html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
        html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }";
        html += "h2 { margin-top: 0; margin-bottom: 20px; }";
        html += "p { color: #cdd6f4; margin-bottom: 20px; line-height: 1.5; }";
        html += "</style>";
        html += "<meta http-equiv=\"refresh\" content=\"3;url=/\">";
        html += "</head><body>";
        html += "<div class='card'>";
        if (success) {
            html += "<h2 style='color: #a6e3a1;'>Cache Cleared</h2>";
            html += "<p>The SD card cache has been successfully cleared.</p>";
        } else {
            html += "<h2 style='color: #f38ba8;'>Failed to Clear Cache</h2>";
            html += "<p>An error occurred while clearing the SD card cache.</p>";
        }
        html += "<p>Returning to dashboard...</p>";
        html += "</div></body></html>";
        _webServer->send(200, "text/html", html);
    });
    _webServer->on("/clear_logs", [this]() {
        bool success = WeatherLogger::clearLogs();
        String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
        html += "<title>Logs Cleared</title>";
        html += "<style>";
        html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
        html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }";
        html += "h2 { margin-top: 0; margin-bottom: 20px; }";
        html += "p { color: #cdd6f4; margin-bottom: 20px; line-height: 1.5; }";
        html += "</style>";
        html += "<meta http-equiv=\"refresh\" content=\"3;url=/\">";
        html += "</head><body>";
        html += "<div class='card'>";
        if (success) {
            html += "<h2 style='color: #a6e3a1;'>Logs Cleared</h2>";
            html += "<p>The SD card logs have been successfully cleared.</p>";
        } else {
            html += "<h2 style='color: #f38ba8;'>Failed to Clear Logs</h2>";
            html += "<p>An error occurred while clearing the SD card logs.</p>";
        }
        html += "<p>Returning to dashboard...</p>";
        html += "</div></body></html>";
        _webServer->send(200, "text/html", html);
    });
    _webServer->on("/screenshot", [this]() { handleScreenshot(); });
    _webServer->on("/settings", HTTP_GET, [this]() { handleSettings(); });
    _webServer->on("/settings/save", HTTP_POST, [this]() { handleSettingsSave(); });
    _webServer->on("/api/config", HTTP_GET, [this]() {
        if (!settings.getApiServerEnabled()) {
            _webServer->send(403, "text/plain", "Forbidden: API server disabled in settings");
            return;
        }
        DynamicJsonDocument doc(2048);
        doc["unit_system"] = settings.getUnitSystem();
        doc["brightness"] = settings.getBrightness();
        doc["auto_brightness"] = settings.getAutoBrightness();
        doc["timezone"] = settings.getTimezone();
        doc["theme_flavor"] = settings.getThemeFlavor();
        doc["sd_logging_enabled"] = settings.getSdLoggingEnabled();
        doc["screenshot_server_enabled"] = settings.getScreenshotServerEnabled();
        doc["api_server_enabled"] = settings.getApiServerEnabled();
        doc["screen_orientation"] = settings.getScreenOrientation();
        doc["led_enabled"] = settings.getLedEnabled();
        doc["led_brightness"] = (settings.getLedBrightness() * 100) / 255;
        doc["mqtt_enabled"] = settings.getMqttEnabled();
        doc["mqtt_server"] = settings.getMqttServer();
        doc["mqtt_port"] = settings.getMqttPort();
        doc["mqtt_user"] = settings.getMqttUser();
        doc["mqtt_password"] = settings.getMqttPassword();
        doc["mqtt_base_topic"] = settings.getMqttBaseTopic();
        doc["wifi_ssid"] = settings.getWifiSSID();
        doc["wifi_password"] = settings.getWifiPassword();
        doc["sd_cache_enabled"] = settings.getSdCacheEnabled();
        doc["screensaver_enabled"] = settings.getScreensaverEnabled();
        doc["screensaver_timeout"] = settings.getScreensaverTimeout();
        doc["sleep_schedule_enabled"] = settings.getSleepScheduleEnabled();
        doc["sleep_start_time"] = settings.getSleepStartTime();
        doc["sleep_end_time"] = settings.getSleepEndTime();
        doc["weather_update_interval"] = settings.getWeatherUpdateInterval();
        doc["static_ip_enabled"] = settings.getStaticIpEnabled();
        doc["static_ip"] = settings.getStaticIp();
        doc["static_gateway"] = settings.getStaticGateway();
        doc["static_subnet"] = settings.getStaticSubnet();
        doc["static_dns"] = settings.getStaticDns();
        doc["ap_password"] = settings.getApPassword();
        doc["zip_code"] = settings.getZipCode();
        doc["city_code"] = settings.getCityCode();
        doc["latitude"] = settings.getLatitude();
        doc["longitude"] = settings.getLongitude();
        doc["owm_api_key"] = settings.getOwmApiKey();
        doc["ntp_server"] = settings.getNtpServer();
        doc["local_sensor_enabled"] = settings.getLocalSensorEnabled();
        doc["local_sensor_type"] = settings.getLocalSensorType();
        doc["local_sensor_update_interval"] = settings.getLocalSensorUpdateInterval();
        doc["local_sensor_temp_offset"] = settings.getLocalSensorTempOffset();
        doc["local_sensor_hum_offset"] = settings.getLocalSensorHumOffset();

        String response;
        serializeJson(doc, response);
        _webServer->send(200, "application/json", response);
    });

    _webServer->on("/api/config", HTTP_POST, [this]() {
        if (!settings.getApiServerEnabled()) {
            _webServer->send(403, "text/plain", "Forbidden: API server disabled in settings");
            return;
        }
        if (!_webServer->hasArg("plain")) {
            _webServer->send(400, "text/plain", "Body not received");
            return;
        }
        
        DynamicJsonDocument doc(2048);
        DeserializationError error = deserializeJson(doc, _webServer->arg("plain"));
        if (error) {
            _webServer->send(400, "text/plain", "Invalid JSON");
            return;
        }
        
        if (doc.containsKey("unit_system")) settings.setUnitSystem(doc["unit_system"]);
        if (doc.containsKey("brightness")) settings.setBrightness(doc["brightness"]);
        if (doc.containsKey("auto_brightness")) settings.setAutoBrightness(doc["auto_brightness"]);
        if (doc.containsKey("timezone")) settings.setTimezone(doc["timezone"].as<String>());
        if (doc.containsKey("theme_flavor")) settings.setThemeFlavor(doc["theme_flavor"]);
        if (doc.containsKey("sd_logging_enabled")) settings.setSdLoggingEnabled(doc["sd_logging_enabled"]);
        if (doc.containsKey("screenshot_server_enabled")) settings.setScreenshotServerEnabled(doc["screenshot_server_enabled"]);
        if (doc.containsKey("api_server_enabled")) settings.setApiServerEnabled(doc["api_server_enabled"]);
        if (doc.containsKey("screen_orientation")) settings.setScreenOrientation(doc["screen_orientation"]);
        if (doc.containsKey("led_enabled")) settings.setLedEnabled(doc["led_enabled"]);
        if (doc.containsKey("led_brightness")) {
            int pct = doc["led_brightness"];
            settings.setLedBrightness((pct * 255) / 100);
        }
        if (doc.containsKey("mqtt_enabled")) settings.setMqttEnabled(doc["mqtt_enabled"]);
        if (doc.containsKey("mqtt_server")) settings.setMqttServer(doc["mqtt_server"].as<String>());
        if (doc.containsKey("mqtt_port")) settings.setMqttPort(doc["mqtt_port"]);
        if (doc.containsKey("mqtt_user")) settings.setMqttUser(doc["mqtt_user"].as<String>());
        if (doc.containsKey("mqtt_password")) settings.setMqttPassword(doc["mqtt_password"].as<String>());
        if (doc.containsKey("mqtt_base_topic")) settings.setMqttBaseTopic(doc["mqtt_base_topic"].as<String>());
        if (doc.containsKey("wifi_ssid")) settings.setWifiSSID(doc["wifi_ssid"].as<String>());
        if (doc.containsKey("wifi_password")) settings.setWifiPassword(doc["wifi_password"].as<String>());
        if (doc.containsKey("sd_cache_enabled")) settings.setSdCacheEnabled(doc["sd_cache_enabled"]);
        if (doc.containsKey("screensaver_enabled")) settings.setScreensaverEnabled(doc["screensaver_enabled"]);
        if (doc.containsKey("screensaver_timeout")) settings.setScreensaverTimeout(doc["screensaver_timeout"]);
        if (doc.containsKey("sleep_schedule_enabled")) settings.setSleepScheduleEnabled(doc["sleep_schedule_enabled"]);
        if (doc.containsKey("sleep_start_time")) settings.setSleepStartTime(doc["sleep_start_time"].as<String>());
        if (doc.containsKey("sleep_end_time")) settings.setSleepEndTime(doc["sleep_end_time"].as<String>());
        if (doc.containsKey("weather_update_interval")) settings.setWeatherUpdateInterval(doc["weather_update_interval"]);
        if (doc.containsKey("static_ip_enabled")) settings.setStaticIpEnabled(doc["static_ip_enabled"]);
        if (doc.containsKey("static_ip")) settings.setStaticIp(doc["static_ip"].as<String>());
        if (doc.containsKey("static_gateway")) settings.setStaticGateway(doc["static_gateway"].as<String>());
        if (doc.containsKey("static_subnet")) settings.setStaticSubnet(doc["static_subnet"].as<String>());
        if (doc.containsKey("static_dns")) settings.setStaticDns(doc["static_dns"].as<String>());
        if (doc.containsKey("ap_password")) settings.setApPassword(doc["ap_password"].as<String>());
        if (doc.containsKey("zip_code")) settings.setZipCode(doc["zip_code"].as<String>());
        if (doc.containsKey("city_code")) settings.setCityCode(doc["city_code"].as<String>());
        if (doc.containsKey("latitude")) settings.setLatitude(doc["latitude"].as<String>());
        if (doc.containsKey("longitude")) settings.setLongitude(doc["longitude"].as<String>());
        if (doc.containsKey("owm_api_key")) settings.setOwmApiKey(doc["owm_api_key"].as<String>());
        if (doc.containsKey("ntp_server")) settings.setNtpServer(doc["ntp_server"].as<String>());
        if (doc.containsKey("local_sensor_enabled")) settings.setLocalSensorEnabled(doc["local_sensor_enabled"]);
        if (doc.containsKey("local_sensor_type")) settings.setLocalSensorType(doc["local_sensor_type"]);
        if (doc.containsKey("local_sensor_update_interval")) settings.setLocalSensorUpdateInterval(doc["local_sensor_update_interval"]);
        if (doc.containsKey("local_sensor_temp_offset")) settings.setLocalSensorTempOffset(doc["local_sensor_temp_offset"]);
        if (doc.containsKey("local_sensor_hum_offset")) settings.setLocalSensorHumOffset(doc["local_sensor_hum_offset"]);
        
        _webServer->send(200, "application/json", "{\"status\":\"ok\"}");
    });
    _webServer->on("/api/tab", [this]() {
        if (!settings.getApiServerEnabled()) {
            _webServer->send(403, "text/plain", "Forbidden: API server disabled in settings");
            return;
        }
        if (_webServer->hasArg("index")) {
            int idx = _webServer->arg("index").toInt();
            setUIActiveTab(idx);
            _webServer->send(200, "text/plain", "Tab updated");
        } else {
            _webServer->send(400, "text/plain", "Missing index parameter");
        }
    });
    _webServer->on("/api/orientation", [this]() {
        if (!settings.getApiServerEnabled()) {
            _webServer->send(403, "text/plain", "Forbidden: API server disabled in settings");
            return;
        }
        if (_webServer->hasArg("val")) {
            int val = _webServer->arg("val").toInt();
            setUIOrientation(val);
            _webServer->send(200, "text/plain", "Orientation updated");
        } else {
            _webServer->send(400, "text/plain", "Missing val parameter");
        }
    });
    registerOTARoutes();
    _webServer->begin();
    Serial.println("[WiFi] Screenshot server started on port 80.");
}

void WifiManager::stopWebServer() {
    if (_webServer) {
        _webServer->stop();
        delete _webServer;
        _webServer = nullptr;
        Serial.println("[WiFi] Web server stopped.");
    }
}

void WifiManager::handleScreenshot() {
    if (!settings.getScreenshotServerEnabled()) {
        _webServer->send(403, "text/plain", "Forbidden: Screenshot server disabled in settings");
        return;
    }

    const char* tmpPath = "/~scr_tmp.bmp";

    // --- Capture the current screen to a temp BMP on SD first ---
    if (!ScreenshotManager::captureToSD(tmpPath)) {
        _webServer->send(500, "text/plain", "Error: Screenshot capture failed");
        return;
    }

    // --- Open the temp file and stream it over HTTP ---
    File f = SD.open(tmpPath, FILE_READ);
    if (!f) {
        _webServer->send(500, "text/plain", "Error: Cannot open temp screenshot file");
        SD.remove(tmpPath);
        return;
    }

    const uint32_t totalSize = f.size();
    _webServer->setContentLength(totalSize);
    _webServer->send(200, "image/bmp", "");

    // --- Stream file in 512-byte chunks to the client ---
    WiFiClient client = _webServer->client();
    uint8_t xferBuf[512];
    while (f.available()) {
        size_t n = f.read(xferBuf, sizeof(xferBuf));
        if (n > 0) {
            client.write(xferBuf, n);
        }
    }

    f.close();
    SD.remove(tmpPath);
    Serial.println("[WiFi] Screenshot streamed to remote client.");
}

void WifiManager::registerOTARoutes() {
    if (!_webServer) return;

    _webServer->on("/update", HTTP_GET, [this]() {
        String html = String(ota_html);
        html.replace("%DEVICE_NAME%", DEVICE_NAME);
        _webServer->send(200, "text/html", html);
    });

    _webServer->on("/settings", HTTP_GET, [this]() {
        Serial.println("[WiFi] Connected to /settings page");
        _webServer->send(200, "text/html", "<html><body><h1>Settings</h1><p>Not implemented</p></body></html>");
    });

    _webServer->on("/update", HTTP_POST, [this]() {
        _webServer->sendHeader("Connection", "close");
        if (Update.hasError()) {
            _webServer->send(500, "text/plain", String(Update.errorString()));
        } else {
            _webServer->send(200, "text/plain", "OK");
            delay(1000);
            ESP.restart();
        }
    }, [this]() {
        HTTPUpload& upload = _webServer->upload();
        if (upload.status == UPLOAD_FILE_START) {
            Serial.printf("Update start: %s\n", upload.filename.c_str());
            if (!Update.begin(UPDATE_SIZE_UNKNOWN)) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_WRITE) {
            if (Update.write(upload.buf, upload.currentSize) != upload.currentSize) {
                Update.printError(Serial);
            }
        } else if (upload.status == UPLOAD_FILE_END) {
            if (Update.end(true)) {
                Serial.printf("Update Success: %u bytes\nRebooting...\n", upload.totalSize);
            } else {
                Update.printError(Serial);
            }
        }
    });
}

void WifiManager::handleSettings() {
    String html = String(settings_html);
    
#ifdef APP_VERSION
    html.replace("%APP_VERSION%", APP_VERSION);
#else
    html.replace("%APP_VERSION%", "unknown");
#endif
    html.replace("%DEVICE_NAME%", DEVICE_NAME);

    html.replace("%UNIT_METRIC%", settings.getUnitSystem() == UNIT_METRIC ? "selected" : "");
    html.replace("%UNIT_IMPERIAL%", settings.getUnitSystem() == UNIT_IMPERIAL ? "selected" : "");
    
    html.replace("%THEME_MOCHA%", settings.getThemeFlavor() == CATPPUCCIN_MOCHA ? "selected" : "");
    html.replace("%THEME_MACCHIATO%", settings.getThemeFlavor() == CATPPUCCIN_MACCHIATO ? "selected" : "");
    html.replace("%THEME_FRAPPE%", settings.getThemeFlavor() == CATPPUCCIN_FRAPPE ? "selected" : "");
    html.replace("%THEME_LATTE%", settings.getThemeFlavor() == CATPPUCCIN_LATTE ? "selected" : "");
    
    html.replace("%ORIENT_0%", settings.getScreenOrientation() == 0 ? "selected" : "");
    html.replace("%ORIENT_1%", settings.getScreenOrientation() == 1 ? "selected" : "");
    html.replace("%ORIENT_2%", settings.getScreenOrientation() == 2 ? "selected" : "");
    html.replace("%ORIENT_3%", settings.getScreenOrientation() == 3 ? "selected" : "");
    
    html.replace("%BRIGHTNESS%", String(settings.getBrightness()));
    html.replace("%AUTO_BRIGHTNESS%", settings.getAutoBrightness() ? "checked" : "");
    html.replace("%SCREENSAVER_ENABLED%", settings.getScreensaverEnabled() ? "checked" : "");
    html.replace("%SLEEP_SCHEDULE_ENABLED%", settings.getSleepScheduleEnabled() ? "checked" : "");
    html.replace("%SLEEP_START_TIME%", settings.getSleepStartTime());
    html.replace("%SLEEP_END_TIME%", settings.getSleepEndTime());
    html.replace("%SCREENSAVER_TIMEOUT%", String(settings.getScreensaverTimeout() / 60000));
    
    html.replace("%OWM_API%", settings.getOwmApiKey());
    html.replace("%ZIP%", settings.getZipCode());
    html.replace("%CITY%", settings.getCityCode());
    html.replace("%LAT%", settings.getLatitude());
    html.replace("%LON%", settings.getLongitude());
    html.replace("%TZ%", settings.getTimezone());
    html.replace("%NTP_SERVER%", settings.getNtpServer());
    html.replace("%UPDATE_INTERVAL%", String(settings.getWeatherUpdateInterval()));

    html.replace("%LOCAL_SENSOR_ENABLED_CHECKED%", settings.getLocalSensorEnabled() ? "checked" : "");
    html.replace("%LOCAL_SENSOR_TYPE_1_SELECTED%", settings.getLocalSensorType() == 1 ? "selected" : "");
    html.replace("%LOCAL_SENSOR_TYPE_2_SELECTED%", settings.getLocalSensorType() == 2 ? "selected" : "");
    html.replace("%LOCAL_SENSOR_TYPE_3_SELECTED%", settings.getLocalSensorType() == 3 ? "selected" : "");
    html.replace("%LOCAL_SENSOR_UPDATE_INTERVAL%", String(settings.getLocalSensorUpdateInterval()));
    html.replace("%LOCAL_SENSOR_TEMP_OFFSET%", String(settings.getLocalSensorTempOffset(), 1));
    html.replace("%LOCAL_SENSOR_HUM_OFFSET%", String(settings.getLocalSensorHumOffset(), 1));
    
    html.replace("%LED_ENABLED%", settings.getLedEnabled() ? "checked" : "");
    html.replace("%LED_BRIGHTNESS%", String((settings.getLedBrightness() * 100) / 255));
    
    html.replace("%MQTT_ENABLED%", settings.getMqttEnabled() ? "checked" : "");
    html.replace("%MQTT_SERVER%", settings.getMqttServer());
    html.replace("%MQTT_PORT%", String(settings.getMqttPort()));
    html.replace("%MQTT_USER%", settings.getMqttUser());
    html.replace("%MQTT_PASSWORD%", settings.getMqttPassword());
    html.replace("%MQTT_BASE%", settings.getMqttBaseTopic());
    
    html.replace("%STATIC_IP_ENABLED%", settings.getStaticIpEnabled() ? "checked" : "");
    
    String staticIp = settings.getStaticIp();
    if (staticIp.isEmpty() && WiFi.status() == WL_CONNECTED) staticIp = WiFi.localIP().toString();
    html.replace("%STATIC_IP%", staticIp);
    
    String staticGw = settings.getStaticGateway();
    if (staticGw.isEmpty() && WiFi.status() == WL_CONNECTED) staticGw = WiFi.gatewayIP().toString();
    html.replace("%STATIC_GW%", staticGw);
    
    String staticSn = settings.getStaticSubnet();
    if (staticSn.isEmpty() && WiFi.status() == WL_CONNECTED) staticSn = WiFi.subnetMask().toString();
    html.replace("%STATIC_SN%", staticSn);
    
    String staticDns = settings.getStaticDns();
    if (staticDns.isEmpty() && WiFi.status() == WL_CONNECTED) staticDns = WiFi.dnsIP().toString();
    html.replace("%STATIC_DNS%", staticDns);
    
    html.replace("%AP_PASSWORD%", settings.getApPassword());
    
    html.replace("%SCREENSHOT_ENABLED%", settings.getScreenshotServerEnabled() ? "checked" : "");
    html.replace("%API_SERVER_ENABLED%", settings.getApiServerEnabled() ? "checked" : "");
    html.replace("%SD_LOGGING%", settings.getSdLoggingEnabled() ? "checked" : "");
    html.replace("%SD_CACHE%", settings.getSdCacheEnabled() ? "checked" : "");
    
    _webServer->send(200, "text/html", html);
}

void WifiManager::handleSettingsSave() {
    if (_webServer->hasArg("unit_system")) settings.setUnitSystem(_webServer->arg("unit_system").toInt());
    if (_webServer->hasArg("theme_flavor")) settings.setThemeFlavor(_webServer->arg("theme_flavor").toInt());
    if (_webServer->hasArg("screen_orientation")) settings.setScreenOrientation(_webServer->arg("screen_orientation").toInt());
    
    if (_webServer->hasArg("brightness")) settings.setBrightness(_webServer->arg("brightness").toInt());
    if (_webServer->hasArg("led_brightness")) {
        int pct = _webServer->arg("led_brightness").toInt();
        settings.setLedBrightness((pct * 255) / 100);
    }
    settings.setAutoBrightness(_webServer->hasArg("auto_brightness"));
    settings.setScreensaverEnabled(_webServer->hasArg("screensaver_enabled"));
    if (_webServer->hasArg("screensaver_timeout")) settings.setScreensaverTimeout(_webServer->arg("screensaver_timeout").toInt() * 60000);
    settings.setSleepScheduleEnabled(_webServer->hasArg("sleep_schedule_enabled"));
    if (_webServer->hasArg("sleep_start_time")) settings.setSleepStartTime(_webServer->arg("sleep_start_time"));
    if (_webServer->hasArg("sleep_end_time")) settings.setSleepEndTime(_webServer->arg("sleep_end_time"));
    
    if (_webServer->hasArg("owm_api")) settings.setOwmApiKey(_webServer->arg("owm_api"));
    if (_webServer->hasArg("zip")) settings.setZipCode(_webServer->arg("zip"));
    if (_webServer->hasArg("city")) settings.setCityCode(_webServer->arg("city"));
    if (_webServer->hasArg("lat")) settings.setLatitude(_webServer->arg("lat"));
    if (_webServer->hasArg("lon")) settings.setLongitude(_webServer->arg("lon"));
    if (_webServer->hasArg("tz")) settings.setTimezone(_webServer->arg("tz"));
    if (_webServer->hasArg("ntp_server")) settings.setNtpServer(_webServer->arg("ntp_server"));
    if (_webServer->hasArg("update_interval")) settings.setWeatherUpdateInterval(_webServer->arg("update_interval").toInt());
    
    settings.setLocalSensorEnabled(_webServer->hasArg("local_sensor_enabled"));
    if (_webServer->hasArg("local_sensor_type")) settings.setLocalSensorType(_webServer->arg("local_sensor_type").toInt());
    if (_webServer->hasArg("local_sensor_update_interval")) settings.setLocalSensorUpdateInterval(_webServer->arg("local_sensor_update_interval").toInt());
    if (_webServer->hasArg("local_sensor_temp_offset")) settings.setLocalSensorTempOffset(_webServer->arg("local_sensor_temp_offset").toFloat());
    if (_webServer->hasArg("local_sensor_hum_offset")) settings.setLocalSensorHumOffset(_webServer->arg("local_sensor_hum_offset").toFloat());
    
    settings.setLedEnabled(_webServer->hasArg("led_enabled"));

    
    settings.setMqttEnabled(_webServer->hasArg("mqtt_enabled"));
    if (_webServer->hasArg("mqtt_server")) settings.setMqttServer(_webServer->arg("mqtt_server"));
    if (_webServer->hasArg("mqtt_port")) settings.setMqttPort(_webServer->arg("mqtt_port").toInt());
    if (_webServer->hasArg("mqtt_user")) settings.setMqttUser(_webServer->arg("mqtt_user"));
    if (_webServer->hasArg("mqtt_password")) settings.setMqttPassword(_webServer->arg("mqtt_password"));
    if (_webServer->hasArg("mqtt_base")) settings.setMqttBaseTopic(_webServer->arg("mqtt_base"));
    
    settings.setStaticIpEnabled(_webServer->hasArg("static_ip_enabled"));
    if (_webServer->hasArg("static_ip")) settings.setStaticIp(_webServer->arg("static_ip"));
    if (_webServer->hasArg("static_gw")) settings.setStaticGateway(_webServer->arg("static_gw"));
    if (_webServer->hasArg("static_sn")) settings.setStaticSubnet(_webServer->arg("static_sn"));
    if (_webServer->hasArg("static_dns")) settings.setStaticDns(_webServer->arg("static_dns"));
    
    if (_webServer->hasArg("ap_password")) settings.setApPassword(_webServer->arg("ap_password"));
    
    settings.setScreenshotServerEnabled(_webServer->hasArg("screenshot_server_enabled"));
    settings.setApiServerEnabled(_webServer->hasArg("api_server_enabled"));
    settings.setSdLoggingEnabled(_webServer->hasArg("sd_logging_enabled"));
    settings.setSdCacheEnabled(_webServer->hasArg("sd_cache_enabled"));

    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
    html += "<title>Settings Saved</title>";
    html += "<style>";
    html += "body { font-family: 'Inter', system-ui, sans-serif; background: #1e1e2e; color: #cdd6f4; margin: 0; padding: 20px; display: flex; justify-content: center; align-items: center; min-height: 100vh; box-sizing: border-box; }";
    html += ".card { background: #181825; border-radius: 12px; padding: 30px; width: 100%; max-width: 400px; box-shadow: 0 8px 30px rgba(0,0,0,0.3); border: 1px solid #313244; text-align: center; }";
    html += "h2 { color: #a6e3a1; margin-top: 0; margin-bottom: 20px; }";
    html += "p { color: #cdd6f4; margin-bottom: 20px; line-height: 1.5; }";
    html += "</style>";
    html += "<script>";
    html += "setTimeout(() => {";
    html += "  let checkInterval = setInterval(() => {";
    html += "    fetch('/').then(response => {";
    html += "      if (response.ok) { clearInterval(checkInterval); window.location.href = '/'; }";
    html += "    }).catch(e => {});";
    html += "  }, 2000);";
    html += "}, 5000);"; // Wait 5 seconds before starting to poll
    html += "</script>";
    html += "</head><body>";
    html += "<div class='card'>";
    html += "<h2>Settings Saved</h2>";
    html += "<p>Your device configuration has been updated.</p>";
    html += "<p>Rebooting device... You will be redirected automatically once it comes back online.</p>";
    html += "</div>";
    html += "</body></html>";

    _webServer->send(200, "text/html", html);
    delay(2000);
    ESP.restart();
}

void WifiManager::handleLanding() {
    String html = landing_html;
    html.replace("%APP_VERSION%", APP_VERSION);
    html.replace("%DEVICE_NAME%", DEVICE_NAME);
    html.replace("%SCREENSHOT_DISABLED%", settings.getScreenshotServerEnabled() ? "" : "disabled");
    html.replace("%SCREENSHOT_HELP_TEXT%", settings.getScreenshotServerEnabled() ? "" : "<p style='font-size: 13px; color: #a6adc8; margin-top: -5px; margin-bottom: 15px;'>Screenshots must be enabled in device settings.</p>");
    _webServer->send(200, "text/html", html);
}
#endif
