#include "game_manager.h"

void GameManager::begin() {
    Serial.println("GameManager ready");
}

void GameManager::loop() {
}

bool GameManager::claimPlayer(uint8_t player) {
    if (player < 1 || player > 2) return false;
    uint8_t idx = player - 1;
    if (_state.taken[idx]) return false;
    _state.taken[idx] = true;
    _state.playerCount++;
    Serial.printf("Player %d claimed (total: %d)\n", player, _state.playerCount);
    return true;
}

void GameManager::heartbeat(uint8_t player, float y) {
    if (player < 1 || player > 2) return;
    _state.paddleY[player - 1] = y;
}
