# EsPong

A two-player (and solo) Pong game running on an ESP32. Connect via WiFi, open the captive portal on your phone, and play — no app install needed.

## Features

- **Two-player mode** — Each player claims a side (left/right) from their phone and controls their paddle by dragging
- **Solo mode** — Control both paddles on one device; score = rally hit count; game over on miss
- **Fire ball** — After 10 consecutive hits the ball catches fire and speeds up
- **Shrinking paddles** — Paddles shrink every 5 hits to keep rallies challenging
- **High scores** — Top 5 solo scores stored persistently on the ESP32 with 3-letter arcade-style initials
- **WiFi station mode** — Connect the ESP32 to your home WiFi (AP+STA dual mode) via an in-game settings panel while the AP stays active
- **Captive portal** — Phones auto-detect the AP as a captive portal and open the game page
- **Zero dependencies on the client** — Pure HTML/CSS/JS, no frameworks, works on any modern phone browser

## How It Works

The ESP32 runs a soft AP (`JASIU-PONG` by default). When a phone connects, the captive portal detection opens the game page automatically. The entire UI is a single `index.html` that gets gzipped and embedded into the firmware at build time.

Game state is synchronized via lightweight HTTP polling (`/s` endpoint) at ~12 Hz. Player 1 is authoritative for ball physics and score; Player 2 receives state updates.

## Hardware

- Any ESP32 dev board (ESP32-WROOM, ESP32-DevKitC, etc.)
- No other components needed — the game runs entirely over WiFi

## Building & Flashing

Requires [PlatformIO](https://platformio.org/).

```bash
# Build
pio run

# Upload to ESP32
pio run --target upload

# Monitor serial output
pio device monitor
```

## Project Structure

```
src/
  main.cpp            — Setup and main loop
  config.h            — WiFi SSID, channel, game tick rate
  wifi_manager.h/cpp  — AP+STA WiFi management, scan, connect
  game_manager.h/cpp  — Player slots and game state
  score_manager.h/cpp — Persistent high scores (NVS)
  web_server.h/cpp    — HTTP endpoints and game page serving
scripts/
  build_web.py        — Pre-build: minifies and gzips index.html into a C header
data/web/
  index.html          — The entire game UI (canvas + JS)
```

## API Endpoints

| Endpoint | Method | Description |
|----------|--------|-------------|
| `/` | GET | Serve the game page |
| `/claim?p=1` | GET | Claim player slot 1 or 2 |
| `/s?p=1&y=0.5&...` | GET | Poll/sync game state |
| `/api/highscores` | GET | Get top 5 high scores (JSON) |
| `/api/highscore` | POST | Submit a new high score |
| `/api/wifi/status` | GET | Get WiFi connection status |
| `/api/wifi/scan` | GET | Scan for nearby networks |
| `/api/wifi/connect` | POST | Connect to a WiFi network |
| `/api/wifi/disconnect` | POST | Disconnect and clear saved credentials |

## Configuration

Edit `src/config.h` to change:

- `WIFI_SSID` — AP network name (default: `JASIU-PONG`)
- `WIFI_CHANNEL` — WiFi channel (default: 6)
- `MAX_CLIENTS` — Max simultaneous AP clients (default: 4)
- `GAME_TICK_MS` — Game loop interval (default: 16ms / ~60 FPS)

## License

MIT
