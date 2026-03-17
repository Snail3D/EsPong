#pragma once

#include <Arduino.h>
#include <ArduinoJson.h>

struct HighScore {
    char name[4]; // 3 letters + null
    int score;
};

class ScoreManager {
public:
    void begin();
    String getScoresJson();
    bool addScore(const char* name, int score);

private:
    static const int MAX_SCORES = 3;
    HighScore _scores[MAX_SCORES];
    int _count = 0;
    void _load();
    void _save();
};
