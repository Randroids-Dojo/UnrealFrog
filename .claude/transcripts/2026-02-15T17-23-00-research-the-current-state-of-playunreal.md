# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `9cad10df-a2ee-4650-adec-4a8034993a77` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~1,578,343 input, ~37 output |
| Duration | 57s |

---

## User (17:23 UTC)

Research the current state of PlayUnreal automation tooling in the UnrealFrog project. Specifically:

1. Read Tools/PlayUnreal/ - what scripts exist and what do they do?
2. Read Source/UnrealFrog/Tests/PlayUnrealTest.cpp - what does the "E2E" test actually test? Does it launch a real game and send inputs, or just call C++ methods directly?
3. Check if there's any Python scripting infrastructure - look for .py files in Tools/, any PythonScriptPlugin usage, any Remote Control API setup
4. Check the .uproject for PythonScriptPlugin being enabled
5. Look at Docs/Planning/ for any PlayUnreal design docs or sprint plans mentioning it

Report:
- What PlayUnreal can actually do TODAY
- What it CANNOT do (e.g., send hop inputs to a running game, read game state from outside, take screenshots)
- Whether PythonScriptPlugin is set up and usable
- What infrastructure would be needed for a Playwright-like tool that can script gameplay (hop across road, hop across river, complete a level)

Do NOT edit any files. Just research and report findings.

## Assistant (17:23 UTC)

🤖️ I'll research the current state of PlayUnreal automation in the UnrealFrog project systematically.

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal -type f...`

## Assistant (17:23 UTC)

Now let me read these scripts and the test file:

- **Used Read** on `Tools/PlayUnreal/play-game.sh`

- **Used Read** on `Tools/PlayUnreal/run-tests.sh`

- **Used Read** on `Source/UnrealFrog/Tests/PlayUnrealTest.cpp`

## Assistant (17:23 UTC)

Now let me check for Python infrastructure and PythonScriptPlugin:

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools -name "*.py" 2>/de...`

- **Used Bash**: `grep -r "PythonScriptPlugin" /Users/randroid/Documents/Dev/Unreal/UnrealFrog/...`

- **Used Read** on `UnrealFrog.uproject`

## Assistant (17:23 UTC)

Now let me check the planning docs and look for any Remote Control API setup:

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning -type f 2>...`

- **Used Bash**: `grep -r "RemoteControl\|Automation\|Playwright\|WebSocket\|RPC" /Users/randro...`

## Assistant (17:23 UTC)

Let me check the planning docs and look for any design or architecture notes around PlayUnreal:

- **Used Read** on `Docs/Planning/sprint4-plan.md`

- **Used Read** on `Docs/Planning/sprint7-plan.md`

- **Used Bash**: `grep -i "playunreal\|automation\|remote.*control\|websocket\|input.*simulatio...`

## Assistant (17:23 UTC)

Now let me check one of the Python scripts to understand the existing Python infrastructure:

- **Used Read** on `Tools/AudioGen/generate_sfx.py` (limit=100)

- **Used Bash**: `grep -r "input\|RequestHop\|send.*input\|websocket\|http" /Users/randroid/Doc...`

## Assistant (17:23 UTC)

Perfect. Let me check a few more key files to understand the full picture:

- **Used Bash**: `head -50 /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/Pu...`

- **Used Bash**: `grep -n "RequestHop\|HandleInput" /Users/randroid/Documents/Dev/Unreal/Unreal...`

## Assistant (17:23 UTC)

Now let me verify what the seam matrix currently covers:

- **Used Read** on `Docs/Testing/seam-matrix.md`

## Assistant (17:24 UTC)

Excellent. Now I have a complete picture. Let me provide the comprehensive report.

---

## PlayUnreal Automation Tooling: Current State Research Report

### What Scripts Exist and What They Do

**1. `/Tools/PlayUnreal/play-game.sh`** (52 lines)
- **Purpose**: Launch the UnrealFrog game in PIE (Play-In-Editor) mode for manual human play-testing
- **Modes**:
  - Default (`--play`): Launches editor with `-game -windowed` flags, auto-PIE enabled
  - `--editor`: Launches editor without auto-play
