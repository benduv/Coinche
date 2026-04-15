# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build

The project uses CMake + Qt6 (MinGW on Windows). The main build directory is `build/` and includes both the app and all tests.

```bash
# Configure (first time or after CMakeLists changes)
cmake -S . -B build

# Build everything (client + server + tests)
cmake --build build

# Build only the server
cmake --build build --target server

# Build only the client
cmake --build build --target coinche
```

## Tests

All tests are compiled into `build/tests/`. The helper script compiles and runs them:

```bash
# Run all tests
cd tests && bash run_tests.sh

# Run a specific test suite
cd tests && bash run_tests.sh gameserver
cd tests && bash run_tests.sh belote_score
cd tests && bash run_tests.sh scorecalculator
cd tests && bash run_tests.sh private_lobby_integration
```

Available test targets (passed as argument to `run_tests.sh`):
`gameserver`, `capot`, `coinche`, `belote_mode`, `scorecalculator`, `belote_score`, `databasemanager`, `networkmanager`, `networkmanager_integration`, `gameserver_integration`, `coinche_tests`, `private_lobby_integration`, `friends`, `friends_integration`, `server_rejection`, `auth`, `belote_bidding_integration`

To build a single test target without running it:
```bash
cmake --build build --target test_belote_score
```

## Run (multiplayer testing)

```powershell
# Launch server + 4 auto-connected clients in a 2x2 grid
.\test_multiplayer.ps1

# Client with auto-login
.\build\coinche.exe --email aaa@aaa.fr --password aaaaaa
```

Test accounts: `aaa@aaa.fr`/`aaaaaa`, `bbb@bbb.fr`/`bbbbbb`, `ccc@ccc.fr`/`cccccc`, `ddd@ddd.fr`/`dddddd`

## Server deployment

```bash
# Deploy to test server (branch: feature/belote)
bash MAJAndBuildServer.sh test

# Deploy to production (must be in /home/coinche/Coinche, branch: main)
bash MAJAndBuildServer.sh
```

## Architecture

### Libraries

The build produces three static libraries with clear separation of concerns:

- **`coinche_common`** (`Carte`, `Deck`, `Player`) — pure game logic, no Qt UI. Used by both client and server. This is the only layer shared.
- **`coinche_client`** (`GameModel`, `HandModel`) — QObject/QML-facing models for the client only. Depends on `coinche_common`.
- **`server`** executable — links only `coinche_common`, never `coinche_client`.

### Server side (`server/`)

| File | Role |
|---|---|
| `GameServer.cpp/.h` | WebSocket server, orchestrates all game rooms and player connections |
| `ScoreCalculator.h` | Header-only, stateless score calculation for both Coinche and Belote rules |
| `DatabaseManager.cpp/.h` | SQLite via Qt Sql — accounts, stats, friends |
| `NetworkManager.cpp/.h` | **Client-side** WebSocket wrapper (confusingly lives in `server/`) |
| `StatsReporter.cpp/.h` | Post-game stat persistence |
| `SmtpClient.cpp/.h` | Email verification on account creation |
| `LogManager.h` | Header-only server logging |

Key structs in `GameServer.h`:
- `PlayerConnection` — one WebSocket connection (network layer only, no game logic)
- `GameRoom` — a live game: holds 4 `Player` objects, the `Deck`, bidding state, scores, pli tracking, bot flags, and `isBeloteMode`

Teams are always **players 0&2 vs players 1&3** (`scoreTeam1` = team of players 0 and 2).

### Client side

- **`NetworkManager`** (header-only on client) — owns the `QWebSocket`, handles all incoming JSON messages from the server, and drives `GameModel`.
- **`GameModel`** — central Qt model exposed to QML. Holds all UI-visible game state (hands, scores, bidding phase, animations, timers).
- **`HandModel`** — `QAbstractListModel` subclass for a player's cards.
- **QML** (`qml/`) — full UI. `CoincheView.qml` and `BeloteView.qml` are the in-game screens; `MainMenu.qml` is the entry point.

### Card representation

```cpp
enum Couleur { COEUR=3, TREFLE=4, CARREAU=5, PIQUE=6, COULEURINVALIDE=7 }
enum Chiffre { SEPT=7, HUIT=8, NEUF=9, DIX=10, VALET=11, DAME=12, ROI=13, AS=14 }
```

Card suit integers 7 and 8 in the bidding protocol mean **Tout Atout** and **Sans Atout** respectively (not actual card suits).

### Two game modes

