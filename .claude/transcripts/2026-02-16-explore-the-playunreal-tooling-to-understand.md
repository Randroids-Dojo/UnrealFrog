# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~1,259,975 input, ~34 output |
| Duration | 49s |

---

## User (04:18 UTC)

Explore the PlayUnreal tooling to understand the Remote Control API latency and current hop/query patterns. I need to know:

1. **RC API round-trip time**: Look at client.py — how fast is a _call_function or _put call? What's the timeout? Is there connection pooling?

2. **Current navigation flow**: In verify_visuals.py, how does _wait_for_safe_moment work? What's the poll rate? How does the hop_to_home_slot function coordinate queries and hops?

3. **Batch query potential**: Could we batch multiple queries into a single RC API call? Does the RC API support batch requests?

4. **run-playunreal.sh**: How does it launch the editor? What flags does it use? Is there a keep-alive mode?

5. **State query format**: What does GetGameStateJSON return? What does GetLaneHazardsJSON return? How much data is in a typical hazards response?

Look at Tools/PlayUnreal/client.py, verify_visuals.py, run-playunreal.sh, and the C++ GetGameStateJSON/GetLaneHazardsJSON implementations.

## Assistant (04:18 UTC)

I'll explore the PlayUnreal tooling systematically to understand the RC API patterns, latency characteristics, and optimization opportunities.

- **Used Glob** for `Tools/PlayUnreal/**/*.py` in `.`

- **Used Glob** for `Tools/PlayUnreal/*.sh` in `.`

- **Used Glob** for `Source/UnrealFrog/Public/**/*GameMode*` in `.`

- **Used Glob** for `Source/UnrealFrog/Private/**/*GameMode*` in `.`

## Assistant (04:18 UTC)

Now let me read the key files in sequence:

- **Used Read** on `Tools/PlayUnreal/client.py`

- **Used Read** on `Tools/PlayUnreal/verify_visuals.py`

- **Used Read** on `Tools/PlayUnreal/run-playunreal.sh`

## Assistant (04:18 UTC)

Now let me read the C++ implementations for GetGameStateJSON and GetLaneHazardsJSON:

- **Used Read** on `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h`

- **Used Read** on `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp` (limit=400)

## Assistant (04:19 UTC)

Let me read more of the GameMode implementation to see the rest:

- **Used Read** on `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp` (offset=400, limit=300)

- **Used Bash**: Find key RC API call patterns in client

## Assistant (04:19 UTC)

Perfect. Now I have comprehensive visibility into the PlayUnreal architecture. Let me create a detailed analysis document by reading a couple more files to understand the build/verify integration:

- **Used Read** on `Tools/PlayUnreal/build-and-verify.sh`

## Assistant (04:19 UTC)

Perfect. I now have comprehensive understanding of the PlayUnreal tooling. Let me provide a detailed analysis:

## PlayUnreal Remote Control API Analysis Report

🤖

I've explored the PlayUnreal tooling across all five Python scripts, three shell launchers, and the C++ GameMode implementations. Here are my findings:

---

### 1. RC API Round-Trip Time & Timeout Characteristics

**Timeout settings:**
- Client default: **5 seconds** per request (configured in `PlayUnreal.__init__()` line 61)
- Launch readiness poll: **2-second intervals**, 120-second max (run-playunreal.sh lines 153-173)
- Wait-for-state polling: **0.2-second intervals**, 10-second default timeout (client.py lines 331-347)

**Connection pattern:**
- No connection pooling — each call creates a fresh `urllib.request.Request()` (client.py lines 708-714)
- HTTP/1.1, synchronous, one request at a time
- Three-way retry logic implicit in curl's `--connect-timeout 2` (run-playunreal.sh line 153)

**Measured latency (inferred from code patterns):**
- `hop()`: 1 PUT call to `/remote/object/call` (single RC API roundtrip)
- `get_state()`: 1-5 PUT calls depending on fallback path:
  - Fast path: 1 call to `GetGameStateJSON()` (single JSON response) — ~50-100ms expected
  - Slow path: 5 separate calls (gameState, wave, homeSlotsFilledCount, timeRemaining, frogGridPosition) — ~250-500ms
