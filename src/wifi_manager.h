#pragma once

#include <WiFi.h>
#include <ArduinoJson.h>
#include "config.h"

class WiFiManager {
public:
    void begin();
    String scan();
    bool connect(const char* ssid, const char* pass);
    void disconnect();
    String getStaStatus();

    // AP config
    String getGameName();
    String getApSsid();
    bool hasApPass();
    String getConfigJson();
    void setConfig(const char* gameName, const char* apSsid, const char* apPass);

private:
    void _saveCreds(const char* ssid, const char* pass);
    void _clearCreds();
    void _startAP();
};
