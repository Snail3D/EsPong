#pragma once

#include <ESPAsyncWebServer.h>
#include "game_manager.h"
#include "score_manager.h"
#include "wifi_manager.h"
#include "config.h"

class WebServer {
public:
    void begin(GameManager& game, ScoreManager& scores, WiFiManager& wifi);

private:
    AsyncWebServer _server{HTTP_PORT};
    GameManager* _game = nullptr;
    ScoreManager* _scores = nullptr;
    WiFiManager* _wifi = nullptr;
};
