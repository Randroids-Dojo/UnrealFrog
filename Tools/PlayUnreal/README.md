# PlayUnreal — Programmatic Game Testing for UnrealFrog

PlayUnreal lets agents (and humans) launch the game, send inputs, read game state, take screenshots, and record video — all from Python. It connects to the running game via UE5's Remote Control API over HTTP.

This is the single most important QA tool in the project. Unit tests verify code structure; PlayUnreal verifies what the player actually sees and experiences.

## Quick Start

```bash
# Run visual verification (launches game, runs checks, captures video)
./Tools/PlayUnreal/run-playunreal.sh verify_visuals.py

# Run with game already running
./Tools/PlayUnreal/run-playunreal.sh --no-launch verify_visuals.py

# Run diagnostics (connection, object discovery, API probing)
./Tools/PlayUnreal/run-playunreal.sh diagnose.py

# Run QA sign-off checklist
./Tools/PlayUnreal/run-playunreal.sh qa_checklist.py

# Run full acceptance test (road + river crossing)
./Tools/PlayUnreal/run-playunreal.sh acceptance_test.py
```

## Architecture

```
run-playunreal.sh          Shell launcher — kills stale editors, launches with RC API,
│                          waits for API readiness, runs a Python script, cleans up.
│
├── client.py              Python client library. Zero pip dependencies (uses urllib).
│                          All scripts import from this.
│
├── verify_visuals.py      Visual smoke test — resets game, hops, captures death video.
│                          THE go-to script for checking if the game looks right.
│
├── diagnose.py            Deep diagnostic — probes RC API, discovers object paths,
│                          tests every endpoint. Run this first if anything is broken.
│
├── qa_checklist.py        QA sprint sign-off gate — minimum checks before "QA: verified".
│
├── acceptance_test.py     Full gameplay test — crosses road, river, fills home slot.
│
├── play-game.sh           Simple launcher (no RC API, no Python). For manual play.
│
└── run-tests.sh           Unit test runner (separate from PlayUnreal — runs headless).
```

## The Client Library (`client.py`)

### Connecting

```python
from client import PlayUnreal, PlayUnrealError, RCConnectionError

pu = PlayUnreal()  # localhost:30010, 5s timeout

# Check if game is running
if not pu.is_alive():
    print("Game not running!")
```

The game **must** be launched with Remote Control API enabled. The launcher script (`run-playunreal.sh`) handles this automatically with `-RCWebControlEnable`.

### Core API

#### `hop(direction)`
Send a hop command to the frog. Direction is `"up"`, `"down"`, `"left"`, or `"right"`.

```python
pu.hop("up")
time.sleep(0.4)  # Wait for hop to complete (HopDuration=0.15s + margin)
```

#### `get_state()`
Returns a dict with the full game state:

```python
state = pu.get_state()
# {
#   "score": 10,
#   "lives": 3,
#   "wave": 1,
#   "frogPos": [6, 0],
#   "gameState": "Playing",
#   "timeRemaining": 25.3,
#   "homeSlotsFilledCount": 0
# }
```

Game states: `Title`, `Spawning`, `Playing`, `Paused`, `Dying`, `RoundComplete`, `GameOver`.

#### `screenshot(path)`
Captures the game window to a PNG file using macOS `screencapture`.

```python
pu.screenshot("/path/to/shot.png")
```

- First call does a window ID lookup via AppleScript (~0.5s)
- Subsequent calls use the cached window ID (~0.1s)
- Falls back to full-screen capture if window ID lookup fails

#### `record_video(path)`
Records a 3-second `.mov` video of the game window. Returns a `subprocess.Popen` handle.

```python
# Start recording, then immediately trigger the action
video = pu.record_video("/path/to/video.mov")
pu.hop("up")  # Action happens DURING recording — no delay!

# Wait for the 3-second recording to finish
video.wait(timeout=8)
```

**Critical timing**: Start the recording FIRST, then send the action on the very next line with NO sleep. The 3-second window captures everything.

