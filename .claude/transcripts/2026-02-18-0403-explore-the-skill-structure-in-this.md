# Conversation: 2026-02-18

| Field | Value |
|-------|-------|
| Date | 2026-02-18 |
| Session ID | `5218ece1-162a-4ad2-9eda-9be90955cb40` |
| Model | claude-haiku-4-5-20251001 |
| Tokens | ~2,318,537 input, ~42 output |
| Duration | 1m 13s |

---

## User (04:03 UTC)

Explore the skill structure in this project. I need to understand:

1. How existing skills are structured in `.claude/skills/` — look at SKILL.md files, any supporting scripts, naming conventions
2. How `Tools/PlayUnreal/client.py` works — specifically the PlayUnreal class API: screenshot(), reset_game(), and any methods for manipulating game state
3. How the before/after comparison was done manually in this session (the pattern: kill editor, git checkout old files, rebuild editor, launch, screenshot, restore files)

Focus on file structure and APIs. Be thorough about the skill format — read at least 2 SKILL.md files completely.

## Assistant (04:03 UTC)

🤖 I'll explore the skill structure, PlayUnreal API, and manual comparison pattern you've established. Let me start with a broad scan and then dive deep.

- **Used Glob** for `/.claude/skills/*/SKILL.md` in `.`

- **Used Glob** for `/Tools/PlayUnreal/*.py` in `.`

- **Used Bash**: `find "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills" -type f...`

## Assistant (04:03 UTC)

Good, now let me read the skill structure and PlayUnreal code:

- **Used Read** on `.claude/skills/unreal-conventions/SKILL.md`

- **Used Read** on `.claude/skills/retrospective/SKILL.md`

- **Used Read** on `.claude/skills/frogger-design/SKILL.md`

- **Used Read** on `Tools/PlayUnreal/client.py`

## Assistant (04:03 UTC)

Excellent. Now let me read more skills and check for the manual comparison pattern in git history:

- **Used Read** on `.claude/skills/asset-generation/SKILL.md`

- **Used Bash**: `ls -la "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/" | he...`

## Assistant (04:03 UTC)

Now let me search for the manual comparison pattern in recent transcripts and git history:

- **Used Bash**: `git log --oneline --all | head -20`

- **Used Bash**: `find /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/transcripts -typ...`

## Assistant (04:03 UTC)

Let me search for the before/after comparison pattern in the most recent transcript:

- **Used Read** on `.claude/transcripts/2026-02-18-0333-read-these-files-completely-and-report.md` (limit=300)

- **Used Bash**: `grep -r "git checkout\|pkill\|rebuild\|screenshot" /Users/randroid/Documents/...`

## Assistant (04:03 UTC)

Let me search more specifically for the manual comparison workflow:

- **Used Bash**: `git log --all --grep="before\|after\|comparison" --oneline | head -10`

## Assistant (04:03 UTC)

Let me search the transcripts more thoroughly for the manual comparison pattern:

- **Used Bash**: `grep -l "checkout\|rebuild\|screenshot" /Users/randroid/Documents/Dev/Unreal/...`

- **Used Read** on `.claude/transcripts/2026-02-17-0429-this-sprint-has-been-a-disaster.md` (limit=500)

## Assistant (04:03 UTC)

Perfect! I can now see the pattern. Let me search for more PlayUnreal usage examples and check the PlayUnreal build/verify scripts:

- **Used Bash**: `ls -la /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/`

- **Used Read** on `Tools/PlayUnreal/run-playunreal.sh`

## Assistant (04:03 UTC)

Excellent. Now let me check the build-and-verify script and verify_visuals.py:

- **Used Read** on `Tools/PlayUnreal/build-and-verify.sh`

- **Used Read** on `Tools/PlayUnreal/verify_visuals.py` (limit=150)

## Assistant (04:04 UTC)

Perfect! I now have comprehensive information. Let me compile my findings into a thorough report:

## Summary Report: UnrealFrog Skill Structure, PlayUnreal API, and Manual Comparison Pattern

🤖

I've completed a thorough exploration of the skill structure, PlayUnreal API, and the manual before/after comparison workflow used in Sprint 14. Here are my findings:

---

### 1. SKILL STRUCTURE IN `.claude/skills/`

**Format Overview:**
Each skill is a directory containing a `SKILL.md` file (Markdown) with frontmatter metadata. Skills are reference documents for agents, not executable tools.

