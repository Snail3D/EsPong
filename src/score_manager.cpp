#include "score_manager.h"
#include <Preferences.h>

static Preferences prefs;

void ScoreManager::begin() {
    prefs.begin("hiscore", false);
    _load();
}

void ScoreManager::_load() {
    _count = prefs.getInt("count", 0);
    if (_count > MAX_SCORES) _count = MAX_SCORES;
    for (int i = 0; i < _count; i++) {
        char kn[8], ks[8];
        snprintf(kn, sizeof(kn), "n%d", i);
        snprintf(ks, sizeof(ks), "s%d", i);
        String n = prefs.getString(kn, "AAA");
        n.toCharArray(_scores[i].name, 4);
        _scores[i].score = prefs.getInt(ks, 0);
    }
}

void ScoreManager::_save() {
    prefs.putInt("count", _count);
    for (int i = 0; i < _count; i++) {
        char kn[8], ks[8];
        snprintf(kn, sizeof(kn), "n%d", i);
        snprintf(ks, sizeof(ks), "s%d", i);
        prefs.putString(kn, _scores[i].name);
        prefs.putInt(ks, _scores[i].score);
    }
}

String ScoreManager::getScoresJson() {
    JsonDocument doc;
    JsonArray arr = doc.to<JsonArray>();
    for (int i = 0; i < _count; i++) {
        JsonObject o = arr.add<JsonObject>();
        o["name"] = _scores[i].name;
        o["score"] = _scores[i].score;
    }
    String out;
    serializeJson(doc, out);
    return out;
}

bool ScoreManager::addScore(const char* name, int score) {
    // Check if it qualifies
    if (_count >= MAX_SCORES && score <= _scores[_count - 1].score) {
        return false;
    }

    // Find insertion point (sorted descending)
    int pos = 0;
    while (pos < _count && _scores[pos].score >= score) pos++;

    // Shift down
    if (_count < MAX_SCORES) _count++;
    for (int i = _count - 1; i > pos; i--) {
        _scores[i] = _scores[i - 1];
    }

    // Insert
    strncpy(_scores[pos].name, name, 3);
    _scores[pos].name[3] = '\0';
    _scores[pos].score = score;

    _save();
    return true;
}
