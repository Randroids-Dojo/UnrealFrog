# Sprint 4 Plan: Seam Tests, PlayUnreal E2E, and Audio

*Drafted by: XP Coach*
*Date: 2026-02-14*
*Status: DRAFT — pending IPM approval*

---

## Sprint Goal

**"Make the game testable by agents, and make it sound like an arcade game."**

Resolve the two P0 action items from Sprint 3 (seam tests and PlayUnreal E2E harness) so agents can autonomously verify gameplay, then add procedurally-generated sound effects to turn our silent Frogger into an arcade experience.

---

## Why This Sprint

Sprint 3 ended with 101 tests passing, but:
- **33% of commits were bug fixes** for seam issues that unit tests missed
- **No agent can play-test** — the stakeholder is still the only one who can verify gameplay
- **The game is silent** — audio stubs log to `UE_LOG` but produce no sound

This sprint stabilizes the test infrastructure (so future sprints don't repeat the 33% fix rate) and adds the most impactful polish layer: sound.

---

## Definition of Done

Sprint 4 is NOT complete until ALL of the following are true:

- [ ] Both build targets succeed (Game + Editor)
- [ ] All automated tests pass (101 existing + new Sprint 4 tests)
- [ ] Seam tests cover all system interaction boundaries
- [ ] PlayUnreal E2E can launch, send inputs, query state, and assert outcomes
- [ ] Sound effects play for: hop, 3 death types, home slot fill, round complete, game over, extra life
- [ ] QA Lead has verified gameplay via PlayUnreal or manual play-testing
- [ ] Visual smoke test passes (launch game, all systems visible + audible)
- [ ] Retrospective completed and agreements updated

---

## Phase 0: Seam Tests (P0 Carry-Forward)

**Driver: Engine Architect** | **Priority: BLOCKING — must complete before Phase 2**

The Sprint 3 retrospective identified a 33% fix rate caused by missing boundary tests. These lightweight unit tests create two interacting systems and verify their handshake.

### Task 1: Seam Test File

Create `Source/UnrealFrog/Tests/SeamTest.cpp` with the following test cases:

| # | Test Name | Systems | Verifies |
|---|-----------|---------|----------|
| 1 | HopFromMovingPlatform | FrogCharacter + HazardBase (log) | Hop origin uses actual world position, not stale grid position. Landing position = (log_x + hop_offset, new_row). |
| 2 | DieOnSubmergedTurtle | FrogCharacter + HazardBase (turtle) | Landing on a submerged turtle triggers Splash death, not mount. |
| 3 | PauseDuringRiverRide | FrogCharacter + HazardBase + GameMode | Pausing while riding a log freezes both frog and log movement. Unpausing resumes correctly. |
| 4 | HopWhilePlatformCarrying | FrogCharacter + HazardBase (log) | Frog can hop off a moving log in any direction. Landing position accounts for platform velocity during hop. |
| 5 | DeathResetsCurrentPlatform | FrogCharacter + HazardBase | Dying while on a platform clears CurrentPlatform. Respawn doesn't inherit old platform state. |
| 6 | TimerFreezesDuringPause | GameMode + FrogPlayerController | Timer value before pause == timer value after unpause. No time leaks. |
| 7 | TurtleSubmergeWhileRiding | FrogCharacter + HazardBase (turtle) | If frog is riding a turtle that submerges, frog dies (Splash). |

**Tests:** 7 new test cases
**Files:** `Source/UnrealFrog/Tests/SeamTest.cpp` (new)

**Organization decision:** Seam tests go in a dedicated file (not mixed into per-system test files) so they're easy to find and cross-reference with the interaction matrix.

---

## Phase 1: PlayUnreal E2E Harness (P0 Carry-Forward)

**Driver: DevOps Engineer** | **Priority: BLOCKING — must complete before Sprint 4 exit**

Agents need to programmatically launch the game, send inputs, query state, and verify outcomes without a human. This is the most impactful infrastructure investment for all future sprints.

### Task 2: PlayUnreal State Query via UE Automation

Extend the existing `run-tests.sh` with a state-reporting automation test that:
- Spawns GameMode + Frog + full game scene
- Queries and logs game state in parseable format: `[PlayUnreal] SCORE=0 LIVES=3 WAVE=1 STATE=Playing FROG_POS=(0,0,0)`
- Reports assertion results

**Approach:** Write a `UPlayUnrealAutomationTest` that runs as a latent automation test (like existing IntegrationTest.cpp) but with structured output that `run-tests.sh` can parse.

**Files:**
- `Source/UnrealFrog/Tests/PlayUnrealTest.cpp` (new — latent automation test)
- `Tools/PlayUnreal/run-tests.sh` (modify — add `--e2e` mode for state parsing)

### Task 3: PlayUnreal Input Simulation

Add simulated input sequences to the PlayUnreal test:
- Simulate hop inputs (up/down/left/right) via `AFrogPlayerController::RequestHop()`
- Simulate pause toggle via `AFrogPlayerController::TogglePause()`
- Verify state changes after each input
- Support scripted test scenarios: "hop 3 times forward, assert score > 0"

**Files:**
- `Source/UnrealFrog/Tests/PlayUnrealTest.cpp` (modify — add input simulation)

### Task 4: PlayUnreal Scenario Tests

Write 5 end-to-end scenarios that exercise the full game loop:

| # | Scenario | Steps | Assert |
|---|----------|-------|--------|
| 1 | Forward progress | Hop up 5 times | Score increases, multiplier capped at 5 |
| 2 | Road death | Hop into road lane, wait for hazard | State → Dying → Spawning, lives decremented |
| 3 | Pause and resume | Hop, pause, verify freeze, unpause, verify resume | Timer unchanged during pause |
| 4 | Game over | Set lives to 1, trigger death | State → GameOver |
| 5 | Full round | Fill all 5 home slots | RoundComplete, wave increments, 1000 bonus |

**Files:**
- `Source/UnrealFrog/Tests/PlayUnrealTest.cpp` (modify — add scenarios)

---

## Phase 2: Audio System

**Depends on Phase 0 completion.** This is the feature work for Sprint 4.

### Task 5: Audio Manager Class

**Driver: Engine Architect** | **Navigator: Sound Engineer**

Create a centralized audio manager that owns all sound cues and plays them on game events.

**Requirements:**
- New class: `UFroggerAudioManager` (UGameInstanceSubsystem)
- Owns all `USoundWave*` references (loaded from Content/Audio/)
- Provides public methods matching existing stubs:
  - `PlayHopSound()` — short, snappy "boing"
  - `PlayDeathSound(EDeathType Type)` — distinct sounds for Squish, Splash, OffScreen
  - `PlayHomeSlotSound()` — satisfying "ding" or chime
  - `PlayRoundCompleteSound()` — brief fanfare (2-3 seconds)
  - `PlayGameOverSound()` — descending tone / sad trombone sting
  - `PlayExtraLifeSound()` — ascending tone / 1-up chime
  - `PlayTimerWarningSound()` — ticking/beeping when timer < 5 seconds
- Sounds play via `UGameplayStatics::PlaySound2D()` (non-positional, UI-style)
- Volume exposed as `UPROPERTY(EditAnywhere)` per category (SFX, Music)
- Audio can be globally muted (for headless testing / CI)

**Files:**
- `Source/UnrealFrog/Public/Core/FroggerAudioManager.h` (new)
- `Source/UnrealFrog/Private/Core/FroggerAudioManager.cpp` (new)

**Tests:**
- AudioManager subsystem initializes without crash
- Each Play method executes without crash when sound assets are missing (graceful fallback)
- Mute flag prevents sound playback

### Task 6: Procedural Sound Generation

**Driver: Sound Engineer** | **Navigator: Art Director**

Generate 8-bit style sound effects using procedural audio generation (Python + numpy/scipy, output as WAV files).

**Sound Assets to Generate:**

| Sound | Style | Duration | Description |
|-------|-------|----------|-------------|
| `SFX_Hop.wav` | 8-bit chirp | 0.1s | Quick ascending pitch sweep (100Hz → 400Hz) |
| `SFX_DeathSquish.wav` | 8-bit crash | 0.3s | Noise burst with descending pitch |
| `SFX_DeathSplash.wav` | 8-bit splash | 0.4s | Filtered noise with reverb-like decay |
| `SFX_DeathOffScreen.wav` | 8-bit alarm | 0.3s | Two-tone descending beep |
| `SFX_HomeSlot.wav` | 8-bit chime | 0.3s | Ascending arpeggio (C-E-G) |
| `SFX_RoundComplete.wav` | 8-bit fanfare | 1.5s | Major scale run + held chord |
| `SFX_GameOver.wav` | 8-bit sad | 1.0s | Descending minor arpeggio |
| `SFX_ExtraLife.wav` | 8-bit 1-up | 0.5s | Classic ascending 5-note pattern |
| `SFX_TimerWarning.wav` | 8-bit tick | 0.2s | Sharp pulse, designed for rapid repetition |

**Output:** 16-bit PCM WAV, 44100Hz, mono

**Files:**
- `Tools/AudioGen/generate_sfx.py` (new — generates all WAV files)
- `Content/Audio/SFX/*.wav` (new — 9 sound files)

### Task 7: Audio Import and Wiring

**Driver: Engine Architect** | **Navigator: Sound Engineer**

Import WAV files as USoundWave assets and wire the AudioManager to game events.

**Requirements:**
- Import WAVs into `Content/Audio/SFX/` as USoundWave assets
  - Option A: Use `RuntimeAudioImporterLibrary` to load WAVs at runtime (no .uasset needed)
  - Option B: Use Python editor scripting to create .uasset wrappers
  - Option C: Load WAVs directly via `FWaveModInfo` (engine API for raw PCM)
- Wire AudioManager to existing game events:
  - `AFrogCharacter::OnHopStarted` → `PlayHopSound()`
  - `AFrogCharacter::OnFrogDied` → `PlayDeathSound(DeathType)`
  - `AUnrealFrogGameMode::OnHomeSlotFilled` → `PlayHomeSlotSound()`
  - `AUnrealFrogGameMode::OnRoundComplete` → `PlayRoundCompleteSound()`
  - `AUnrealFrogGameMode::OnGameOver` → `PlayGameOverSound()`
  - `UScoreSubsystem::OnExtraLife` → `PlayExtraLifeSound()`
  - `AUnrealFrogGameMode::OnTimerWarning` (new) → `PlayTimerWarningSound()`
- Remove existing `UE_LOG` audio stubs from FrogCharacter and GameMode (replaced by AudioManager)

**Files:**
- `FrogCharacter.h/.cpp` (modify — remove audio stubs, add OnHopStarted delegate if missing)
- `UnrealFrogGameMode.h/.cpp` (modify — remove audio stubs, add OnTimerWarning, wire AudioManager)
- `ScoreSubsystem.h/.cpp` (modify — add OnExtraLife delegate if missing)
- `FroggerAudioManager.cpp` (modify — wire to events)

**Tests:**
- Hop triggers PlayHopSound (verify via mock or delegate count)
- Death triggers correct PlayDeathSound variant
- Home slot fill triggers PlayHomeSlotSound
- Wiring verification: grep all AudioManager Play* calls have corresponding delegate bindings

### Task 8: Audio Wiring Tests

**Driver: Engine Architect**

Add audio-specific wiring tests to ensure all delegate bindings exist and fire correctly.

**Files:**
- `Source/UnrealFrog/Tests/AudioTest.cpp` (new)

**Test cases:**
- AudioManager initializes as a subsystem
- Each game event has a corresponding AudioManager binding
- Playing sounds with nullptr assets doesn't crash
- Mute flag suppresses playback
- Timer warning fires when timer < 5 seconds

---

## Phase 3: Infrastructure Cleanup (P1 Items)

**Can parallelize with Phase 2. Lower priority but should be done this sprint.**

### Task 9: Generate M_FlatColor.uasset

**Driver: DevOps Engineer**

Run the existing `Tools/CreateMaterial/create_flat_color_material.py` in the editor to generate a persistent material asset.

**Requirements:**
- Launch editor with Python script
- Verify `Content/Materials/M_FlatColor.uasset` is created
- Verify `LoadObject` path in `FlatColorMaterial.h` finds the asset
- Commit the .uasset

**Files:**
- `Content/Materials/M_FlatColor.uasset` (new, generated)
- `Source/UnrealFrog/Private/Core/FlatColorMaterial.h` (verify — LoadObject path)

### Task 10: run-tests.sh Reliability

**Driver: DevOps Engineer**

Harden the test runner for consistent CI-like usage.

**Requirements:**
- Add `--seam` filter mode (runs only `UnrealFrog.Seam.*` tests)
- Add `--audio` filter mode (runs only `UnrealFrog.Audio.*` tests)
- Add `--all` mode that runs unit + integration + seam + audio (default)
- Report per-category pass/fail counts
- Exit code 0 only if all tests pass

**Files:**
- `Tools/PlayUnreal/run-tests.sh` (modify)

---

## Phase 4: Verification and Play-Test

**Depends on Phases 0-2. Phase 3 can be in progress.**

### Task 11: Visual + Audio Smoke Test

**Driver: QA Lead** | **Navigator: Game Designer**

Launch the game and verify all systems work together.

**Checklist:**
- [ ] Game launches without errors
- [ ] Frog visible as green sphere, hops on arrow keys
- [ ] Hop sound plays on every hop
- [ ] Road hazards visible and moving, collision kills frog
- [ ] Death sound plays (squish variant)
- [ ] River platforms visible and moving, frog rides logs
- [ ] Splash death sound plays when missing a log
- [ ] Home slot fill plays chime sound
- [ ] Round complete plays fanfare
- [ ] Game over plays descending sting
- [ ] Timer warning beeps when timer < 5 seconds
- [ ] Pause works (ESC), all movement and sound stops
- [ ] Turtle submerge works, death on submerged turtle plays splash sound
- [ ] Score, lives, timer HUD all update correctly
- [ ] No crashes or error logs during full play-through

### Task 12: PlayUnreal E2E Validation

**Driver: DevOps Engineer**

Run the full PlayUnreal E2E test suite and verify all 5 scenarios pass.

```bash
Tools/PlayUnreal/run-tests.sh --e2e
```

All scenarios must pass. If any fail, fix the underlying issue before marking Sprint 4 complete.

---

## Driver Assignments Summary

| Agent | Tasks | Role |
|-------|-------|------|
| **Engine Architect** | Tasks 1, 5, 7, 8 | Seam tests, AudioManager, audio wiring |
| **DevOps Engineer** | Tasks 2, 3, 4, 9, 10, 12 | PlayUnreal E2E, M_FlatColor, test runner |
| **Sound Engineer** | Task 6 | Procedural sound generation |
| **QA Lead** | Task 11 | Play-test verification |
| **Art Director** | -- | Navigator on Task 6 (validates audio aesthetic) |
| **Game Designer** | -- | Navigator on Task 11 (validates game feel) |
| **XP Coach** | -- | Coordination, retrospective facilitation |

---

## Dependency Graph

```
Phase 0 (seam tests) ──────────────────────────────┐
  Task 1 (SeamTest.cpp)                             │
                                                    │
Phase 1 (PlayUnreal E2E) ──────── can parallelize ──┤
  Task 2 (state query)                              │
    └── Task 3 (input sim)                          │
          └── Task 4 (scenarios)                    │
                                                    │
Phase 2 (audio) ── depends on Phase 0 ──────────────┤
  Task 5 (AudioManager class)                       │
  Task 6 (sound generation) ── parallel with 5 ──   │
    └── Task 7 (import + wiring) ── after 5 & 6     │
          └── Task 8 (audio tests)                  │
                                                    │
Phase 3 (infra cleanup) ── parallel with Phase 2 ── │
  Task 9 (M_FlatColor.uasset)                       │
  Task 10 (run-tests.sh)                             │
                                                    │
Phase 4 (verification) ── after all above ──────────┘
  Task 11 (smoke test)
  Task 12 (PlayUnreal validation)
```

**Critical path:** Task 1 (seam tests) → Task 5+6 (AudioManager + sound gen) → Task 7 (wiring) → Task 8 (audio tests) → Task 11 (smoke test)

**Parallel track:** Tasks 2-4 (PlayUnreal E2E) run independently from Phase 2.

---

## Risks and Mitigations

| Risk | Impact | Likelihood | Mitigation |
|------|--------|------------|------------|
| Procedural audio sounds bad | Game feels cheap | Medium | Generate multiple variants per sound, let Art Director pick. Fall back to free CC0 8-bit samples if needed. |
| Runtime WAV loading not supported without plugin | Audio blocked | Medium | Use editor Python scripting to create .uasset wrappers. Worst case: commit raw WAVs and load via FFileHelper. |
| PlayUnreal latent tests are flaky | False E2E failures | Medium | Add generous timing tolerances. Use deterministic frame stepping if available. |
| Seam tests require complex multi-actor setup | Tests are slow/brittle | Low | Keep seam tests focused on one interaction each. Reuse existing test helpers from IntegrationTest.cpp. |
| M_FlatColor.uasset generation fails | Packaging risk remains | Low | Already have runtime fallback. This is polish, not blocking. |

---

## Test Count Projection

| Category | Existing | New | Sprint 4 Total |
|----------|----------|-----|----------------|
| Unit tests | ~80 | 0 | ~80 |
| Integration | 5 | 0 | 5 |
| Delegate wiring | 9 | 0 | 9 |
| Seam tests | 0 | 7 | 7 |
| Audio tests | 0 | 5 | 5 |
| PlayUnreal E2E | 0 | 5 | 5 |
| Functional (editor) | 5 | 0 | 5 |
| **Total** | **101** | **~17** | **~118** |

---

## Sprint 4 Exit Criteria

Sprint 4 is DONE when:

1. All 12 tasks are complete per their individual requirements
2. Both Game and Editor targets build with `Result: Succeeded`
3. All automated tests pass (~118 tests, 0 failures)
4. Seam tests cover all system interaction pairs
5. PlayUnreal E2E harness can run 5 scripted scenarios
6. 9 sound effects play at appropriate game events
7. QA Lead has passed the 15-item smoke test checklist
8. Retrospective has been run and agreements updated
9. All files committed with conventional commit messages

---

## Carried-Forward Items (Deferred to Sprint 5)

These are explicitly not in Sprint 4:

- **P1: Run functional tests in CI** (Xvfb/Gauntlet headless mode investigation)
- **P2: Visual regression testing** (screenshot comparison)
- **P2: Packaging step in CI** (catch WITH_EDITORONLY_DATA issues)
- **Feature: Background music** (gameplay loop, title theme — audio infra must land first)
- **Feature: Particle effects** (death puff, hop trail, round complete celebration)
- **Feature: HUD polish** (custom font, arcade styling, score pop animation)
- **Feature: Persistent high scores** (save to disk)
- **Feature: Title screen** (state exists, needs visual implementation)

---

*"The first step toward a better game is being able to hear it." — Koji Kondo (paraphrased)*

*Sprint 4 gives our agents ears (PlayUnreal) and gives our players ears (audio). Both are overdue.*