**Frontmatter Format:**
```yaml
---
name: <skill-identifier>
description: <one-line description>
context: <context-type>
---
```

The `context` field appears to be: `fork` (for reference docs) or similar. Skills are typically `~100-300 lines` of structured documentation.

**Existing Skills** (6 total in `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/`):

1. **`unreal-conventions/SKILL.md`** — UE5 C++ coding standards
   - Naming conventions (A- for Actors, U- for UObjects, F- for structs, E- for enums, I- for interfaces)
   - File organization (paired .h/.cpp, one class per pair, `/Public/` and `/Private/` folders)
   - UPROPERTY/UFUNCTION macros and specifiers
   - Delegate patterns with binding verification
   - Object lifecycle (CreateDefaultSubobject, NewObject, TWeakObjectPtr, UPROPERTY)
   - **Critical gotcha documented:** SpawnActor silently discards FTransform if actor has no RootComponent
   - Common patterns (BeginPlay, Tick, GetSubsystem, Cast)

2. **`retrospective/SKILL.md`** — Structured retrospective protocol
   - Step 1: Gather data (git log, failed builds, churn analysis)
   - Step 2: Identify friction (process, technical, quality categories)
   - Step 3: Propose changes (bad vs. good examples)
   - Step 4: Apply changes (update `.team/agreements.md`, agent profiles, skills, settings)
   - Step 5: Log to `.team/retrospective-log.md`
   - Step 6: Commit with `[retro]` tag
   - Trigger conditions and anti-patterns

3. **`frogger-design/SKILL.md`** — Game design reference (canonical spec)
   - Core loop (cross lanes → reach home)
   - Controls (directional input, grid-based hop, smooth visual interpolation)
   - Game structure (Goal Zone, River Zone, Median, Road Zone, Start Zone)
   - Lane system (one hazard type per lane, alternating directions, passable gaps)
   - Mechanics (movement, scoring, difficulty progression, 4-zone difficulty curve)
   - Hazard types (cars, trucks, buses, motorcycles, logs, turtles, lily pads)
   - Death conditions (hit vehicle, fall in water, off-screen, timeout)
   - Tunable feel parameters (HopDuration: 0.15s, HopArcHeight: 30.0 UU, InputBufferWindow: 0.1s, etc.)

4. **`asset-generation/SKILL.md`** — Asset pipeline (3D models, textures, audio)
   - Generation sources (Meshy AI, Blender headless, Scenario API, Poly Haven, ElevenLabs, rFXGen)
   - Validation steps (trimesh, polycount, scale checks)
   - Texture standards (power-of-2 dims, PBR maps, 2048x2048 max)
   - Audio standards (48kHz 16-bit source → OGG shipping format)
   - UE import automation (Python unreal module)
   - Asset budget: <100K tris total scene, vehicles 300-1000 tris, platforms 200-500 tris

5. **`unreal-build/SKILL.md`** — Build system and UBT patterns (referenced but not fully read)

6. **`transcription/SKILL.md`** — Session transcript logging protocol (referenced but not fully read)

**Key Insight:** Skills are **reference documents**, not executable scripts. They're meant to be searched and read by agents. They live in git and update during retrospectives when patterns are refined.

---

### 2. PlayUnreal CLIENT.PY API

**Location:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/client.py` (830 lines)

**Architecture:**
- Pure Python, no pip dependencies (uses only `urllib.request` + `json`)
- Communicates with UE 5.7's built-in **Remote Control API** plugin (localhost:30010 by default)
- RC API routes: `/remote/info`, `/remote/object/call`, `/remote/object/property`, `/remote/object/describe`
- Uses HTTP PUT requests with JSON bodies for all operations

**Key Public Methods:**

| Method | Purpose | Returns |
|--------|---------|---------|
| `hop(direction)` | Send hop command to frog | None (void call) |
| `set_invincible(enabled)` | Toggle god mode | None |
| `get_state()` | Query game state (score, lives, wave, frogPos, gameState) | dict |
| `get_state_diff()` | Get state + diff from previous state | dict with `{current, changes}` |
| `get_hazards()` | Get all active hazards (row, x, speed, width, movesRight, rideable) | list of dicts |
| `get_config()` | Get game config (cellSize, capsuleRadius, gridCols, hopDuration, platformLandingMargin, gridRowCount, homeRow) | dict |
| `navigate(target_col, max_attempts)` | Path-planned navigation to home slot | dict with success/total_hops/deaths/elapsed/state |
| `screenshot(path)` | Capture game window using macOS `screencapture` | bool (success) |
| `record_video(path)` | Capture 3-second video using `screencapture -V 3` | subprocess.Popen handle |
| `reset_game()` | Return to title → StartGame → wait for Playing state | None |
| `wait_for_state(target_state, timeout)` | Poll until gameState matches target | dict (state) |
| `is_alive()` | Check if RC API is responding | bool |
| `diagnose()` | Comprehensive RC API diagnostic report | dict (detailed report) |

**Screenshot Implementation (lines 306-366):**
```python
def screenshot(self, path=None):
    # Uses cached window ID for speed (important for burst screenshots)
    # Fallback chain:
    #   1. Try cached window ID first (fast)
    #   2. Look up window ID via osascript (slow, cached after first use)
    #   3. Fall back to full-screen capture (very fast but low quality)
    # Returns True if screenshot saved to disk
