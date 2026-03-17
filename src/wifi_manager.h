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

private:
    void _loadCreds();
    void _saveCreds(const char* ssid, const char* pass);
    void _clearCreds();
};
