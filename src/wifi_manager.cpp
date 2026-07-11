#include "wifi_manager.h"
#include <Arduino.h>
#include "settings_manager.h"

extern SettingsManager settings;

WifiManager::WifiManager(const char* ssid, const char* password)
    : _ssid(ssid), _password(password), _state(WIFI_STATE_DISCONNECTED), _lastReconnectAttempt(0), _connectionStartTime(0) {}

void WifiManager::begin() {
    Serial.println("[WiFi] Starting Wi-Fi Manager...");
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid.c_str(), _password.c_str());
    _state = WIFI_STATE_CONNECTING;
    _connectionStartTime = millis();
    Serial.printf("[WiFi] Connecting to %s...\n", _ssid.c_str());
}

void WifiManager::update() {
    wl_status_t status = WiFi.status();

    switch (_state) {
        case WIFI_STATE_DISCONNECTED:
            if (millis() - _lastReconnectAttempt > _reconnectInterval) {
                _lastReconnectAttempt = millis();
                Serial.println("[WiFi] Reconnecting...");
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
            }
            break;

        case WIFI_STATE_AP_MODE:
#ifndef NATIVE_TEST
            if (_dnsServer) _dnsServer->processNextRequest();
            if (_webServer) _webServer->handleClient();
#endif
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
    WiFi.disconnect();
    delay(100);

    WiFi.mode(WIFI_AP_STA);
    IPAddress apIP(192, 168, 4, 1);
    WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
    WiFi.softAP(apSSID.c_str());

    _dnsServer = new DNSServer();
    _dnsServer->start(53, "*", apIP);

    _webServer = new WebServer(80);
    _webServer->on("/", [this]() { handleRoot(); });
    _webServer->on("/save", [this]() { handleSave(); });
    _webServer->onNotFound([this]() { handleNotFound(); });
    _webServer->begin();

    Serial.println("[WiFi] AP Mode Web Server and DNS Server started.");
#endif
}

void WifiManager::handleRoot() {
#ifndef NATIVE_TEST
    int n = WiFi.scanComplete();
    if (n == -2) {
        WiFi.scanNetworks(true);
    }
    if (n <= 0) {
        n = WiFi.scanNetworks();
    }

    String html = "<!DOCTYPE html><html><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">";
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
    
    html += "<label>Select Network</label>";
    html += "<div class='net-list'>";
    if (n <= 0) {
        html += "<div class='net-item'>No networks found</div>";
    } else {
        for (int i = 0; i < n; ++i) {
            String ssidName = WiFi.SSID(i);
            int32_t rssi = WiFi.RSSI(i);
            html += "<div class='net-item' onclick='selectSSID(\"" + ssidName + "\")'>";
            html += "<span>" + ssidName + "</span>";
            html += "<span style='color: #a6adc8; font-size: 12px;'>" + String(rssi) + " dBm</span>";
            html += "</div>";
        }
    }
    html += "</div>";
    
    html += "<label for='ssid'>SSID</label>";
    html += "<input type='text' id='ssid' name='ssid' placeholder='SSID name' required>";
    
    html += "<label for='pass'>Password</label>";
    html += "<input type='password' id='pass' name='pass' placeholder='Password'>";
    
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

    ESP.restart();
#endif
}

void WifiManager::handleNotFound() {
#ifndef NATIVE_TEST
    _webServer->sendHeader("Location", "http://192.168.4.1/", true);
    _webServer->send(302, "text/plain", "");
#endif
}