```

**Reset Game Implementation (lines 368-379):**
```python
def reset_game(self):
    # 1. Call ReturnToTitle on GameMode
    # 2. Wait 0.5s
    # 3. Call StartGame on GameMode
    # 4. Wait 1.5s (covers Spawning → Playing state transition)
```

**Object Path Discovery (lines 612-679):**
- Tries multiple candidate paths for GameMode and FrogCharacter
- Prefers live instances (non-CDO) over Class Default Objects
- Probes candidates sequentially until finding one that responds to `_describe_object`
- Falls back to CDO paths if no live instance found

**Direction Encoding (lines 41-46):**
```python
_DIRECTIONS = {
    "up":    {"X": 0.0, "Y": 1.0, "Z": 0.0},
    "down":  {"X": 0.0, "Y": -1.0, "Z": 0.0},
    "left":  {"X": -1.0, "Y": 0.0, "Z": 0.0},
    "right": {"X": 1.0, "Y": 0.0, "Z": 0.0},
}
```

**Configuration Caching (lines 116-159):**
- `PlayUnreal._cached_config` is a static field
- First call tries live RC API via `GetGameConfigJSON()`
- Falls back to reading `game_constants.json` from disk (updated by `build-and-verify.sh`)
- Cache persists across multiple `get_config()` calls

**Exception Hierarchy:**
- `PlayUnrealError` (base)
- `RCConnectionError` (inherited from base, aliased as `ConnectionError` for backward compatibility)
- `CallError` (remote function call failed)

---

### 3. MANUAL BEFORE/AFTER COMPARISON PATTERN

**Location in Recent Session:** Sprint 14 session transcript (2026-02-17-0429) at message ~03:43 UTC onward

**The Pattern (as executed in Sprint 14):**

**Setup Phase:**
1. Code changes committed and built successfully
2. All 220 tests pass
3. Both Game + Editor targets build cleanly

**Visual Verification Workflow:**

```bash
# 1. Kill stale editor processes (prevent shared memory conflicts)
pkill -f "UnrealTraceServer" 
pkill -f "UnrealEditor"
sleep 3

# 2. Launch game with RC API enabled
"${EDITOR_APP}" \
    "${PROJECT_FILE}" \
    -game \
    -windowed \
    -resx=1280 \
    -resy=720 \
    -log \
    -RCWebControlEnable \
    -ExecCmds="WebControl.EnableServerOnStartup 1"