- **Coinche** (default): classic bidding with annonces (80–160, Capot, Générale), Coinche/Surcoinche multipliers. `ScoreCalculator::calculateMancheScore()` handles all Coinche scoring rules.
- **Belote**: bidding via Prendre/Passer with a face-up card (`retournee`), two-round bid system (`beloteBidRound`). Separate score logic. `isBeloteMode` flag propagates from `GameRoom` → `GameServer` → `NetworkManager` → `GameModel`.

## Environment configuration

To switch between environments (local dev, test server, production), edit the single `environment` property in [qml/Config.qml](qml/Config.qml):

```qml
property string environment: "localhost"   // dev local
// property string environment: "test-remote"  // test.nebuludik.fr
// property string environment: "remote"        // game.nebuludik.fr (prod)
```

Available environments: `localhost` (WS), `emulator` (Android emulator), `local-network`, `test-remote` (WSS), `remote` (WSS prod). This is the **only file to change** when switching environments.

## QML context properties

The following C++ objects are exposed to QML via `setContextProperty` in `main.cpp`:

| Name | Type | Notes |
|---|---|---|
| `networkManager` | `NetworkManager*` | Always available |
| `gameModel` | `GameModel*` | **Initially `null`** — set only after `networkManager.gameModelReady` fires. Always guard with `if (gameModel !== null)` in QML. |
| `windowPositioner` | `WindowPositioner*` | Desktop only |
| `orientationHelper` | `OrientationHelper*` | Android orientation control via JNI |

Persistent settings (avatar, credentials, preferences) are stored via `QSettings("Nebuludik", "CoincheDelEspace")`.

## C++ and Qt conventions

### Memory management

Prefer smart pointers (`std::unique_ptr`, `std::shared_ptr`) over raw pointers for heap-allocated objects, especially in new code. The codebase currently uses raw pointers in many places (e.g. `std::vector<Carte*>`, `Player*`) — this is a known technical debt. When touching existing code, migrate to smart pointers where feasible without breaking the surrounding logic.

Exception: `QObject`-derived objects that have a Qt parent do **not** need smart pointers — Qt manages their lifetime via the parent-child tree. Never wrap a parented `QObject` in a `std::unique_ptr`.

### Qt-specific rules

- **Threading**: never use `std::thread`. Use `QThread` or Qt's `QThreadPool`/`QtConcurrent` so the event loop and signals/slots work correctly.
- **`Q_PROPERTY`**: every property must have its `NOTIFY` signal wired up and emitted whenever the value changes, otherwise QML bindings silently break.
- **`QObject` lifetime**: objects with a Qt parent are deleted by their parent — do not `delete` them manually or wrap them in `std::unique_ptr`. Objects without a parent (e.g. top-level `QWebSocket`) must be deleted explicitly or use `std::unique_ptr`.
- **Signals/slots across threads**: use `Qt::QueuedConnection` (or let Qt auto-detect) when connecting signals across thread boundaries — never call UI methods directly from a non-GUI thread.

### Timing synchronization

`GameServer.h` and `GameModel.h` both define timing constants for the card deal animation. **They must stay in sync** — if you change one, change the other:

| Constant | `GameServer.h` | `GameModel.h` |
|---|---|---|
| `GOOD_GAME_DELAY_MS` | 5000 | 4000 |
| `DEAL_CARD_INTERVAL_MS` | 920 | 720 |

The server value is intentionally slightly larger to account for network latency.

### Client version

`GameServer::MIN_CLIENT_VERSION` (currently `7`) must be bumped whenever a breaking change is made to the WebSocket protocol. The client sends its version on `login`; the server rejects connections below the minimum.

### `GameServer.h` vs `GameServer.cpp`

Most of `GameServer`'s logic lives in the **header** (`GameServer.h`), not in `GameServer.cpp`. `GameServer.cpp` only contains a handful of methods. When looking for server-side game logic, read `GameServer.h` first.

### Card point values by mode

Points differ depending on the game mode:

| Card | Normal (atout) | Normal (non-atout) | Tout Atout | Sans Atout |
|---|---|---|---|---|
| Valet | 20 | 2 | 14 | 2 |
| Neuf | 0 | 0 | 9 | 0 |
| As | 11 | 11 | 6 | 19 |
| Dix | 10 | 10 | 4 | 10 |
| Roi | 4 | 4 | 3 | 4 |
| Dame | 3 | 3 | 2 | 3 |
| Huit/Sept | 0 | 0 | 0 | 0 |

Total points per manche: 162 (standard), Dix de der = 10 pts bonus.

### Communication protocol

Client and server communicate via JSON over WebSocket. Messages have a `type` field. Key client→server types: `login`, `bid`, `playCard`, `coinche`, `surcoinche`, `prendreOrPasser` (Belote), `forfeit`. Key server→client types: `gameStarted`, `updateGameState`, `gameOver`, `matchmakingStatus`, `lobbyUpdate`.