- **Features**:
  - Validates UE 5.7 engine installation and project file exist
  - Sets resolution to 1280x720, windowed mode (fullscreen crashes on macOS per agreement §14)
  - Prints helpful message: "Use arrow keys to move the frog. Escape to pause."
- **Limitation**: This is purely a **human play-testing launcher**. No programmatic control, no input injection, no state queries.

**2. `/Tools/PlayUnreal/run-tests.sh`** (338 lines)
- **Purpose**: Headless test runner using UE's native Automation Framework
- **Capabilities**:
  - Launches `UnrealEditor-Cmd` in NullRHI mode (no rendering) with test filters
  - 5-minute timeout with auto-kill on hang
  - Pre-flight cleanup: kills stale `UnrealEditor-Cmd` and `UnrealTraceServer` processes
  - **Category filters**: `--all`, `--seam`, `--audio`, `--e2e`, `--integration`, `--wiring`, `--vfx`
  - Per-category pass/fail breakdown table
  - JSON report generation with `-ReportExportPath`
  - Can list available tests (`--list` mode)
- **Test Discovery**: Tests are identified by UE path prefix (e.g., `UnrealFrog.PlayUnreal.*`)
- **Limitations**: 
  - Runs in NullRHI (headless, no rendering)
  - Only supports `--functional` flag to use a rendering RHI (Editor binary required)
  - No remote control capability — tests run in-process only

---

### What PlayUnrealTest.cpp Actually Tests (E2E Reality Check)

**Location**: `/Source/UnrealFrog/Tests/PlayUnrealTest.cpp`

**What it actually does:**
- **Direct C++ method calls only** — NO real game launch, NO actual input sending, NO reading live game state
- Creates 5 `IMPLEMENT_SIMPLE_AUTOMATION_TEST` scenarios:
  1. **Scenario 1: Forward Progress** — Calls `GM->StartGame()`, then `GM->HandleHopCompleted(FIntPoint(6, Row))` directly, verifies score/multiplier via direct variable reads
  2. **Scenario 2: Road Death** — Calls `GM->HandleFrogDied(EDeathType::Squish)`, then `Scoring->LoseLife()` directly
  3. **Scenario 3: Pause/Resume** — Calls `GM->PauseGame()`, then `GM->TickTimer(5.0f)` directly, reads `GM->RemainingTime`
  4. **Scenario 4: Game Over** — Calls `Scoring->LoseLife()` 3x, then reads `Scoring->IsGameOver()`
  5. **Scenario 5: Full Round** — Calls `GM->HandleHopCompleted()` 5 times with hardcoded home slot positions

**Structured logging**: Outputs `[PlayUnreal]` tagged logs for bash script parsing (e.g., `[PlayUnreal] SCORE=150 LIVES=3 WAVE=1 STATE=Playing`).

**Critical limitation**: This is **NOT a Playwright-like tool**. It:
- Does NOT launch a real game instance
- Does NOT send keyboard/controller inputs to a running game
- Does NOT capture screenshots
- Does NOT read live game state from an external process
- Does NOT use any network protocol (no WebSocket, HTTP, Remote Control API)
- Does NOT support scripted sequences like "hop left 3 times, then press Start"

**Result**: All 5 scenarios *pass*, but they're testing C++ method calls in isolation, not actual gameplay. A real Frogger bug (e.g., hop physics broken, input lag, collision miss) would NOT be caught because inputs are never actually sent to the game.

---

### Python Scripting Infrastructure

**PythonScriptPlugin Status**:
- **Enabled**: Yes, in `UnrealFrog.uproject` (line 15-17, `"Name": "PythonScriptPlugin", "Enabled": true`)
- **Available at runtime**: Yes — plugin is in UE 5.7 at `Plugins/Experimental/PythonScriptPlugin`