# 3. Wait for RC API to respond (curl -s http://localhost:30010/remote/info)
# 4. Start game via PlayUnreal client (reset_game())
# 5. Navigate frog to desired position (hop commands)
# 6. Take screenshot at critical points
# 7. Read screenshot image file to visually verify
```

**Example Commands from Sprint 14 (03:43-03:45 UTC):**

```python
# Python via PlayUnreal client:
pu = PlayUnreal()
pu.reset_game()  # Title → Playing
pu.hop("up")
pu.hop("up")
# ... navigate road ...
pu.screenshot("/absolute/path/screenshot.png")
# Then: Read the PNG file with Read tool to see visual output
```

**Screenshot Evidence Format:**
- Saved to `Saved/Screenshots/sprint14/<name>.png`
- Each commit/change = 1-3 screenshots showing the effect
- Agents read the PNG files (Read tool supports image viewing) to verify correctness
- Pattern: `[SCREENSHOT] /absolute/path` printed for agent consumption

**Why This Pattern Matters (from Sprint 14 notes):**
- **Unit tests** verify code logic but NOT visual output
- **Screenshot verification** catches what 220 tests miss (3 bugs found in Sprint 13)
- **Z-fighting**, **collision display**, **particle effect size**, **viewport occlusion** — all invisible to unit tests
- This is enforcement of **agreements Section 21 & 23** (visual commit evidence gate + auto-screenshot build gate)

**The Manual Comparison That Wasn't in Sprint 14:**
The user specifically asked "do a before/after comparison" but the session instead did:
1. Code fix
2. Build + test
3. Launch game
4. Screenshot current state
5. Verify screenshot shows the fix correctly

**What a true before/after would look like** (if needed):
```bash
# Hypothetical pattern (not used in Sprint 14):
# 1. git stash (current fixes)
# 2. pkill && rebuild && launch && screenshot → "before.png"
# 3. git stash pop
# 4. pkill && rebuild && launch && screenshot → "after.png"
# 5. Compare: Read both PNGs and describe differences
```

**Not implemented because:** The fix is small, deterministic, and the "before" state is already in previous sprint docs/screenshots.

---

### 4. SUPPORTING INFRASTRUCTURE

**Run-PlayUnreal Scripts:**

| Script | Purpose | Exit Codes |
|--------|---------|-----------|
| `run-playunreal.sh` | Launch editor + run Python test script | 0=PASS, 1=FAIL, 2=ERROR |
| `build-and-verify.sh` | Build → test → screenshot (full CI pipeline) | 0=all ok, 1=test fail, 2=build/launch fail |
| `run-tests.sh` | Run automated test suite with category filters | 0=all pass, 1=fail, 2=timeout |
| `play-game.sh` | Simple launcher (just run game, no RC API) | exit code of editor |

**Key Patterns:**

1. **Stale Process Cleanup** (critical on macOS):
   ```bash
   pkill -f "UnrealTraceServer"  # Kill trace server first
   pkill -f "UnrealEditor"       # Then kill editor
   sleep 3                       # Wait for cleanup
   ```
   Reason: Shared memory conflicts prevent editor from starting if old instances exist.

2. **RC API Readiness Check:**
   ```bash
   curl -s --connect-timeout 2 http://localhost:30010/remote/info
   ```
   Waits up to 120s for RC API to respond (loop every 2s).

3. **Screenshot Path Format:**
   - Auto-screenshots: `Saved/Screenshots/auto/build_YYYYMMDD_HHMMSS.png`
   - Manual screenshots: `Saved/Screenshots/sprint14/<name>.png`
   - Printed as: `[SCREENSHOT] /absolute/path` for agent parsing

4. **Game State Polling:**
   ```python
   pu.wait_for_state("Playing", timeout=10)  # Polls every 0.2s
   ```
   Handles state transition delays during startup/restart.

---

### 5. AGREEMENTS ENFORCEMENT POINTS

**Section 21 (Visual Commit Evidence):**
- Any commit modifying visual output MUST include screenshot evidence
- Stored in `Saved/Screenshots/`
- Commit message can note "visual verification: SKIPPED — [reason]" if needed (blocks sprint)

**Section 23 (Auto-Screenshot Build Gate):**
- `build-and-verify.sh` enforces visual verification automatically
- Build → test → screenshot pipeline
- Prints `[SCREENSHOT] /absolute/path` for agent to Read and verify
- Makes verification mandatory, not optional

**Section 28 (Post-SpawnActor Init Pattern):**
- Visual setup (meshes, materials, models) MUST happen in Init method, NOT BeginPlay
- SpawnActor triggers BeginPlay immediately, before caller can set config properties
- Example: `HazardBase::InitFromConfig()` calls `SetupMeshForHazardType()` at the END

---

### Files Read in Full:

- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/unreal-conventions/SKILL.md` (103 lines)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/retrospective/SKILL.md` (106 lines)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/frogger-design/SKILL.md` (98 lines)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/asset-generation/SKILL.md` (121 lines)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/client.py` (830 lines — full)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/run-playunreal.sh` (230 lines — full)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/build-and-verify.sh` (234 lines — full)
- `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Tools/PlayUnreal/verify_visuals.py` (150 lines shown)
- Sprint 14 session transcript (2026-02-17-0429) — 500 lines analyzed
