#include "wifi_manager.h"
#include <Preferences.h>

static Preferences wifiPrefs;

void WiFiManager::begin() {
    wifiPrefs.begin("wificreds", false);

    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(WIFI_SSID, nullptr, WIFI_CHANNEL, 0, MAX_CLIENTS);
    Serial.printf("AP started: %s @ %s\n", WIFI_SSID,
                  WiFi.softAPIP().toString().c_str());

    // Try saved STA credentials
    String ssid = wifiPrefs.getString("ssid", "");
    String pass = wifiPrefs.getString("pass", "");
    if (ssid.length() > 0) {
        Serial.printf("Connecting to saved WiFi: %s\n", ssid.c_str());
        WiFi.begin(ssid.c_str(), pass.c_str());
        unsigned long start = millis();
        while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
            delay(250);
        }
        if (WiFi.status() == WL_CONNECTED) {
            Serial.printf("STA connected: %s\n", WiFi.localIP().toString().c_str());
        } else {
            Serial.println("STA connection failed");
        }
    }
}

String WiFiManager::scan() {
    int n = WiFi.scanNetworks();
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < n; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["ssid"] = WiFi.SSID(i);
        o["rssi"] = WiFi.RSSI(i);
        o["open"] = (WiFi.encryptionType(i) == WIFI_AUTH_OPEN);
    }
    WiFi.scanDelete();
    String out;
    serializeJson(doc, out);
    return out;
}

bool WiFiManager::connect(const char* ssid, const char* pass) {
    WiFi.begin(ssid, pass);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 10000) {
        delay(250);
    }
    if (WiFi.status() == WL_CONNECTED) {
        _saveCreds(ssid, pass);
        Serial.printf("STA connected: %s\n", WiFi.localIP().toString().c_str());
        return true;
    }
    return false;
}

void WiFiManager::disconnect() {
    WiFi.disconnect(false);
    _clearCreds();
    Serial.println("STA disconnected, creds cleared");
}

String WiFiManager::getStaStatus() {
    JsonDocument doc;
    doc["connected"] = (WiFi.status() == WL_CONNECTED);
    doc["ssid"] = WiFi.SSID();
    doc["ip"] = WiFi.localIP().toString();
    doc["apIp"] = WiFi.softAPIP().toString();
    String out;
    serializeJson(doc, out);
    return out;
}

void WiFiManager::_saveCreds(const char* ssid, const char* pass) {
    wifiPrefs.putString("ssid", ssid);
    wifiPrefs.putString("pass", pass);
}

void WiFiManager::_clearCreds() {
    wifiPrefs.remove("ssid");
    wifiPrefs.remove("pass");
}

void WiFiManager::_loadCreds() {
    // Used internally by begin()
}