**macOS limitation**: `screencapture -V` with `-l` (window targeting) has a hard 3-second cap. This is a macOS limitation, not a bug.

#### `reset_game()`
Resets to title screen and starts a new game. Waits for `Playing` state.

```python
pu.reset_game()
state = pu.get_state()
assert state["gameState"] == "Playing"
```

#### `wait_for_state(target, timeout=10)`
Polls `get_state()` until `gameState` matches the target string.

```python
state = pu.wait_for_state("Playing", timeout=10)
```

#### `diagnose()`
Returns a detailed diagnostic report as a dict. Probes every aspect of the RC API connection: routes, object paths, function calls, property reads.

```python
report = pu.diagnose()
print(json.dumps(report, indent=2))
```

### Low-Level API

For advanced use, the client exposes internal methods:

```python
# Call any UFUNCTION on any UObject
gm_path = pu._get_gm_path()
pu._call_function(gm_path, "ReturnToTitle")
pu._call_function(gm_path, "StartGame")

# Read any UPROPERTY
wave = pu._read_property(gm_path, "CurrentWave")

# Discover object paths
frog_path = pu._get_frog_path()
```

## Writing a New Test Script

Create a new `.py` file in `Tools/PlayUnreal/`. Follow this pattern:

```python
"""Brief description of what this script tests.

Usage:
    ./Tools/PlayUnreal/run-playunreal.sh my_test.py
"""

import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from client import PlayUnreal, PlayUnrealError, RCConnectionError

SCREENSHOT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Saved", "Screenshots", "my_test"
)


def main():
    pu = PlayUnreal()

    if not pu.is_alive():
        print("FATAL: Game not running")
        return 2

    os.makedirs(SCREENSHOT_DIR, exist_ok=True)

    # Reset to a known state
    gm_path = pu._get_gm_path()
    pu._call_function(gm_path, "ReturnToTitle")
    time.sleep(0.5)
    pu._call_function(gm_path, "StartGame")
    time.sleep(2.0)

    # Your test logic here
    pu.hop("up")
    time.sleep(0.4)
    state = pu.get_state()
    print(f"State: {state}")

    # Capture evidence
    pu.screenshot(os.path.join(SCREENSHOT_DIR, "result.png"))

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except RCConnectionError as e:
        print(f"CONNECTION ERROR: {e}")
        sys.exit(2)
    except KeyboardInterrupt:
        sys.exit(130)
```

Run it:
```bash
./Tools/PlayUnreal/run-playunreal.sh my_test.py
```

## Capturing Visual Evidence

### Screenshots
For verifying static visual state (HUD text, actor positions, colors):

```python
pu.screenshot(os.path.join(SCREENSHOT_DIR, "01_title_screen.png"))
```

### Video for Transient Effects
For effects that last <1 second (death puff, hop dust, score pops), screenshots are too slow. Use video:

```python
# Record first, act immediately after — no delay between these lines
video = pu.record_video(os.path.join(SCREENSHOT_DIR, "death_capture.mov"))
pu.hop("up")  # Hop into traffic to trigger death

video.wait(timeout=8)
```

### Extracting Frames from Video
Use the Swift frame extractor to pull individual PNG frames at 0.5s intervals:

```bash
swift Saved/Screenshots/verify_visuals/frames/extract.swift \
    Saved/Screenshots/verify_visuals/04_death_video.mov \
    Saved/Screenshots/verify_visuals/frames/
```

Then read the PNGs with Claude Code's Read tool for visual analysis.

## Launcher Script (`run-playunreal.sh`)

The launcher handles the full lifecycle:

1. Kills any stale UnrealEditor/UnrealTraceServer processes
2. Launches the editor in `-game` mode with Remote Control API enabled
3. Polls `localhost:30010/remote/info` until the API responds (up to 120s)
4. Waits 3s extra for game actors to finish spawning
5. Runs the specified Python script
6. Kills the editor on exit (trap handler)

### Flags

```bash
# Default: launch game, run script, shut down
./Tools/PlayUnreal/run-playunreal.sh verify_visuals.py

# Skip launch (game already running from a previous run or manual launch)
./Tools/PlayUnreal/run-playunreal.sh --no-launch verify_visuals.py
```

