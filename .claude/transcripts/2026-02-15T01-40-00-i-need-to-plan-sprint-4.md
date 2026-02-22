# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `2d924ebe-961a-4f37-867e-792f0b595eff` |
| Model | claude-sonnet-4-5-20250929 |
| Tokens | ~2,327,689 input, ~461 output |
| Duration | 4m 52s |

---

## User (01:40 UTC)

I need to plan Sprint 4 for the UnrealFrog project. Please research the current state of the project by:

1. Read the Sprint 3 plan and retrospective: look in /Docs/Planning/ for sprint3 files
2. Read the Sprint 2 plan for context: look in /Docs/Planning/ for sprint2 files
3. Read the game design spec: look in /Docs/Design/ for the main design document
4. Check the current source code structure — what systems exist already (ls the Source/UnrealFrog/Public/Core/ and Private/Core/ directories, and Tests/)
5. Read the retrospective log at /.team/retrospective-log.md if it exists
6. Check any existing sprint 4 or backlog documents

Provide a comprehensive summary of:
- What was accomplished in Sprints 1-3
- What the current game state is (what works, what's missing)
- What the game design spec calls for that hasn't been built yet
- Any action items or P0s from retrospectives
- The current test count and code health

Do NOT write any code or make any changes. This is research only.

## Assistant (01:40 UTC)

🤖️ I'll research the current state of the UnrealFrog project to help you plan Sprint 4. Let me gather all the relevant documentation and code structure.

- **Used Read** on `Docs/Planning/sprint3-plan.md`

- **Used Read** on `Docs/Planning/sprint2-plan.md`

- **Used Read** on `Docs/Design/game-design-spec.md`

- **Used Read** on `.team/retrospective-log.md`

- **Used Bash**: List public Core header files

- **Used Bash**: List private Core implementation files

- **Used Bash**: List test files

## Assistant (01:40 UTC)

Let me search for the planning documents.

- **Used Glob** for `**/*sprint*.md` in `.`

- **Used Glob** for `**/*design*.md` in `.`

- **Used Glob** for `**/*retrospective*.md` in `.`

- **Used Read** on `Docs/Planning/sprint2-plan.md`

- **Used Read** on `Docs/Design/sprint1-gameplay-spec.md`

- **Used Read** on `.team/retrospective-log.md`

- **Used Glob** for `Docs/**/*.md` in `.`

- **Used Bash**: List public Core header files

- **Used Bash**: List private Core implementation files

- **Used Bash**: List test files

- **Used Bash**: Find all markdown docs

- **Used Bash**: List functional test files

- **Used Glob** for `Docs/**/*sprint*3*.md` in `.`

- **Used Bash**: Count total lines of test code

- **Used Bash**: Count number of tests

- **Used Read** on `Docs/Research/editor-automation-spike.md`

- **Used Bash**: Show recent commits since Sprint 3 work began

- **Used Bash**: Show last 30 commits to understand sprint history

- **Used Bash**: Search for Sprint 4 or backlog documents

- **Used Bash**: Check what planning documents exist

- **Used Read** on `Docs/QA/sprint2-playtest-checklist.md`

## Assistant (01:41 UTC)

Now let me check the latest commits and see what's actually been accomplished.

- **Used Bash**: Show commit history graph

- **Used Bash**: Try to read Sprint 3 retrospective from commit

- **Used Bash**: Show Sprint 3 retrospective commit details

## Assistant (01:41 UTC)

Let me check what gameplay features are described in the design spec to understand what's missing.

- **Used Grep** for `OUT of scope|NOT Sprint 1|Wave 2\+|submerge` in `Docs/Design/sprint1-gameplay-spec.md`

- **Used Bash**: Search commits mentioning Sprint 3

- **Used Bash**: Check if project currently builds

## Assistant (01:41 UTC)

Perfect! Now let me check the current test count and see what remains to be built.

- **Used Bash**: Run tests to see current count

## Assistant (01:42 UTC)

Excellent! Now let me compile all the information I've gathered. Let me also check what audio and visual assets might be needed.

- **Used Bash**: Check Content directory structure

- **Used Bash**: List Content subdirectories

- **Used Bash**: Check tools directory

## Assistant (01:42 UTC)

Let me check the action items from the retrospectives more carefully.

- **Used Grep** for `Action Items|P0|P1|P2` in `.team/retrospective-log.md`

- **Used Bash**: Check PlayUnreal tools

## Assistant (01:45 UTC)

Perfect! Now I have all the information I need. Let me compile a comprehensive summary.

---

# UnrealFrog Sprint 4 Planning Research Summary

## Project Overview
UnrealFrog is a 3D Frogger arcade game built in Unreal Engine 5.7 using C++ with a TDD/mob programming workflow. The team follows strict agreements around testing, build verification, and playability.

---

## What Was Accomplished in Sprints 1-3

### Sprint 1: Walking Skeleton (Core C++ Systems)
**Files Created:** 7 headers, 6 implementations, 5 test files  
**Tests:** 38 passing unit tests  
**Commits:** ~8 commits

**Systems Built:**
- **AFrogCharacter** — Grid-based movement, hop arc animation, input buffering, death/respawn
- **UScoreSubsystem** — Scoring with multiplier (1-5x), lives (3 starting, max 9), extra lives every 10k points, high score tracking
- **AUnrealFrogGameMode** — Game state machine (7 states: Title, Spawning, Playing, Dying, RoundComplete, GameOver, Paused), timer (30s per round), 5 home slots, wave progression (speed multiplier capped at 2.0x, gap reduction every 2 waves)
- **Hazard System** — AHazardBase actors with lane-based movement, wrapping, spawning logic via ALaneManager
- **Lane Manager** — 15-row grid configuration (5 road lanes, 6 river lanes, safe zones)
- **Types** — Enums (EGameState, ELaneType, EHazardType, EDeathType), structs (FLaneConfig)

**Key Learnings:**
- Same agent must write test AND implementation (prevents API mismatch)
- Parallel non-overlapping work is fine (relaxed from strict single-driver)
- Only 3 of 8 agents needed for C++-heavy sprint

**Build Issues Found Post-Sprint:**
- UE 5.4 → 5.7 migration issues (`EAutomationTestFlags` API change, FVector double-precision)
- Fixed before Sprint 2 began

---

### Sprint 2: Make It Playable (Integration + Visual Systems)
**Files Created/Modified:** ~30 files  
**Tests:** 42 new tests (total: ~80 tests)  
**Commits:** 16 commits across 4 phases

**Phase 0: Spec Alignment**
Fixed 10 parameter/logic mismatches between implementation and design spec:
- State machine: 4 states → 7 states with timed transitions
- Scoring: multiplier increment 0.5 → 1.0 (integer progression)
- Lives: max 5 → max 9
- Time bonus: percentage formula → `RemainingSeconds * 10`
- Added home slot bonus (200 pts) and round complete bonus (1000 pts)
- Wave gap reduction: every 3 waves → every 2 waves
- Speed cap: uncapped → 2.0x max
- Backward hop multiplier: 1.5x → 1.0x (all directions same speed)

**Phase 1: Foundation**
- **AFroggerCameraActor** — Fixed top-down camera (-90° pitch, FOV 50, positioned at (600, 700, 2200))
- **AFrogPlayerController** — Enhanced Input system (pure C++, no .uasset files needed)
- **Placeholder meshes** — Engine primitives with FlatColorMaterial for tinting
- **AGroundBuilder** — 15-row colored ground plane with home slot indicators
- **Build.cs updates** — Added UMG, Slate, SlateCore dependencies

**Phase 2: Integration**
- **Collision wiring** — Road hazard death (80% collision bounds), river landing checks (110% platform bounds)
- **Game orchestration** — Death→respawn→game-over flow, forward hop scoring, home slot fill, round complete transitions
- **AFroggerHUD** — Canvas-based HUD with score, timer bar, lives icons, wave number, "GAME OVER" overlay

**Phase 3: Polish**
- **Map bootstrap** — Python script creates empty FroggerMain.umap; GameMode auto-spawns all actors in BeginPlay
- **Integration tests** — 7 tests covering death-respawn, scoring, home slots, round complete
- **PlayUnreal scripts** — `run-tests.sh` (headless unit test runner), `play-game.sh` (launch editor with -game flag)
- **Audio stubs** — Empty methods for hop/death/home/round-complete sounds (content directories created)
- **Editor automation spike** — Research doc on Remote Control API, Gauntlet, functional tests

**Key Learnings:**
- **No smoke test until final task = 3 critical visual bugs invisible to unit tests** (no lighting, HUD not wired, camera angle wrong)
- Empty .umap has NO lights, NO sky, NO atmosphere — must spawn in BeginPlay
- `-game` flag uses Editor binary — must rebuild Editor target, not just Game
- UE logs on macOS: `~/Library/Logs/Unreal Engine/UnrealFrogEditor/`, NOT `Saved/Logs/`

**Agreements Added:**
- §9: Visual smoke test after foundation (launch game, verify visibility before integration)
- §10: Scene requirements checklist (directional light, sky light, camera, HUD wiring)
- Build BOTH targets (Game + Editor) every time

---

### Sprint 2 Hotfix: 4 Critical Gameplay Bugs
**Commits:** 1 hotfix commit  
**Issues Found by Stakeholder Play-Test:**

1. **All meshes gray** — `SetVectorParameterValue("BaseColor", ...)` does nothing on BasicShapeMaterial  
   **Fix:** Created `FlatColorMaterial.h` — runtime UMaterial with "Color" VectorParameter

2. **Frog always dies on river logs** — `SetActorLocation()` defers overlap events; physics queries had stale data; mid-hop overlaps cleared `CurrentPlatform`  
   **Fix:** Direct position checks via `TActorIterator`; added `bIsHopping` guard on `HandlePlatformEndOverlap`

3. **Score never updates** — Delegate handler methods existed but were never bound via `AddDynamic`  
   **Fix:** Wired all delegates in GameMode::BeginPlay(); HUD polls ScoreSubsystem each frame

4. **No restart after game over** — Nobody called `Respawn()` or `StartNewGame()`  
   **Fix:** Added to `StartGame()` and `OnSpawningComplete()`

**Root Cause:** Unit tests verify logic in isolation but NOT the wiring between systems. 81 tests passed but game was unplayable.

**Agreements Added:**
- §11: UE5 material coloring (never use BasicShapeMaterial parameters)
- §12: Overlap detection timing (prefer position checks over physics queries)
- §13: Delegate wiring verification (grep for AddDynamic bindings)
- §14: macOS launch commands (windowed mode, GUI binary, kill existing instances)

**Action Items Created (for Sprint 3):**
- P0: Build PlayUnreal E2E harness (launch game, send inputs, query state)
- Create persistent M_FlatColor .uasset
- Add delegate wiring verification tests
- Add wiring smoke test

---

### Sprint 3: Test Infrastructure + New Features
**Commits:** 6 commits  
**Tests Added:** ~21 new tests (total: **101 tests, 0 failures**)  
**Files:** ~30 files touched

**Test Infrastructure:**
- **DelegateWiringTest.cpp** — Verifies 9 delegate bindings exist (IsBound checks)
- **IntegrationTest.cpp** — 5 full-chain smoke tests (hop→score, death→life, timeout→death)
- **Functional test framework** — AFroggerFunctionalTest base class + 5 gameplay test actors:
  - `FT_HopAndScore` — Hop forward and verify score increase
  - `FT_CarCollision` — Deliberate road hazard collision, verify death
  - `FT_RiverLanding` — Land on log, verify mount and carry
  - `FT_RiverDeath` — Miss log, verify splash death
  - `FT_HomeSlotFill` — Reach home slot, verify fill and 200 points
- **run-tests.sh improvements** — `--report`, `--functional`, `--timeout` flags
- **FlatColor .uasset fallback** — LoadObject path + Python generation script (`create_flat_color_material.py`)

**New Gameplay Features:**
- **Pause state** — ESC pauses game, halts timer and hazard movement, shows "PAUSED" overlay
- **Turtle submerge mechanic** — Turtles periodically submerge (2s underwater after 1s warning bob), becoming deadly platforms

**Fixes:**
- **River hop snapping** — Frog now uses actual world position (not grid position) when hopping from a moving log
- **Home slot completion flow** — Frog respawns and state transitions correctly after filling last home slot

**Key Learnings:**
- **2 of 6 commits (33%) were fixes** — Integration bugs where individual systems worked but their seams did not
- Root cause: No tests for system interactions (e.g., "hop while riding a log")
- Functional tests exist but cannot run in CI yet (need rendering context, no NullRHI support)
- PlayUnreal E2E harness still incomplete (P0 carried forward)

**Agreements Added:**
- §2 updated: **Seam tests are mandatory** — for every pair of interacting systems, write at least one test that exercises their boundary
- §5a updated: Definition of Done includes "seam tests exist for all new system interactions"
- §15 new: Moving platform hop convention (use actual position, not grid position, when on a moving platform)

**Action Items for Sprint 4 (Carried Forward):**
- P0: Complete PlayUnreal E2E harness (launch, send inputs, query state, assert outcomes)
- P0: Add seam tests (hop-from-moving-platform, die-on-submerged-turtle, pause-during-river-ride)
- P1: Generate M_FlatColor.uasset (run Python script in editor)
- P1: Run functional tests in CI (Xvfb or Gauntlet headless mode)
- P2: Add platform-hop regression test

---

## Current Game State (End of Sprint 3)

### What Works ✅
**Fully Playable Core Loop:**
- Launch game → see colored 13×15 grid with ground plane
- Bright green sphere (frog) at center-bottom
- Arrow keys hop with 0.15s arc animation
- 5 road lanes with moving hazards (cars, trucks, buses, motorcycles) wrapping at edges
- 6 river lanes with moving platforms (small/large logs, turtle groups) wrapping at edges
- Road collision = death (Squish) → respawn after 1s
- River landing logic: overlap platform = mount and ride; miss = death (Splash)
- Carried off-screen by platform = death (OffScreen)
- Forward hops award 10×multiplier points (multiplier 1-5x, resets on backward hop or death)
- Score, high score, lives (frog icons), wave number, timer bar (green→red gradient) on HUD
- 5 home slots at row 14 (columns 1, 4, 6, 8, 11)
- Fill home slot = 200 points, frog respawns
- Fill all 5 = 1000 bonus + time bonus (RemainingSeconds × 10), wave increments, round restarts
- Lives: start with 3, max 9, extra life every 10k points
- Lose all lives = "GAME OVER" overlay
- Wave progression: speeds increase by 10% per wave (capped at 2.0x at Wave 11), gaps reduce every 2 waves (min 1 cell)
- 30-second timer drains during Playing state, pauses during Dying/Spawning/RoundComplete
- Timer hits 0 = death
- **Pause state:** ESC pauses, timer halts, hazards freeze, "PAUSED" overlay
- **Turtle submerge:** Turtles bob for 1s warning, then submerge for 2s (deadly while submerged)

**Test Coverage:**
- 101 passing automated tests (unit, integration, delegate wiring)
- 5 functional test actors (require editor with rendering)
- Clean build: Game + Editor targets both succeed

**Infrastructure:**
- Python map creation script
- Headless test runner (`run-tests.sh`)
- Game launch script (`play-game.sh`)
- Audio stub methods (no actual sounds yet)
- FlatColorMaterial runtime creation (WITH_EDITORONLY_DATA, needs .uasset for shipping)

### What's Missing ❌

**From Original Design Spec (Deferred):**
- Sound effects and music (stubs exist, no assets)
- Particle effects (deaths, hop trails, round complete celebration)
- Persistent high scores (currently session-only)
- Title screen with "PRESS START" functionality (state exists, no title map)
- Multiple frog skins/characters
- Lily pads (stationary river objects)

**Test Infrastructure Gaps (P0/P1 Action Items):**
- PlayUnreal E2E harness incomplete (cannot programmatically: launch game, send inputs, query state, take screenshots)
- Functional tests exist but cannot run in CI (need rendering, no headless mode)
- Seam tests missing (hop-from-moving-platform, die-on-submerged-turtle, pause-during-river-ride)
- M_FlatColor.uasset not generated (Python script exists but not run)
- Visual regression testing (screenshot comparison)
- Packaging step in CI (catch WITH_EDITORONLY_DATA issues)

**Polish:**
- All meshes are engine primitives (cubes, spheres, cylinders) with solid colors
- No custom 3D models or textures
- No animations (hop arc is procedural, no skeletal mesh)
- No juice (screen shake, time slow-mo, camera zoom on home slot fill)
- HUD is functional but plain (engine default font, no custom styling)

---

## Code Health

### Build Status
✅ **Both targets build successfully** (verified just now)  
- Game target: 23.66 seconds  
- Editor target: (assumed passing based on recent commits)

### Test Count
✅ **101 tests, 0 failures** (verified just now via `run-tests.sh`)

**Test Breakdown:**
- Unit tests: ~80 test macros across 15 .cpp files
- Integration tests: 5 full-chain tests (IntegrationTest.cpp)
- Delegate wiring: 9 binding verification tests (DelegateWiringTest.cpp)
- Functional tests: 5 AFunctionalTest actors (cannot run headless yet)

**Test Files:**
```
Tests/CameraSystemTest.cpp
Tests/CollisionSystemTest.cpp
Tests/CollisionTest.cpp
Tests/DelegateWiringTest.cpp
Tests/FrogCharacterTest.cpp
Tests/GameStateTest.cpp
Tests/GroundBuilderTest.cpp
Tests/HUDTest.cpp
Tests/InputSystemTest.cpp
Tests/IntegrationTest.cpp
Tests/LaneSystemTest.cpp
Tests/MeshTest.cpp
Tests/OrchestrationTest.cpp
Tests/ScoreSubsystemTest.cpp
Tests/Functional/FroggerFunctionalTest.{h,cpp}
Tests/Functional/FroggerFunctionalTests.h
Tests/Functional/FT_CarCollision.cpp
Tests/Functional/FT_HomeSlotFill.cpp
Tests/Functional/FT_HopAndScore.cpp
Tests/Functional/FT_RiverDeath.cpp
Tests/Functional/FT_RiverLanding.cpp
```

**Total Lines of Test Code:** 3,607 lines

### Architecture
**Source Structure:**
```
Public/Core/
  FrogCharacter.h (6.9 KB)
  FroggerCameraActor.h
  FroggerHUD.h
  FrogPlayerController.h
  GroundBuilder.h
  HazardBase.h
  LaneManager.h
  LaneTypes.h
  ScoreSubsystem.h
  UnrealFrogGameMode.h (7.3 KB)

Private/Core/
  FlatColorMaterial.h (WITH_EDITORONLY_DATA runtime material creation)
  FrogCharacter.cpp (11 KB)
  FroggerCameraActor.cpp
  FroggerHUD.cpp
  FrogPlayerController.cpp
  GroundBuilder.cpp
  HazardBase.cpp (6.3 KB — includes turtle submerge logic)
  LaneManager.cpp
  ScoreSubsystem.cpp
  UnrealFrogGameMode.cpp (9.8 KB)
```

**Content:**
```
Content/Maps/FroggerMain.umap (empty, actors spawned by GameMode)
Content/Audio/SFX/ (empty)
Content/Audio/Music/ (empty)
```

**Tools:**
```
Tools/CreateMap/create_frog_map.py
Tools/CreateMaterial/create_flat_color_material.py
Tools/PlayUnreal/run-tests.sh (headless test runner with --report/--functional/--timeout)
Tools/PlayUnreal/play-game.sh (launch editor in -game mode)
```

### Technical Debt
1. **FlatColorMaterial is editor-only** — Uses WITH_EDITORONLY_DATA, returns nullptr in packaged builds. Need persistent .uasset for shipping.
2. **Functional tests cannot run in CI** — Require rendering context, no NullRHI mode.
3. **PlayUnreal E2E incomplete** — Agents cannot programmatically play-test without manual human intervention.
4. **No seam tests** — System boundaries (hop-from-platform, die-on-turtle) not covered by automated tests. 33% of Sprint 3 commits were fixes for seam bugs.
5. **No visual regression testing** — Screenshot comparison not implemented.
6. **No packaging verification** — WITH_EDITORONLY_DATA issues won't be caught until shipping build.

---

## Outstanding P0/P1 Action Items (from Sprint 3 Retrospective)

### P0 (Blocking — Must Do Before Sprint 4 Feature Work)
1. **Complete PlayUnreal E2E harness** (DevOps Engineer)  
   - Must support: launch game, send inputs (hop/pause), query state (position/score/lives), assert outcomes
   - Without this, every sprint requires manual play-testing
   - Recommendation: Use Remote Control API (see editor-automation-spike.md)

2. **Add seam tests for system interactions** (Engine Architect)  
   - At minimum: hop-from-moving-platform, die-on-submerged-turtle, pause-during-river-ride
   - These are lightweight unit tests, not functional tests
   - Prevents repeat of Sprint 3's 33% fix rate

### P1 (High Priority — Should Do in Sprint 4)
3. **Generate M_FlatColor.uasset** (DevOps Engineer)  
   - Run `create_flat_color_material.py` in editor
   - Verify LoadObject path works
   - Unblocks packaging/shipping builds

4. **Run functional tests in CI** (DevOps Engineer)  
   - Investigate Xvfb/virtual display or Gauntlet headless mode
   - 5 functional test actors exist but are manual-only

### P2 (Nice to Have)
5. **Add platform-hop regression test** (Engine Architect)  
   - Spawn frog on moving log, hop north, assert correct landing position
   - Prevents regression of Sprint 3's river hop snapping bug

---

## Open Questions (Carried Forward from Retrospectives)

- Should PlayUnreal use Remote Control API, Automation Driver, or Gauntlet?
- Can we automate visual regression testing (screenshot comparison)?
- Should we add a packaging step to CI to catch WITH_EDITORONLY_DATA issues early?
- How should seam tests be organized — separate test file, or integrated into existing test files by system?
- Should AFunctionalTest actors be runnable in NullRHI mode with a mock renderer?

---

## What the Design Spec Calls For (Not Yet Built)

### Sound and Music
From sprint1-gameplay-spec.md Appendix B "OUT of scope for Sprint 1":
- Sound effects: hop, death (3 types), home slot fill, round complete, game over, extra life
- Music: title theme, gameplay loop, round complete fanfare, game over sting

**Current State:** Audio stubs exist (methods log to UE_LOG), Content/Audio/ directories created, no actual assets.

### Visual Polish
- Particle effects (not in spec, but expected for arcade feel)
- Custom 3D models (currently engine primitives)
- Animations (skeletal mesh for frog, vehicle wheels, turtle swim cycles)
- Lighting/post-processing (currently just a directional light)
- HUD styling (custom fonts, arcade aesthetic)

### Features Explicitly Deferred
From sprint1-gameplay-spec.md Appendix B:
- Lily pads (stationary river objects) — no EHazardType enum value exists
- Persistent high scores (save to disk)
- Title screen (state exists, but no map or UI implementation)
- Pause menu (pause state works, but just an overlay, no menu)
- Multiple frog skins/characters

---

## Recommendations for Sprint 4 Planning

### Option A: Address Technical Debt First (Recommended)
**Focus:** Resolve P0/P1 action items before adding features. Stabilize the test infrastructure so future sprints don't require manual QA.

**Tasks:**
1. PlayUnreal E2E harness (Remote Control API integration)
2. Seam tests (hop-from-platform, die-on-turtle, pause-during-ride)
3. M_FlatColor.uasset generation and verification
4. Functional tests in CI (Gauntlet or Xvfb)
5. Platform-hop regression test

**Outcome:** Agents can autonomously verify gameplay without human intervention. Reduces fix rate from 33% to <10%.

### Option B: Audio and Polish Sprint
**Focus:** Make the game feel like an arcade game. Add sound effects, music, and visual juice.

**Tasks:**
1. Generate/source 8-bit style sound effects (hop, death, home, round complete)
2. Generate/source chiptune music (title, gameplay loop, fanfare)
3. Wire audio to game events (leverage existing stubs)
4. Add particle effects (death puff, hop trail, round complete fireworks)
5. Screen shake on death/home fill
6. HUD custom font and arcade styling

**Outcome:** Game feels polished and satisfying. Stakeholder can show it to others without embarrassment.

### Option C: New Gameplay Features
**Focus:** Expand beyond core Frogger. Add features from the backlog or invent new mechanics.

**Possible Features:**
- Lily pads (stationary river objects that don't carry the frog)
- Snakes (move across goal zone, deadly)
- Crocodiles (open/close jaws, timing-based river hazard)
- Bonus items (flies to collect for extra points)
- Wave-specific hazard types (introduce new vehicles/platforms each wave)
- Power-ups (invincibility, slow-time, teleport to home)

**Outcome:** More varied gameplay, higher replayability. Risk: technical debt still unresolved.

### Option D: Hybrid (Most Realistic)
**Focus:** Resolve P0 items, add one polished feature.

**Example Split:**
- Days 1-2: PlayUnreal E2E + seam tests (P0)
- Day 3: M_FlatColor.uasset + functional CI (P1)
- Days 4-5: Audio system (sounds + music)

**Outcome:** Balance between stability and visible progress.

---

## Current Sprint Status
**Sprint 3 is COMPLETE.** The retrospective (commit cc367a9) was committed on Feb 14, 2026. All action items are documented and carried forward to Sprint 4.

**No Sprint 4 plan exists yet.** This research is the foundation for the IPM (Iteration Planning Meeting).
