# Fortnite Stream Analyser

A real-time C++ computer vision app that analyses live Fortnite gameplay and extracts performance stats.

## Instructions
- **REQUIRED** — add your own fortnite clip at the root repository, named as gameplay.mp4

## Goals

Extract live gameplay statistics:

- **Shot tracking** — shots fired (ammo deltas), shots landed (hitmarkers), accuracy %, headshot % (hitmarker colour)
- **Health, shield & materials** — damage taken, material economy (farmed vs used)
- **Damage dealt** — floating damage numbers, damage dealt vs taken ratio
- **Nearby players** — count visual audio footstep indicators
- **Kill feed & leaderboard** — eliminations and per-player kill counts via character matching
- **Zone status** — in/out of storm
- **Fight analysis** *(aspirational)* — per-fight aggregation

## Tech Stack

- **Language:** C++20
- **CV:** OpenCV (template matching)
- **Video source:** local file, webcam, or HLS stream (streamlink/FFmpeg for Twitch/YouTube)
- **Build:** CMake

## Build

```bash
mkdir build && cd build
cmake ..
make
```
