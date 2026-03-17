#pragma once

#include <Arduino.h>

struct GameState {
    float paddleY[2] = {0.5f, 0.5f};
    float ballX = 0.5f, ballY = 0.5f;
    float ballVX = 0.0f, ballVY = 0.0f;
    int score1 = 0, score2 = 0;
    bool taken[2] = {false, false};
    uint8_t playerCount = 0;
};

class GameManager {
public:
    void begin();
    void loop();

    // Returns true if slot was free
    bool claimPlayer(uint8_t player);
    void heartbeat(uint8_t player, float y);
    GameState& state() { return _state; }

private:
    GameState _state;
};