**Existing Python scripts in Tools/** (all standalone, NOT using PythonScriptPlugin):
1. `/Tools/CreateMap/create_frog_map.py` — Creates empty `.umap` file (standalone bootstrap)
2. `/Tools/CreateMaterial/create_flat_color_material.py` — Creates material asset (presumably editor Python API)
3. `/Tools/AudioGen/generate_sfx.py` — Generates 9 WAV files with numpy (standalone, no UE integration)
4. `/Tools/AudioGen/generate_music.py` — Generates 2 music tracks with numpy (standalone, no UE integration)

**PythonScriptPlugin usage TODAY**: None. The plugin is enabled but not actively used by any test or tool.

**Potential use**: Could call editor Python commands via `-ExecCmds="py_exec( ... )"` in headless mode, but this is NOT currently implemented.

---

### Remote Control API Setup

**Status**: NOT SET UP

**Grep results**: Zero references to `RemoteControl`, `Automation Driver`, `WebSocket`, or HTTP APIs in source code.

**What would be needed**:
- `RemoteControlAPI` plugin enabled (separate from PythonScriptPlugin)
- HTTP server listening on localhost (e.g., port 30010)
- WebSocket endpoint for sending commands like `{"action": "HopUp"}` or `{"query": "GetScore"}`
- No existing infrastructure for this

---

### What PlayUnreal Can Do TODAY

| Capability | Status | Details |
|-----------|--------|---------|
| Run unit tests headless | ✅ YES | `run-tests.sh --all` via UE Automation Framework in NullRHI mode |
| Filter tests by category | ✅ YES | `--seam`, `--audio`, `--e2e`, `--integration`, `--wiring`, `--vfx` |
| Parse test output to bash | ✅ YES | Grep for `Test Completed. Result={Success\|Fail}` in logs |
| Report per-category metrics | ✅ YES | 19-line table printed after run |
| Call C++ methods directly in tests | ✅ YES | `GM->StartGame()`, `Scoring->AddForwardHopScore()`, etc. |
| Verify game state via direct reads | ✅ YES | `TestEqual(GM->HighestRowReached, 5)` |
| Launch PIE for human play-testing | ✅ YES | `play-game.sh` opens editor with `-game` flag |
| Generate JSON test reports | ✅ YES | `-ReportExportPath` flag |

---

### What PlayUnreal CANNOT Do TODAY

| Capability | Status | Why |
|-----------|--------|-----|
| Send hop input to a running game | ❌ NO | No input injection mechanism; no Remote Control API |
| Capture screenshots | ❌ NO | No screenshot API wired; NullRHI mode has no rendering |
| Read game state from outside process | ❌ NO | Tests are in-process only; no external state query |
| Script gameplay sequences | ❌ NO | No replay system, no scenario DSL |
| WebSocket/HTTP remote control | ❌ NO | RemoteControlAPI plugin not enabled |
| Run Playwright-style browser tests | ❌ NO | N/A — this is Unreal, not web |
| Simulate input timing/buffering | ❌ NO | PlayUnrealTest calls methods directly, no timing variance |
| Test UI input responsiveness | ❌ NO | No keyboard/controller input path tested |
| Detect gameplay bugs like collision misses | ❌ NO | Hop position is hardcoded (`FIntPoint(6, 1)`), not computed |

---

### Infrastructure Requirements for a Playwright-Like Tool

To build a tool that can **script gameplay autonomously** (e.g., "hop across road, cross river, reach goal, verify round complete"), you would need:

#### **Core Components**

| Component | Current State | What's Needed |
|-----------|---------------|---------------|
| **Input injection** | ❌ None | Bind a network input handler to `AFrogPlayerController::IA_HopUp/Down/Left/Right` |
| **Remote state query** | ❌ None | Expose read-only game state endpoint: `GetScore()`, `GetGameState()`, `GetFrogPos()` |
| **Network protocol** | ❌ None | HTTP/JSON API or WebSocket for bidirectional comms |
| **Screenshot capture** | ❌ None | Hook `FSceneCapture2D` or RHI framebuffer to save PNG |
| **Timing control** | ⚠️ Partial | `AFrogCharacter` has `HopDuration` parameter; could expose frame-stepping |
| **Session management** | ❌ None | Track active game instance, support multiple concurrent tests |

#### **Architecture Options**

**Option A: Remote Control API (UE native)**
- Enable `RemoteControlAPI` plugin
- Expose C++ functions as HTTP endpoints via UE's built-in web server
- Clients call REST endpoints: `POST /remote/object/call/AFrogCharacter/RequestHop` with JSON args
- **Pros**: Native UE support, officially documented
- **Cons**: Overhead, requires UE binary to be running in networked mode

**Option B: Custom HTTP server in C++**
- Create a new `UPlayUnrealServer` subsystem that listens on localhost:30010
- Implement handlers for:
  - `POST /input/hop` with `{"direction": "up"}` 
  - `GET /state/game` returns `{"score": 150, "state": "Playing", "frogPos": [6, 3]}`
  - `POST /screenshot` saves PNG to disk
- Thread-safe access to AFrogCharacter and AUnrealFrogGameMode
- **Pros**: Full control, lightweight, testable
- **Cons**: Requires new C++ code, threading complexity

**Option C: Python-based automation via UE Python API**
- Enable `PythonScriptPlugin` and create a test runner
- Use PythonScriptPlugin's Unreal editor Python API to:
  - Launch PIE: `unreal.Frogger_Functions.start_game()`
  - Send input: `unreal.Frogger_Functions.request_hop("Up")`
  - Query state: `unreal.Frogger_Functions.get_score()`
  - Take screenshots: `unreal.GameplayStatics.take_screenshot()`
- Wrap in a pytest-style test framework
- **Pros**: Python ecosystem, pytest integration, easy scripting
- **Cons**: Requires editor to be running, editor binary bloat, experimental plugin

#### **Recommended Next Steps (if this becomes a priority)**

1. **Phase 1 (2-3 hours)**: Create minimal HTTP server option (B)
   - New file: `Source/UnrealFrog/Public/Core/PlayUnrealServer.h/cpp`
   - Expose: `/input/hop`, `/state/game` endpoints
   - Wire to `AFrogPlayerController` and `AUnrealFrogGameMode`
   - Add 3 tests verifying endpoints respond

2. **Phase 2 (1-2 hours)**: Python client library
   - New file: `Tools/PlayUnreal/client.py`
   - Helper class: `PlayUnrealClient(host="localhost", port=30010)`
   - Methods: `client.hop("up")`, `client.get_score()`, `client.take_screenshot()`

3. **Phase 3 (2-3 hours)**: Example test scenarios
   - Rewrite existing PlayUnrealTest.cpp scenarios to use the client
   - Verify: "hop 3 times forward, score > 0"
   - Verify: "hop into road, collide with car, state is Dying"

---

### Summary: Current PlayUnreal Status

**Today's PlayUnreal tooling is a hybrid**:
- ✅ **Strong**: 154 automated unit/seam/integration tests run headless via UE Automation Framework
- ✅ **Usable**: `run-tests.sh` is reliable, filters tests by category, reports per-category metrics
- ✅ **Manual**: `play-game.sh` is fine for human play-testing
- ❌ **Weak**: "E2E" tests in PlayUnrealTest.cpp are NOT end-to-end — they're direct C++ method calls, not actual gameplay
- ❌ **Missing**: No ability to send inputs to a running game, no screenshot capture, no external state queries
- ⚠️ **Enabled but unused**: PythonScriptPlugin is in the project but no scripts use it yet

**The gap**: A real Playwright-like tool (`hop left 3x, cross river, reach home, verify round complete with screenshot`) is **not possible with current infrastructure**. To build it would require:
1. Input injection mechanism (HTTP or Python API)
2. State query endpoints
3. Screenshot capture wiring
4. Test orchestration framework (pytest or similar)

**Effort estimate**: 6-8 hours for a minimal "move frog via HTTP API" implementation; 12-15 hours for a full Playwright-like testing framework.