### Manual Launch (without the script)

If you need the game running for interactive debugging:

```bash
pkill -f UnrealEditor 2>/dev/null; sleep 2

"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" \
    "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject" \
    -game -windowed -resx=1280 -resy=720 -log \
    -RCWebControlEnable \
    -ExecCmds="WebControl.EnableServerOnStartup 1"
```

Then run scripts with `--no-launch`:
```bash
./Tools/PlayUnreal/run-playunreal.sh --no-launch diagnose.py
```

## Existing Test Scripts

### `verify_visuals.py` — Visual Smoke Test
**When to run**: After any change to visual systems (VFX, HUD, materials, camera, lighting, meshes).

Checks:
- RC API connection and live object discovery
- Game reset to Title → StartGame → Playing transition
- Frog movement (left/right hops on safe row 0)
- Timer countdown
- Death capture (3-second video of hopping into traffic)
- Final state responsiveness

Output: Screenshots in `Saved/Screenshots/verify_visuals/` + `04_death_video.mov`.

### `diagnose.py` — Connection Diagnostics
**When to run**: When PlayUnreal can't connect, or objects aren't found.

Probes:
- RC API endpoint availability
- Route enumeration
- GameMode object path discovery (tries 12+ candidate paths)
- FrogCharacter object path discovery
- GetGameStateJSON function call
- RequestHop parameter format validation
- Individual property reads

### `qa_checklist.py` — Sprint QA Gate
**When to run**: Before adding "QA: verified" to a commit message.

Checks:
- Connection
- Game state readability
- Game reset + start
- 3 forward hops (position changes, score increases, survival)
- Post-gameplay responsiveness
- Screenshot capture

### `acceptance_test.py` — Full Gameplay
**When to run**: After major gameplay changes.

Attempts to hop the frog across the road (7 hops), across the river (7 hops), and into a home slot. The river crossing is non-deterministic (depends on log positions), so partial success is acceptable.

## Troubleshooting

### "Remote Control API is not responding"
1. Is the editor running? Check: `pgrep -f UnrealEditor`
2. Was it launched with `-RCWebControlEnable`? Use `run-playunreal.sh`, not `play-game.sh`
3. Is port 30010 in use? Check: `lsof -i :30010`
4. Run `diagnose.py` for detailed probing

### "Default__ in object path" (CDO, not live instance)
The game loaded but actors weren't spawned. The map might not have loaded.
- Verify FroggerMain.umap exists
- Check editor log: `Saved/PlayUnreal/editor_*.log`
- Run `diagnose.py` to see which candidate paths respond

### Screenshots are black or empty
- The game window must be visible (not minimized or behind other windows)
- macOS requires screen recording permission for `screencapture`
- Check System Settings > Privacy & Security > Screen Recording

### Video recording produces no file
- `screencapture -V` with `-l` (window ID) is the only reliable combo on macOS
- Without `-l`, `screencapture -V` silently produces nothing
- Continuous mode (`-v`) can't be terminated programmatically

### Editor crashes on launch
- Always use `-windowed` (fullscreen crashes on macOS with UE 5.7)
- Kill stale processes first: `pkill -f UnrealEditor; pkill -f UnrealTraceServer`
- Rebuild Editor target before launching (not just Game target)

## When to Use PlayUnreal

| Situation | Script | Why |
|-----------|--------|-----|
| Changed VFX, HUD, materials, camera | `verify_visuals.py` | Visual bugs are invisible to unit tests |
| Changed gameplay mechanics, scoring | `qa_checklist.py` | Verifies the game loop works end-to-end |
| Major feature addition | `acceptance_test.py` | Full gameplay path verification |
| Something is broken, don't know what | `diagnose.py` | Deep connection and API diagnostics |
| Sprint sign-off | `qa_checklist.py` | Required gate before "QA: verified" |
| Investigating a specific bug | Write a new script | Custom reproduction steps + evidence capture |