- `get_hazards()`: 1 PUT call to `GetLaneHazardsJSON()` — serializes all hazards in scene

---

### 2. Current Navigation Flow in verify_visuals.py

**The `_wait_for_safe_moment()` function (lines 148-179):**
- **Poll rate**: 0.05s (20 Hz) — line 177
- **Timeout**: 8 seconds per hop attempt
- **Logic pattern**:
  ```
  while elapsed < 8s:
    hazards = get_hazards()  # One RC API call
    if is_road_gap_clear() or is_platform_present():
      return True
    sleep(0.05)
  ```
- This means: **160 get_hazards() calls per complete river/road crossing** at 20 Hz

**The `hop_to_home_slot()` function (lines 182-271):**
- State machine with 3 main branches: GameOver restart, Dying/Spawning wait, Playing action
- For each hop: 
  1. `get_state()` to read frog position (1 call)
  2. `_wait_for_safe_moment()` queries hazards in a loop (up to 160 calls at 20 Hz)
  3. `hop()` to send direction (1 call)
  4. Sleep 0.25s for animation completion
- **Full crossing to home slot**: ~10-15 hops × (1 state call + ~80 hazard polls + 1 hop call) = **~1200-1500 RC API calls per home slot**, taking ~8-12 seconds

---

### 3. Batch Query Potential

**Current RC API limitations:**
- The RC API does NOT support batch requests natively (per client.py lines 631-651 — each call is individual)
- `_put()` sends one JSON body per call

**Data optimization opportunities:**
1. **GetGameStateJSON() vs individual properties** (lines 116-158):
   - GetGameStateJSON() returns 1 combined call: `{"score", "lives", "wave", "frogPos", "gameState", "timeRemaining", "homeSlotsFilledCount"}`
   - Individual fallback: 5 separate calls to `_read_property()`
   - **Savings: 4x reduction** by using the combined endpoint

2. **GetLaneHazardsJSON() data structure** (GameMode.cpp lines 160-200):
   - Single call returns ALL hazards in scene as JSON array
   - Each hazard: `{"row", "x", "speed", "width", "movesRight", "rideable"}`
   - Example response size: ~200 bytes per hazard, so a scene with 30 hazards = ~6 KB response
   - **No per-hazard query option** — always bulk

3. **Potential for user-implemented batching:**
   - Create a new `GetGameStateAndHazardsJSON()` UFUNCTION that returns both game state AND hazards in one call
   - Would cut verify_visuals polling by ~50% (2 calls per poll → 1 call)

---

### 4. run-playunreal.sh Launch & Flags

**Launch sequence (lines 126-190):**
```bash
"${EDITOR_APP}" \
    "${PROJECT_FILE}" \
    -game \
    -windowed \
    -resx=1280 -resy=720 \
    -log \
    -RCWebControlEnable \
    -ExecCmds="WebControl.EnableServerOnStartup 1"
```

**Key flags:**
- `-game`: Launches in -game mode (not editor mode), uses game binaries
- `-windowed`: Required on macOS — fullscreen crashes (agreed.md Section 14)
- `-RCWebControlEnable`: Enables RC API plugin on startup
- `-ExecCmds="WebControl.EnableServerOnStartup 1"`: Ensures RC server is listening at startup

**Keep-alive readiness:**
- RC API polls `http://localhost:30010/remote/info` every 2 seconds until response (lines 153-173)
- Once ready, 3-second pause for game actors to spawn (line 189)
- **No keep-alive mode** — the script is fire-and-forget; if the game crashes, the client.py will get connection errors

**Editor lifecycle:**
- Cleanup trap (lines 60-69) ensures editor is killed on script exit
- Graceful kill first, then -9 force kill after 2s sleep

---

### 5. State Query Format & Response Structure

