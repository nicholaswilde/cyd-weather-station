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
#include "version.h"
#include <ArduinoJson.h>
#endif

extern SettingsManager settings;

static void configureStaticIP() {
#ifndef NATIVE_TEST
#ifdef STATIC_IP
    IPAddress local_ip;
    if (local_ip.fromString(STATIC_IP)) {
        IPAddress gateway_ip;
        IPAddress subnet_ip;
        IPAddress dns_ip;

        #ifdef STATIC_GATEWAY
        gateway_ip.fromString(STATIC_GATEWAY);
        #endif
        #ifdef STATIC_SUBNET
        subnet_ip.fromString(STATIC_SUBNET);
        #endif
        #ifdef STATIC_DNS
        dns_ip.fromString(STATIC_DNS);
        #endif

        if (WiFi.config(local_ip, gateway_ip, subnet_ip, dns_ip)) {
            Serial.println("[WiFi] Static IP configured successfully.");
        } else {
            Serial.println("[WiFi] Failed to configure Static IP.");
        }
    }
#endif
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
#ifdef AP_PASSWORD
    if (strlen(AP_PASSWORD) >= 8) {
        apPass = AP_PASSWORD;
    }
#endif
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
    html += ".net-list { margin-bottom: 20px; max-height: 150px; overflow-y: auto; border: 1px solid #313244; border-radius: 6px; padding: 10px; background: #11111b; }";
    html += ".net-item { display: flex; justify-content: space-between; padding: 8px; cursor: pointer; border-bottom: 1px solid #1e1e2e; }";
    html += ".net-item:last-child { border-bottom: none; }";
    html += ".net-item:hover { background: #313244; color: #f5c2e7; }";
    html += "</style>";
    html += "<script>";
    html += "function selectSSID(ssid) { document.getElementById('ssid').value = ssid; }";
    html += "</script>";
    html += "</head><body>";
    html += "<div class='card'>";
    html += "<h2>Wi-Fi Configuration</h2>";
    html += "<form method='POST' action='/save'>";
    
    html += "<div style='display: flex; justify-content: space-between; align-items: center;'>";
    html += "<label style='margin-bottom: 0;'>Select Network</label>";
    html += "<a href='/scan' style='color: #cba6f7; font-size: 12px; text-decoration: none;'>🔄 Refresh List</a>";
    html += "</div>";
    html += "<div style='height: 8px;'></div>";
    
    html += "<div class='net-list'>";
    html += _cachedNetworksHTML;
    html += "</div>";
    
    html += "<label for='ssid'>SSID</label>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='SSID name' required>";
    
    html += "<label for='pass'>Password</label>";
    html += "<input type='password' id='pass' name='pass' placeholder='Password'>";
    
    html += "<label for='owm_api'>OpenWeatherMap API Key</label>";
    html += "<input type='password' id='owm_api' name='owm_api' placeholder='API Key' value='" + settings.getOwmApiKey() + "'>";
    
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

    _webServer->send(200, "text/html", html);
    delay(2000);

    settings.setWifiSSID(ssid);
    settings.setWifiPassword(pass);
    settings.setZipCode(zip);
    settings.setCityCode(city);
    if (lat.length() > 0) settings.setLatitude(lat);
    if (lon.length() > 0) settings.setLongitude(lon);
    if (tz.length() > 0) settings.setTimezone(tz);
    if (owmApi.length() > 0) settings.setOwmApiKey(owmApi);
    if (ntpServer.length() > 0) settings.setNtpServer(ntpServer);

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
    _webServer->on("/screenshot", [this]() { handleScreenshot(); });
    _webServer->on("/settings", HTTP_GET, [this]() { handleSettings(); });
    _webServer->on("/settings/save", HTTP_POST, [this]() { handleSettingsSave(); });
    _webServer->on("/api/config", [this]() {
        if (!settings.getApiServerEnabled()) {
            _webServer->send(403, "text/plain", "Forbidden: API server disabled in settings");
            return;
        }
        StaticJsonDocument<512> doc;
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
        doc["led_brightness"] = settings.getLedBrightness();
        doc["mqtt_enabled"] = settings.getMqttEnabled();
        doc["wifi_ssid"] = settings.getWifiSSID();
        doc["sd_cache_enabled"] = settings.getSdCacheEnabled();
        doc["screensaver_enabled"] = settings.getScreensaverEnabled();
        doc["weather_update_interval"] = settings.getWeatherUpdateInterval();

        String response;
        serializeJson(doc, response);
        _webServer->send(200, "application/json", response);
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
        _webServer->send_P(200, "text/html", ota_html);
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

    html.replace("%UNIT_METRIC%", settings.getUnitSystem() == 0 ? "selected" : "");
    html.replace("%UNIT_IMPERIAL%", settings.getUnitSystem() == 1 ? "selected" : "");
    
    html.replace("%THEME_MOCHA%", settings.getThemeFlavor() == 0 ? "selected" : "");
    html.replace("%THEME_MACCHIATO%", settings.getThemeFlavor() == 1 ? "selected" : "");
    html.replace("%THEME_FRAPPE%", settings.getThemeFlavor() == 2 ? "selected" : "");
    html.replace("%THEME_LATTE%", settings.getThemeFlavor() == 3 ? "selected" : "");
    
    html.replace("%ORIENT_0%", settings.getScreenOrientation() == 0 ? "selected" : "");
    html.replace("%ORIENT_1%", settings.getScreenOrientation() == 1 ? "selected" : "");
    html.replace("%ORIENT_2%", settings.getScreenOrientation() == 2 ? "selected" : "");
    html.replace("%ORIENT_3%", settings.getScreenOrientation() == 3 ? "selected" : "");
    
    html.replace("%BRIGHTNESS%", String(settings.getBrightness()));
    html.replace("%AUTO_BRIGHTNESS%", settings.getAutoBrightness() ? "checked" : "");
    html.replace("%SCREENSAVER_ENABLED%", settings.getScreensaverEnabled() ? "checked" : "");
    
    html.replace("%OWM_API%", settings.getOwmApiKey());
    html.replace("%ZIP%", settings.getZipCode());
    html.replace("%CITY%", settings.getCityCode());
    html.replace("%LAT%", settings.getLatitude());
    html.replace("%LON%", settings.getLongitude());
    html.replace("%TZ%", settings.getTimezone());
    html.replace("%NTP_SERVER%", settings.getNtpServer());
    html.replace("%UPDATE_INTERVAL%", String(settings.getWeatherUpdateInterval()));
    
    html.replace("%LED_ENABLED%", settings.getLedEnabled() ? "checked" : "");
    html.replace("%LED_BRIGHTNESS%", String(settings.getLedBrightness()));
    
    html.replace("%MQTT_ENABLED%", settings.getMqttEnabled() ? "checked" : "");
    html.replace("%MQTT_SERVER%", settings.getMqttServer());
    html.replace("%MQTT_PORT%", String(settings.getMqttPort()));
    html.replace("%MQTT_USER%", settings.getMqttUser());
    html.replace("%MQTT_PASSWORD%", settings.getMqttPassword());
    
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
    settings.setAutoBrightness(_webServer->hasArg("auto_brightness"));
    settings.setScreensaverEnabled(_webServer->hasArg("screensaver_enabled"));
    
    if (_webServer->hasArg("owm_api")) settings.setOwmApiKey(_webServer->arg("owm_api"));
    if (_webServer->hasArg("zip")) settings.setZipCode(_webServer->arg("zip"));
    if (_webServer->hasArg("city")) settings.setCityCode(_webServer->arg("city"));
    if (_webServer->hasArg("lat")) settings.setLatitude(_webServer->arg("lat"));
    if (_webServer->hasArg("lon")) settings.setLongitude(_webServer->arg("lon"));
    if (_webServer->hasArg("tz")) settings.setTimezone(_webServer->arg("tz"));
    if (_webServer->hasArg("ntp_server")) settings.setNtpServer(_webServer->arg("ntp_server"));
    if (_webServer->hasArg("update_interval")) settings.setWeatherUpdateInterval(_webServer->arg("update_interval").toInt());
    
    settings.setLedEnabled(_webServer->hasArg("led_enabled"));
    if (_webServer->hasArg("led_brightness")) settings.setLedBrightness(_webServer->arg("led_brightness").toInt());
    
    settings.setMqttEnabled(_webServer->hasArg("mqtt_enabled"));
    if (_webServer->hasArg("mqtt_server")) settings.setMqttServer(_webServer->arg("mqtt_server"));
    if (_webServer->hasArg("mqtt_port")) settings.setMqttPort(_webServer->arg("mqtt_port").toInt());
    if (_webServer->hasArg("mqtt_user")) settings.setMqttUser(_webServer->arg("mqtt_user"));
    if (_webServer->hasArg("mqtt_password")) settings.setMqttPassword(_webServer->arg("mqtt_password"));
    
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
    html += "</style></head><body>";
    html += "<div class='card'>";
    html += "<h2>Settings Saved</h2>";
    html += "<p>Your device configuration has been updated.</p>";
    html += "<p>Rebooting device...</p>";
    html += "</div>";
    html += "</body></html>";

    _webServer->send(200, "text/html", html);
    delay(2000);
    ESP.restart();
}
#endif
