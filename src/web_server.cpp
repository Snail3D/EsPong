#include "web_server.h"
#include "web_content.h"

void WebServer::begin(GameManager& game, ScoreManager& scores, WiFiManager& wifi) {
    _game = &game;
    _scores = &scores;
    _wifi = &wifi;

    // Serve game page
    _server.on("/", HTTP_GET, [](AsyncWebServerRequest* request) {
        AsyncWebServerResponse* response = request->beginResponse(
            200, "text/html", WEB_CONTENT, WEB_CONTENT_SIZE);
        response->addHeader("Content-Encoding", "gzip");
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
    });

    // Claim a player slot: /claim?p=1 or /claim?p=2
    _server.on("/claim", HTTP_GET, [this](AsyncWebServerRequest* request) {
        if (!request->hasParam("p")) { request->send(400); return; }
        uint8_t p = request->getParam("p")->value().toInt();
        bool ok = _game->claimPlayer(p);
        request->send(200, "text/plain", ok ? "ok" : "taken");
    });

    // Poll: /s?p=1&y=0.5&bx=..&by=..&bvx=..&bvy=..&s1=..&s2=..
    _server.on("/s", HTTP_GET, [this](AsyncWebServerRequest* request) {
        uint8_t p = 0;
        float y = 0.5f;
        if (request->hasParam("p")) p = request->getParam("p")->value().toInt();
        if (request->hasParam("y")) y = request->getParam("y")->value().toFloat();
        if (p > 0) _game->heartbeat(p, y);

        // P1 sends ball + score state
        GameState& s = _game->state();
        if (p == 1) {
            if (request->hasParam("bx")) s.ballX = request->getParam("bx")->value().toFloat();
            if (request->hasParam("by")) s.ballY = request->getParam("by")->value().toFloat();
            if (request->hasParam("bvx")) s.ballVX = request->getParam("bvx")->value().toFloat();
            if (request->hasParam("bvy")) s.ballVY = request->getParam("bvy")->value().toFloat();
            if (request->hasParam("s1")) s.score1 = request->getParam("s1")->value().toInt();
            if (request->hasParam("s2")) s.score2 = request->getParam("s2")->value().toInt();
        }

        char buf[256];
        snprintf(buf, sizeof(buf), "%d,%.3f,%.3f,%d,%d,%.4f,%.4f,%.5f,%.5f,%d,%d",
            s.playerCount, s.paddleY[0], s.paddleY[1],
            s.taken[0] ? 1 : 0, s.taken[1] ? 1 : 0,
            s.ballX, s.ballY, s.ballVX, s.ballVY,
            s.score1, s.score2);

        AsyncWebServerResponse* response = request->beginResponse(200, "text/plain", buf);
        response->addHeader("Cache-Control", "no-store");
        request->send(response);
    });

    // --- High Score endpoints ---
    _server.on("/api/highscores", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", _scores->getScoresJson());
    });

    _server.on("/api/highscore", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!request->hasParam("name", true) || !request->hasParam("score", true)) {
            request->send(400);
            return;
        }
        String name = request->getParam("name", true)->value();
        int score = request->getParam("score", true)->value().toInt();
        bool ok = _scores->addScore(name.c_str(), score);
        request->send(200, "text/plain", ok ? "ok" : "no");
    });

    // --- WiFi endpoints ---
    _server.on("/api/wifi/status", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", _wifi->getStaStatus());
    });

    _server.on("/api/wifi/scan", HTTP_GET, [this](AsyncWebServerRequest* request) {
        request->send(200, "application/json", _wifi->scan());
    });

    _server.on("/api/wifi/connect", HTTP_POST, [this](AsyncWebServerRequest* request) {
        if (!request->hasParam("ssid", true)) {
            request->send(400);
            return;
        }
        String ssid = request->getParam("ssid", true)->value();
        String pass = request->hasParam("pass", true) ? request->getParam("pass", true)->value() : "";
        bool ok = _wifi->connect(ssid.c_str(), pass.c_str());
        request->send(200, "text/plain", ok ? "ok" : "fail");
    });

    _server.on("/api/wifi/disconnect", HTTP_POST, [this](AsyncWebServerRequest* request) {
        _wifi->disconnect();
        request->send(200, "text/plain", "ok");
    });

    // --- PWA endpoints ---
    _server.on("/manifest.json", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "application/json",
            "{\"name\":\"JASIU PONG\",\"short_name\":\"Pong\","
            "\"start_url\":\"/\",\"display\":\"fullscreen\","
            "\"orientation\":\"portrait\","
            "\"background_color\":\"#0a0a2e\",\"theme_color\":\"#0a0a2e\","
            "\"icons\":[{\"src\":\"/icon.svg\",\"sizes\":\"any\",\"type\":\"image/svg+xml\",\"purpose\":\"any\"}]}");
    });

    _server.on("/icon.svg", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "image/svg+xml",
            "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 100 100'>"
            "<rect width='100' height='100' rx='20' fill='#0a0a2e'/>"
            "<text x='50' y='42' text-anchor='middle' font-size='40'>&#127955;</text>"
            "<text x='50' y='82' text-anchor='middle' font-size='40'>&#127952;</text>"
            "</svg>");
    });

    _server.on("/sw.js", HTTP_GET, [](AsyncWebServerRequest* request) {
        request->send(200, "application/javascript",
            "const CACHE='pong-v1';"
            "self.addEventListener('install',e=>{self.skipWaiting();});"
            "self.addEventListener('activate',e=>{e.waitUntil(clients.claim());});"
            "self.addEventListener('fetch',e=>{"
            "e.respondWith(fetch(e.request).then(r=>{return r;}).catch(()=>caches.match(e.request)));"
            "});");
    });

    // Tell phones "internet works" so captive portal popup doesn't hijack us
    _server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(204); });
    _server.on("/gen_204", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(204); });
    _server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest* r) {
        r->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
    _server.on("/library/test/success.html", HTTP_GET, [](AsyncWebServerRequest* r) {
        r->send(200, "text/html", "<HTML><HEAD><TITLE>Success</TITLE></HEAD><BODY>Success</BODY></HTML>");
    });
    _server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(200, "text/plain", "Microsoft Connect Test"); });
    _server.on("/fwlink", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(200); });
    _server.on("/redirect", HTTP_GET, [](AsyncWebServerRequest* r) { r->send(200); });
    _server.onNotFound([](AsyncWebServerRequest* r) { r->send(404); });

    _server.begin();
    Serial.println("Web server started");
}