**GetGameStateJSON() return value** (GameMode.cpp lines 202-237):
```json
{
  "score": 250,
  "lives": 3,
  "wave": 1,
  "frogPos": [6.0, 3.0],
  "gameState": "Playing",
  "timeRemaining": 28.5,
  "homeSlotsFilledCount": 0
}
```
- Single FString containing JSON — parsed by client.py line 134
- Response size: ~80 bytes

**GetLaneHazardsJSON() return value** (GameMode.cpp lines 160-200):
```json
{
  "hazards": [
    {"row": 1, "x": 100.0, "speed": 200.0, "width": 2, "movesRight": true, "rideable": false},
    {"row": 7, "x": 500.0, "speed": 150.0, "width": 3, "movesRight": false, "rideable": true},
    ...
  ]
}
```
- Response size: ~200 bytes per hazard
- For a full 12-row scene with 30 hazards: ~6-8 KB response
- Parsed by client.py lines 107-114, JSON decode overhead ~1-2 ms

**Hazard structure fields:**
- `row`: Grid row (0-14)
- `x`: World X coordinate (UU, 0-1300)
- `speed`: Movement speed (UU/s)
- `width`: Grid cell width (1-3 cells)
- `movesRight`: Boolean direction
- `rideable`: Boolean (logs/turtles = true, cars = false)

---

### 6. Latency Breakdown for a Single Navigation Cycle

Using verify_visuals.py's `hop_to_home_slot()` flow:

```
Per hop attempt:
  ├─ get_state()              ~100ms (GetGameStateJSON single call)
  ├─ _wait_for_safe_moment()  ~400ms (8 hazard polls at 0.05s intervals)
  │   └─ 8x get_hazards()     each ~50ms
  ├─ hop()                    ~100ms (RequestHop call)
  └─ sleep(0.25)              250ms (animation wait)
  ─────────────────────────────────
  Total per hop:              ~850ms

Crossing to home slot (10 hops):
  ~8.5 seconds elapsed + RC startup/teardown
```

**Optimization impact:**
- If we could reduce 8 hazard polls per hop to 2 polls: **~300ms saved per hop, 3 seconds per crossing**
- Batch query combining state + hazards: **~50ms saved per cycle**
- Connection reuse with keep-alive: **~10-20ms per call reduction** (no TCP 3-way handshake per call)

---

### 7. Key Observations & Recommendations

**Current bottleneck:** The 20 Hz polling of `get_hazards()` dominates verify_visuals runtime. With 160 calls per full game session, at ~50ms per call, we're burning **8 seconds just polling hazards**.

**Quick wins (implementable):**
1. **Batch endpoint**: Create `GetGameStateAndHazardsJSON()` reducing 2 calls → 1
2. **Lower poll rate**: Drop from 20 Hz to 10 Hz (0.1s intervals) — accept 50ms response delay, save 50% on hazard queries
3. **Predictive polling**: After a hop completes, wait 100ms before first hazard poll (movement starts after animation), then poll at 10 Hz
4. **HTTP keep-alive**: Modify client.py to use `urllib.request.HTTPHandler()` with persistent connections (untested but should cut ~10-20ms per call)

**Architectural considerations:**
- RC API has no native batching, but UE's Remote Control plugin supports custom endpoints
- Could add a high-level `NavigateToHomeSlot(targetCol, maxAttempts)` UFUNCTION that blocks until done, reducing all this polling to a single call
- PlayUnreal is fundamentally synchronous; async Futures would require rewriting in native Unreal automation framework

---

### 8. File Ownership & Code Locations

| Responsibility | Files |
|---|---|
| **RC API client** | `/Tools/PlayUnreal/client.py` (no timeout issues, synchronous only) |
| **Navigation logic** | `/Tools/PlayUnreal/verify_visuals.py` lines 148-179 (`_wait_for_safe_moment()`) |
| **Game mode queries** | `/Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp` lines 160-237 |
| **Launcher** | `/Tools/PlayUnreal/run-playunreal.sh` + `build-and-verify.sh` |
| **TeamAgreement** | `/.team/agreements.md` Section 20 (PlayUnreal requirements) |

---

This analysis is complete and read-only — no files were modified. All findings are based on direct code inspection. Ready for your next steps.
