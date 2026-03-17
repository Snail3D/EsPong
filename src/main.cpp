#include <Arduino.h>
#include "config.h"
#include "wifi_manager.h"
#include "game_manager.h"
#include "score_manager.h"
#include "web_server.h"

WiFiManager wifiMgr;
GameManager gameMgr;
ScoreManager scoreMgr;
WebServer webSrv;

void setup() {
    Serial.begin(115200);
    delay(500);
    Serial.println("\n=== JASIU PONG ===");

    wifiMgr.begin();
    gameMgr.begin();
    scoreMgr.begin();
    webSrv.begin(gameMgr, scoreMgr, wifiMgr);

    Serial.println("Ready! Connect to WiFi: " WIFI_SSID);
}

void loop() {
    gameMgr.loop();
}
