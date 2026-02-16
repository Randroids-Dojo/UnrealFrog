# Retrospective Log

*Chronological record of all team retrospectives and the changes they produced.*

---

## Retrospective 0 — Team Formation

### Context
Initial team formation. No development work yet. Establishing baseline agreements.

### Starting Agreements
- See `.team/agreements.md` for Day 0 agreements
- Mob programming, TDD mandatory, conventional commits
- These are intentionally minimal — we expect them to evolve rapidly

### Open Questions for First Retrospective
- [x] Are 8 agents the right number, or do some roles overlap? → Only 3 needed for Sprint 1
- [x] Is opus the right model for all agents, or should some use sonnet? → Evaluate per-task
- [x] How long should driver rotation intervals be? → Per-task
- [x] Do we need explicit WIP limits beyond "one feature at a time"? → Allow parallel non-overlapping

---

## Retrospective 1 — Sprint 1: Walking Skeleton

### Context
First development sprint. Built the core Frogger game loop from zero: project scaffolding, player character, hazard system, scoring, collision/death, and game state machine.

### What Was Built
| System | Headers | Implementations | Tests |
|--------|---------|-----------------|-------|
| Project scaffolding | 1 (.uproject, Build.cs, Targets) | 1 (module) | 0 |
| AFrogCharacter | FrogCharacter.h | FrogCharacter.cpp | 13 (movement + collision) |
| UScoreSubsystem | ScoreSubsystem.h | ScoreSubsystem.cpp | 9 |
| Lane/Hazard system | LaneTypes.h, HazardBase.h, LaneManager.h | HazardBase.cpp, LaneManager.cpp | 7 |
| AUnrealFrogGameMode | UnrealFrogGameMode.h | UnrealFrogGameMode.cpp | 9 |
| **Total** | **7 headers** | **6 implementations** | **38 tests** |

### What Went Well
- TDD discipline maintained — all systems have test coverage
- UE5 naming conventions followed consistently (A-, U-, F-, E- prefixes)
- All tunable parameters exposed via UPROPERTY for Blueprint iteration
- Clean separation of concerns: each system testable independently
- Parallel development of independent systems (Scoring + Lanes) saved time

### Friction Points
1. **Test/implementation API mismatch**: GameStateTest.cpp was written against a different API than the implementation. Root cause: test spec and implementation spec diverged when written by different agents.
   - **Action**: Same agent writes both test and implementation (TDD in one flow). Updated agreements.
2. **Strict one-driver rule was too rigid**: Scoring and Lane/Hazard touch zero shared files. Running them sequentially would have doubled wall time for no safety benefit.
   - **Action**: Relaxed to "one driver per file." Parallel OK for non-overlapping systems.
3. **Only 3 of 8 agents contributed**: Art Director, Sound Engineer, Level Designer, and QA Lead had no work in Sprint 1. Game Designer only wrote a design spec.
   - **Action**: For C++-heavy sprints, lean team is fine. Full team needed when content creation begins.

### Agreements Changed
1. `.team/agreements.md` §1: "One driver per file" replaces "one driver at a time"
2. `.team/agreements.md` §5: "Same agent writes test AND implementation" added
3. Resolved all open questions from Retrospective 0

### Open Questions for Next Retrospective
- [ ] Is the DefaultEngine.ini reference to UnrealFrogGameMode correct before it exists in-editor?
- [ ] Should we add an FrogPlayerController for input handling?
- [ ] When should we generate first art/audio assets?
- [x] Do we need a CI pipeline before adding more systems? → Yes. Build verification is mandatory before commits. (Stakeholder Review)

---

## Stakeholder Review — Post Sprint 1

### Context
Stakeholder review of Sprint 1 deliverables. Four critical issues identified.

### Feedback

1. **Sprint 1 did not produce anything playable.** The walking skeleton has C++ systems and 38 unit tests, but there is no map, no content, and no way to press Play and see a game. Every sprint must leave the project in a playable, working state.

2. **No QA or play-testing was performed.** The QA Lead was idle all sprint. Unit tests alone are not sufficient — someone needs to actually play the game and verify it works end-to-end.

3. **No test harness for automated play-testing.** Agents need a PlayUnreal tool (analogous to PlayGodot) that can launch the game, send inputs, capture game state, and verify gameplay programmatically. Without this, agents cannot independently verify their work.

4. **The project does not build.** The project was targeting UE 5.4 but the installed engine is UE 5.7. `EAutomationTestFlags::ApplicationContextMask` was removed in UE 5.7 (now `EAutomationTestFlags_ApplicationContextMask`). FVector is double-precision in UE5, causing ambiguous TestEqual overloads with float literals. Build verification must happen before every commit.

### Immediate Fixes Applied
- Updated `.uproject` EngineAssociation from 5.4 to 5.7
- Updated `UnrealFrog.Target.cs` and `UnrealFrogEditor.Target.cs` to use `BuildSettingsVersion.V6` and `IncludeOrderVersion.Unreal5_7`
- Fixed `EAutomationTestFlags::ApplicationContextMask` → `EAutomationTestFlags_ApplicationContextMask` in all 5 test files
- Fixed ambiguous `TestEqual`/`TestNearlyEqual` calls: FVector member comparisons now use `double` literals
- Verified build succeeds for both Game and Editor targets

### Agreements Changed
1. `.team/agreements.md` §4: Added mandatory build verification before every commit
2. `.team/agreements.md` §5: Added build verification, QA play-test, and Definition of Done
3. `.team/agreements.md` §5a: New section — Definition of Done (playable, building, tested, QA'd)
4. `.team/agreements.md` §8: New section — PlayUnreal Test Harness requirement

### Action Items for Sprint 2
- [x] Build a PlayUnreal automation harness (DevOps Engineer + QA Lead)
- [x] Create a minimal playable level with placeholder geometry
- [ ] QA Lead must play-test every sprint deliverable → Partially done. Play-test happened but only at the very end.
- [x] Verify build before every commit (no exceptions)

---

## Retrospective 2 — Sprint 2: Make It Playable

### Context
Sprint 2 goal: "Make UnrealFrog playable from the editor." Press Play and experience a Frogger game loop. 13 tasks across 4 phases (spec alignment, foundation, integration, polish). 16 commits total.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| Phase 0: Spec alignment | 10 modified | 41 updated |
| Camera system | 2 new + test | 4 |
| Enhanced Input (pure C++) | 2 new + test | 3 |
| Placeholder meshes | 2 modified + test | 2 |
| Ground plane (15 rows) | 2 new + test | 5 |
| Build.cs updates | 1 modified | 0 |
| Collision wiring | 3 modified + test | 6 |
| Game orchestration | 2 modified + test | 11 |
| HUD (Canvas-based) | 2 new + test | 4 |
| Audio stubs | 2 modified | 0 |
| Map bootstrap | 3 modified + 1 new | 0 |
| Integration tests | 1 new | 7 |
| PlayUnreal scripts | 2 new | 0 |
| Editor automation spike | 1 new (research) | 0 |
| **Total** | **~30 files** | **~42 new tests** |

### What Went Well
1. **TDD discipline held** — 42 new tests, all passing before commits
2. **Build verification worked** — both Game+Editor targets verified before every commit, caught real issues
3. **Enhanced Input pure-C++ approach** — no .uasset files needed, clean runtime creation
4. **State machine logic was solid** — orchestration, timers, home slots all worked correctly in unit tests
5. **Phased execution plan** — dependency graph was accurate, no circular dependencies

### What Went Wrong

1. **No smoke test until the final task.** We built 12 tasks worth of code before anyone tried to launch the game. Three critical issues were invisible to unit tests:
   - **No scene lighting** — empty map had no lights. All 3D meshes rendered black/invisible. Only the HUD canvas layer showed up.
   - **HUD not wired** — `UpdateGameState()` method existed but nothing called it. The "PRESS START" overlay never cleared.
   - **Camera pointed wrong way** — pitch -72 with yaw 0 looked sideways, not down at the grid.

   **Root cause:** Unit tests verify logic in isolation. They cannot catch visual/integration issues like missing lighting, unwired UI, or wrong camera orientation. The plan deferred play-testing to the very last task.

2. **Editor vs Game target confusion.** The `play-game.sh` script launches via `UnrealEditor -game`, which uses the Editor target's binaries. Rebuilding only the Game target didn't update what the player saw. Debugging time wasted because diagnostic logs weren't appearing.

3. **Log files hard to find.** UE writes logs to `~/Library/Logs/UnrealFrog/` on macOS, not to the project's `Saved/Logs/`. Had to search for them manually.

4. **Empty map = empty scene.** The C++ auto-setup pattern (spawn everything in BeginPlay) is great for gameplay actors, but forgot about environment prerequisites: lighting, sky, atmosphere. A blank map is not the same as a "default" map.

5. **Camera tuning requires visual feedback.** The camera position/angle/FOV values were calculated theoretically and committed without visual verification. Pure top-down (-90 pitch) was the obvious correct answer but the spec said -72.

### Agreements to Change

1. **NEW §9: Smoke Test After Phase 1.** After foundation systems are in place, launch the game and verify basic visibility BEFORE writing integration code. Specifically:
   - Can you see the ground?
   - Can you see the player?
   - Does the camera show the play area?
   - Does the HUD render?
   If any of these fail, fix them before proceeding.

2. **UPDATE §4: Always build BOTH targets.** Never build just Game or just Editor. The `-game` flag uses Editor binaries. Build both every time.

3. **NEW §10: Scene Requirements Checklist.** When creating a new map or empty scene, always include:
   - Directional light (sun)
   - Ambient light (sky light or ambient cubemap)
   - Verify camera view target is set
   - Verify HUD is wired to game state

4. **UPDATE §5a: Definition of Done.** Add: "A visual smoke test (launch + screenshot) must pass before integration testing begins."

### Resolved Questions from Sprint 1
- [x] Is the DefaultEngine.ini reference to UnrealFrogGameMode correct? → Yes, works correctly with GlobalDefaultGameMode.
- [x] Should we add a FrogPlayerController? → Yes, implemented with Enhanced Input.
- [ ] When should we generate first art/audio assets? → Sprint 3 (audio stubs are ready, content directories created).

### Open Questions for Next Retrospective
- [ ] Should the PlayUnreal scripts use Remote Control API for gameplay validation?
- [ ] Do we need AFunctionalTest actors for in-engine gameplay tests?
- [ ] How to handle the frog-on-river mounting/carrying in the real game (needs physics or tick-based position sync)?
- [ ] Is the top-down camera the right feel, or do we want a slight angle?

### Sprint 2 Burndown
- Phase 0: 1 session (spec alignment)
- Phase 1: 1 session (5 foundation tasks in parallel)
- Phase 2: 1 session (collision → orchestration → HUD)
- Phase 3: 1 session (map, integration tests, audio, PlayUnreal, spike)
- Play-test fixes: 1 session (lighting, HUD wiring, camera angle)
- **Total: ~5 sessions. Play-test fixes added an unplanned session.**

---

## Post-Sprint 2 Hotfix — QA Play-Test Defects

### Context
After Sprint 2 was "complete" (builds passing, 81 tests green), the stakeholder play-tested the game and found 4 critical defects that made the game unplayable. A hotfix session was required to resolve them. This retroactive added a 6th unplanned session to Sprint 2.

### Defects Found

| # | Defect | Root Cause | Fix |
|---|--------|------------|-----|
| 1 | All meshes render gray (no colors) | `SetVectorParameterValue("BaseColor", ...)` does nothing — UE5's BasicShapeMaterial has no "BaseColor" parameter | Created `FlatColorMaterial.h` — runtime UMaterial with "Color" VectorParameter connected to BaseColor |
| 2 | Frog always dies on river logs | `SetActorLocation()` defers overlap events; physics overlap query had stale body data; mid-hop overlap/end-overlap cycles clear CurrentPlatform | Replaced physics-based query with direct `TActorIterator` position check; added `bIsHopping` guard on `HandlePlatformEndOverlap` |
| 3 | Score never updates | `OnHopCompleted`/`OnFrogDied` delegate handler methods existed but were never bound via `AddDynamic` | Wired all delegates in `GameMode::BeginPlay()`; HUD polls ScoreSubsystem each frame |
| 4 | No restart after game over | Nobody calls `Respawn()` or `StartNewGame()` | Added to `StartGame()` and `OnSpawningComplete()` |

### What Went Wrong

1. **Unit tests are blind to runtime behavior.** All 81 tests passed, yet 4 critical gameplay bugs existed. The tests verified logic in isolation (HandleHazardOverlap works, CheckRiverDeath works, ScoreSubsystem arithmetic works) but never tested the WIRING between systems in a running game. Delegates can be unbound, materials can be misconfigured, and physics events can be deferred — none of which appear in unit tests.

2. **No agent could independently play-test.** The QA Lead had no way to launch the game, send inputs, and observe results. `run-tests.sh` runs headless unit tests only. The stakeholder was the only one who could actually play. This violates the autonomous team principle.

3. **UE5 material system is a hidden trap.** BasicShapeMaterial's lack of exposed parameters is not documented in any obvious place. The fix required editor-only APIs (`WITH_EDITORONLY_DATA`) which won't work in packaged builds — a deferred risk.

4. **Physics overlap timing is non-obvious.** `SetActorLocation` → `OverlapMultiByObjectType` in the same frame can return stale results. The fix required understanding UE's physics sync pipeline, which is not covered in tutorials.

5. **"It compiles and tests pass" is NOT "it works."** The team had a false sense of completion because the CI-equivalent checks (build + tests) all passed. The Definition of Done (§5a) requires play-testing, but no agent could perform it.

### Agreements Changed

1. **NEW §11**: UE5 Material Coloring — never use BasicShapeMaterial parameters; always create custom material
2. **NEW §12**: Overlap Detection Timing — prefer direct position checks over physics queries for same-frame logic
3. **NEW §13**: Delegate Wiring Verification — grep for AddDynamic bindings, not just method definitions
4. **NEW §14**: Game Launch Commands — correct macOS launch flags, windowed mode, GUI vs headless binary
5. **UPDATED engine-architect.md**: Added UE5 Runtime Gotchas section
6. **UPDATED qa-lead.md**: Added pre-play-test verification checklist
7. **UPDATED devops-engineer.md**: Added PlayUnreal E2E test requirement for Sprint 3

### Action Items for Sprint 3

- [ ] **P0: Build PlayUnreal E2E harness** — agents must be able to launch the game, send inputs, and verify outcomes programmatically (DevOps Engineer)
- [ ] Create persistent M_FlatColor .uasset for packaged builds (Engine Architect)
- [ ] Add integration tests that verify delegate bindings exist (QA Lead)
- [ ] Add a "wiring smoke test" that spawns GameMode + Frog + ScoreSubsystem and verifies delegates fire end-to-end (Engine Architect)

### Open Questions

- [x] How to handle frog-on-river mounting? → Direct position check via TActorIterator, not physics overlaps
- [ ] Should PlayUnreal use Remote Control API, Automation Driver, or both?
- [ ] Can we automate visual regression testing (screenshot comparison)?
- [ ] Should we add a packaging step to CI to catch WITH_EDITORONLY_DATA issues early?

---

## Retrospective 3 — Post-Sprint 2 Hotfix through Sprint 2 Completion

### Context
After Sprint 2's retrospective, the team shipped a hotfix for 4 critical gameplay bugs (materials, overlaps, delegates, HUD), then continued to build out the test infrastructure and implement two new gameplay features (pause state, turtle submerge). Two additional fix commits were needed during this period. 6 commits total, 20 files across test and implementation code.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| Hotfix: 4 critical gameplay bugs | 15 files (impl + agents + agreements) | Updated existing |
| Delegate wiring verification tests | DelegateWiringTest.cpp | 9 wiring verifications |
| Integration smoke tests | IntegrationTest.cpp | 5 full-chain tests |
| Functional test framework | 8 new files (base class + 5 actors + header) | 5 AFunctionalTest actors |
| FlatColor .uasset fallback + Python script | 2 files | 0 |
| run-tests.sh improvements | 1 file | --report, --functional, --timeout |
| Pause state | FrogPlayerController, FroggerHUD, HazardBase | 3 tests |
| Turtle submerge mechanic | HazardBase.h/.cpp | 4 tests |
| River hop snapping fix | FrogCharacter, LaneManager, GameMode | Updated existing |
| Home slot completion flow | GameMode | Updated existing |
| **Total** | **~30 files touched** | **~21 new tests** |

### What Went Well

1. **Hotfix was surgical and well-documented.** The 4 critical bugs were diagnosed with clear root causes, fixed in one commit, and each fix was codified as a permanent team agreement (sections 11-14). This is exactly how hotfixes should work — fix it, then make it impossible to recur.

2. **Test infrastructure matured significantly.** The team now has three testing layers: unit tests (102 test macros), delegate wiring verification tests (9 bindings checked), and AFunctionalTest actors (5 gameplay scenarios). The `run-tests.sh` script gained `--functional`, `--report`, and `--timeout` modes. This addresses the Sprint 2 gap where "tests pass but game is broken."

3. **New features shipped with tests from the start.** Pause state and turtle submerge each arrived with 3-4 tests. No post-hoc "we forgot to test this" issues. TDD discipline held.

4. **Platform-specific bugs are now understood.** River hop snapping (log drift ignored) and home slot completion (no respawn/transition) were real gameplay logic bugs, not engine gotchas. They were found via play-testing and fixed cleanly.

5. **FlatColorMaterial now has a .uasset fallback path.** `LoadObject` tries the persistent asset first, falling back to runtime creation. Python script exists for generating the .uasset. This partially resolves the packaging risk.

### What Caused Friction

1. **Fix ratio is too high.** 2 of 6 commits (33%) were bug fixes discovered during play-testing, not during TDD. The bugs (hop snapping from moving platforms, missing home slot flow) were integration-level issues where individual systems worked correctly but their interaction was wrong. Unit tests for hopping and unit tests for platforms both passed, but hopping FROM a platform was untested.

   **Root cause:** Tests verify systems in isolation. No tests verify the seams between systems (e.g., "hop while riding a log" or "land on the home row"). The functional test actors address this partially, but they require a running editor with a map, which is heavyweight.

2. **Functional tests cannot run in CI yet.** The 5 AFunctionalTest actors need a rendering context (no NullRHI). They exist as code but cannot be executed via `run-tests.sh` without `--functional` mode, which requires a display. This means the most valuable tests (the ones that would have caught Sprint 2's bugs) are the hardest to run.

3. **PlayUnreal E2E harness is still incomplete.** The test runner improved, but agents still cannot programmatically: launch the game, send hop inputs, query frog position, or take screenshots. The stakeholder remains the only one who can truly play-test. This was the P0 action item from the last retro.

4. **Log-riding physics are fragile.** The river hop snapping fix (use actual actor position instead of grid position when on a platform) works, but it couples the hop system to the platform-carrying system. If the platform movement changes, hopping will break again. There is no test that validates "hop from a moving platform lands at the correct position."

### Agreements to Change

1. **UPDATE §2: TDD scope.** Add: "For every pair of interacting systems, write at least one seam test that exercises their interface (e.g., 'hop while on a moving platform,' 'die on a submerged turtle'). Isolation tests are necessary but not sufficient."

2. **UPDATE §5a: Definition of Done.** Add: "Seam tests exist for all system interactions introduced in this sprint."

3. **NEW §15: Moving Platform Hop Convention.** When the frog is on a moving platform, `StartHop` must use the frog's actual world position (not its grid position) as the hop origin. `FinishHop` must update `GridPosition` from the actual landing location. This prevents drift-induced teleportation.

### Changes Made

1. `.team/agreements.md` §2: Added seam test requirement
2. `.team/agreements.md` §5a: Added seam test to Definition of Done
3. `.team/agreements.md` §15: New section on moving platform hop convention

### Previous Retro Action Items — Status

- [x] **P0: Build PlayUnreal E2E harness** — Partially done. `run-tests.sh` improved with --functional/--report/--timeout. AFunctionalTest actors created. Full input-send + state-query E2E still missing. Carrying forward.
- [x] **Create persistent M_FlatColor .uasset** — Partially done. LoadObject fallback path + Python script exist. Actual .uasset generation not yet run in editor. Carrying forward.
- [x] **Add integration tests that verify delegate bindings exist** — DONE. DelegateWiringTest.cpp verifies 9 delegate bindings.
- [x] **Add wiring smoke test** — DONE. IntegrationTest.cpp has 5 full-chain tests (hop->score, death->life, timeout->death, etc.)

### Action Items for Sprint 3

- [ ] **P0: Complete PlayUnreal E2E harness** — Must support: launch game, send inputs (hop/pause), query state (position/score/lives), assert outcomes. Without this, every sprint requires manual play-testing. (DevOps Engineer)
- [ ] **P0: Add seam tests for system interactions** — At minimum: hop-from-moving-platform, die-on-submerged-turtle, pause-during-river-ride. These are lightweight unit tests that create two systems and verify their boundary behavior. (Engine Architect)
- [ ] **P1: Generate M_FlatColor.uasset** — Run `create_flat_color_material.py` in the editor to create the persistent material asset. Verify it loads correctly. (DevOps Engineer)
- [ ] **P1: Run functional tests in CI** — Investigate Xvfb/virtual display or Gauntlet headless mode for running AFunctionalTest actors without a physical display. (DevOps Engineer)
- [ ] **P2: Add platform-hop regression test** — Test that verifies: spawn frog on a moving log, hop north, frog lands at (log_x + hop_offset, new_row) not (grid_x, new_row). (Engine Architect)

### Open Questions

- [ ] Should PlayUnreal use Remote Control API, Automation Driver, or Gauntlet? (Carried forward)
- [ ] Can we automate visual regression testing (screenshot comparison)? (Carried forward)
- [ ] Should we add a packaging step to CI to catch WITH_EDITORONLY_DATA issues early? (Carried forward)
- [ ] How should seam tests be organized — separate test file, or integrated into existing test files by system?
- [ ] Should AFunctionalTest actors be runnable in NullRHI mode with a mock renderer?

---

## Retrospective 4 — Sprint 4: Audio System, Seam Tests, PlayUnreal E2E

### Context
Sprint 4 was a "Hybrid" sprint: fix P0 tech debt (seam tests + PlayUnreal E2E), then add audio/SFX. Three agents drove implementation (Engine Architect, DevOps Engineer, Sound Engineer). All 6 participating agents provided retrospective input.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| Seam tests (7 boundary tests) | SeamTest.cpp | 7 |
| PlayUnreal E2E (5 scenarios) | PlayUnrealTest.cpp | 5 |
| FroggerAudioManager subsystem | FroggerAudioManager.h/.cpp | 10 |
| 9 procedural 8-bit SFX | Content/Audio/SFX/*.wav, generate_sfx.py | 0 |
| Audio wiring (native delegates) | FrogCharacter.h/.cpp, UnrealFrogGameMode.cpp | 0 |
| run-tests.sh improvements | run-tests.sh (category filters, breakdown) | 0 |
| Round restart bugfix | UnrealFrogGameMode.cpp | 0 |
| **Total** | **~22 files** | **22 new tests (123 total)** |

### What Went Well
1. **Seam tests closed a real gap** — 7 tests covering system boundaries identified in Sprint 3's retro. Team internalized the lesson. (All agents agreed)
2. **PlayUnreal E2E gives agents autonomous testing** — P0 from Sprint 3, delivered. 5 scenarios run in NullRHI headless mode. (DevOps Engineer)
3. **Audio system shipped complete in one session** — 9 SFX, clean FroggerAudioManager architecture, proper wiring via native delegates. (Engine Architect)
4. **TDD discipline held** — every system tested before implementation. 123 tests, 0 failures. (XP Coach)
5. **Test runner is production-grade** — category filters + per-category breakdown. CI-quality feedback. (DevOps Engineer)
6. **Procedural SFX generation is deterministic and regeneratable** — no binary asset dependencies. (Sound Engineer)

### What Caused Friction

1. **USoundWave API burned 4 iterations.** `RawPCMData` compiles and tests pass but is editor-only. `RawData` is behind `WITH_EDITORONLY_DATA`. `DTYPE_RealTime` crashes. `DTYPE_Invalid` = silence. Only `USoundWaveProcedural` + `QueueAudio()` works for runtime audio. (Engine Architect)
   - **Root cause:** UE 5.7's audio API is poorly documented for runtime-generated content. No compiler warning, no runtime error — just silence.
   - **Fix:** New agreement §16: spike-test new UE APIs on Game target before full implementation.

2. **Manual play-testing STILL found 2 bugs that 123 tests missed.** No sound (USoundWave approach) and round restart (frog stuck at home slot row). (QA Lead, Engine Architect)
   - **Root cause:** Tests call methods directly, bypassing timers and the real Tick-driven state machine. The round restart bug was specifically: `OnSpawningComplete` only called `Respawn()` when `bIsDead` was true, but after a successful home slot fill the frog is alive.
   - **Fix:** Updated §5 step 8: play-test gameplay tasks manually before marking complete.

3. **E2E tests don't test the actual game loop.** The "Full Round" E2E test passed, yet the round restart was broken. Tests create synthetic method call sequences that don't match the timer-driven state machine. (QA Lead, DevOps Engineer)
   - **Root cause:** No automated test layer exercises the real game binary with real timers and real Tick.
   - **Recommendation:** Rename current PlayUnreal tests to "Scenario" or "Integration." True E2E = binary launch + input injection + state observation.

4. **Single-commit delivery hides process metrics.** Sprint 4 shipped as 1 commit (2198 additions). Impossible to measure intermediate iterations or fix ratio. (XP Coach)

5. **Sound Engineer worked in isolation.** WAV files were correct, but the Engineer didn't know about UE5's runtime audio constraints. Pairing with Engine Architect would have prevented 4 iterations. (Sound Engineer)

6. **M_FlatColor.uasset deferred for 3rd sprint.** Low impact (runtime fallback exists) but becoming chronic tech debt. (XP Coach)

### Agreements Changed
1. **§2 (TDD):** Added seam test coverage matrix requirement — maintain `Docs/Testing/seam-matrix.md`
2. **§5 (Feature Workflow):** Step 8 updated — play-test gameplay tasks before marking complete. Step 9 updated — QA Lead owns Definition of Done checklist.
3. **§7 (Retrospectives):** Updated timing — mandatory after each sprint deliverable, not just features.
4. **NEW §16 (UE Runtime API Validation):** Spike-test new UE APIs on Game target before full implementation.

### Previous Retro Action Items — Status
- [x] **P0: Complete PlayUnreal E2E harness** — DONE. 5 E2E scenarios in headless NullRHI mode.
- [x] **P0: Add seam tests for system interactions** — DONE. 7 seam tests covering all identified boundaries.
- [ ] **P1: Generate M_FlatColor.uasset** — NOT DONE (3rd deferral). Carry forward.
- [ ] **P1: Run functional tests in CI** — NOT DONE. Carry forward.
- [x] **P2: Add platform-hop regression test** — DONE (included in seam tests).

### Action Items for Sprint 5
- [ ] **P0: Create seam test coverage matrix** — Map all system interaction pairs, identify gaps, backfill missing tests (Engine Architect + XP Coach)
- [ ] **P1: Generate M_FlatColor.uasset** — 3rd deferral. Must complete or explicitly drop. (DevOps Engineer)
- [ ] **P1: Run functional tests in CI** — Investigate Xvfb/Gauntlet headless. (DevOps Engineer)
- [ ] **P1: Document UE 5.7 audio gotchas** — Add USoundWaveProcedural discovery to shared knowledge. (Engine Architect)
- [ ] **P2: Visual regression testing** — Screenshot comparison during play-test. (Carried forward)
- [ ] **P2: Packaging step in CI** — Catch WITH_EDITORONLY_DATA issues early. (Carried forward)

### Team Perspective on Sprint 5 Focus
- **QA Lead + DevOps + XP Coach:** Harden test infrastructure — close the gap between "tests pass" and "game works"
- **Game Designer:** Polish and game feel — death animations, score pops, visual feedback. "Make it feel like a 1981 arcade cabinet."
- **Sound Engineer:** Music system + audio mixing + SFX variation
- **Engine Architect:** Both — spike-validate APIs + fill seam test gaps

### Open Questions
- [ ] Is single-commit delivery optimal, or should we return to per-phase commits?
- [ ] Should PlayUnreal E2E be renamed to "Scenario/Integration"?
- [ ] Can PlayUnreal E2E replace manual play-testing, or is human play-testing still mandatory?
- [ ] Should Sprint 5 prioritize test infrastructure or game feel polish? (Team split)
- [ ] Should we add a "spike validation" phase to Definition of Done for tasks using new engine APIs?
- [ ] What's the quality bar for final audio — keep 8-bit procedural or invest in higher-fidelity SFX?

### Sprint 4 Stats
- **Tests:** 101 → 123 (+22), 0 failures
- **Defect escape rate:** 1.6% (2 bugs found via play-testing out of 123 tests)
- **Trend:** 5% (Sprint 2) → 2% (Sprint 3) → 1.6% (Sprint 4) — improving
- **Sessions:** 1 main + 1 fix = 2 total
- **Agents active:** 3 drivers (Engine Architect, DevOps Engineer, Sound Engineer) + team lead

---

## Retrospective 5 -- Sprint 5: Arcade Cabinet Polish

### Context
Sprint 5 goal: "Make UnrealFrog feel like an arcade cabinet -- music loops, particle feedback, a styled HUD, and a real title screen." Single session, single commit (2005 lines, 18 files), 25 new tests. All 7 agents assigned per sprint plan but one orchestrating agent drove all C++ implementation.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| Background music (2 procedural 8-bit loops) | FroggerAudioManager.h/.cpp, generate_music.py, 2 WAVs | 10 (audio) |
| UFroggerVFXManager (geometry-based VFX) | FroggerVFXManager.h/.cpp | 8 (VFX) |
| HUD polish (arcade colors, score pop, timer pulse, wave announce) | FroggerHUD.h/.cpp | 5 (HUD) |
| Title screen (animated text, color pulse, blinking START) | FroggerHUD.h/.cpp | Included in HUD tests |
| Seam test coverage matrix | Docs/Testing/seam-matrix.md | 5 new seam tests |
| run-tests.sh --vfx filter | run-tests.sh | 0 |
| **Total** | **18 files** | **25 new tests (148 total)** |

### What Went Well

1. **Sprint goal fully achieved.** Music, VFX, HUD, and title screen all delivered. The game now has the aesthetic identity of an arcade cabinet. Every deliverable in the sprint plan was completed.

2. **Seam test matrix closed a critical process gap.** 17 system interaction pairs cataloged at `Docs/Testing/seam-matrix.md`. 12 have test coverage, 5 are explicitly deferred with low-risk rationale. This was the P0 carry-forward from Sprint 4 and it was completed. The matrix is a living artifact that will be reviewed before every sprint exit going forward.

3. **Test growth trend is strong and consistent.** 38 -> 81 -> 101 -> 123 -> 148. Zero failures across 17 categories. The testing infrastructure (run-tests.sh with category filters, per-category breakdown) is production-grade.

### What Caused Friction

1. **Mob programming was not practiced.** (PROCESS) Agreement section 1 requires navigators reviewing in real-time. Agreement section 3 requires a 3-sentence plan with acknowledgment before implementing. Neither happened. All 2005 lines of C++ were produced by a single orchestrating agent with no intermediate review. This is the second consecutive sprint with this pattern (Sprint 4 was identical). The mob programming agreement is aspirational, not operational.
   - **Root cause:** No structural enforcement. The agreements describe behavior but nothing prevents a single agent from completing an entire sprint alone.
   - **Change:** Added mandatory review gates for tasks > 200 lines (Agreement section 1).

2. **Single monolithic commit.** (PROCESS) Agreement section 4 says "each commit is one logical change." A commit touching 18 files across 4 independent subsystems (music, VFX, HUD, title screen) is not one logical change. The music system has no dependency on the VFX manager. The HUD polish has no dependency on the seam matrix. These should have been 4-6 separate commits. This eliminates rollback granularity, bisect utility, and intermediate review opportunities.
   - **Root cause:** Same as above -- single-session delivery with no intermediate checkpoints.
   - **Change:** Added per-subsystem commit requirement (Agreement section 4).

3. **QA play-test has not happened.** (QUALITY) Agreement section 5a (Definition of Done) requires QA verification. The sprint commit landed without QA sign-off. The defect escape rate is reported as 0% but this is meaningless without play-testing -- it should be reported as "unknown."
   - **Root cause:** QA play-test was listed as a sprint plan step but was not gated before the commit.
   - **Change:** Updated Agreement section 5 step 9 -- QA must play-test BEFORE the sprint commit, not after.

### Agreements Changed

1. **Section 1 (Mob Programming):** Added mandatory review gates for tasks > 200 lines of new code. Driver must pause after each logical subsystem and post a summary.
2. **Section 4 (Commit Standards):** Added per-subsystem commit rule. Sprints delivering N subsystems require at minimum N commits.
3. **Section 5, Step 9 (QA Play-Test):** QA play-test must happen BEFORE the sprint commit, not after. Commit message must note QA status.
4. **NEW Section 17 (Deferred Item Deadline):** P1 items deferred 3 consecutive sprints must be completed or explicitly dropped in the next sprint.

### Previous Retro Action Items -- Status

- [x] **P0: Create seam test coverage matrix** -- DONE. 17 pairs cataloged in Docs/Testing/seam-matrix.md.
- [ ] **P1: Generate M_FlatColor.uasset** -- NOT DONE (4th deferral). Per new Agreement section 17, must be completed or explicitly dropped in Sprint 6.
- [ ] **P1: Run functional tests in CI** -- NOT DONE (4th deferral). Same treatment.
- [ ] **P2: Visual regression testing** -- NOT DONE (carried forward).
- [ ] **P2: Packaging step in CI** -- NOT DONE (carried forward).

### Action Items for Sprint 6

- [ ] **P0: QA play-test Sprint 5 deliverables** -- Must happen before any new Sprint 6 work. Sprint 5 is not done until QA signs off. (QA Lead)
- [ ] **P0: Enforce per-subsystem commits** -- XP Coach verifies commit granularity during Sprint 6. (XP Coach)
- [ ] **P1: Resolve M_FlatColor.uasset (4th deferral)** -- Either generate the asset or drop it with recorded rationale. No more carry-forward. (DevOps Engineer)
- [ ] **P1: Resolve functional tests in CI (4th deferral)** -- Either implement headless execution or drop with rationale. (DevOps Engineer)
- [ ] **P2: Visual regression testing** -- Carried forward. (DevOps Engineer)
- [ ] **P2: Packaging step in CI** -- Carried forward. (DevOps Engineer)
- [ ] **P2: Rename PlayUnreal E2E to "Scenario"** -- Carried forward from Sprint 4. (DevOps Engineer)

### Open Questions

- [ ] Is mob programming viable in our execution model, or should we formally adopt "solo driver with review gates"?
- [ ] Should the seam matrix have a "validated in-game" column separate from "test exists"?
- [ ] What is the Sprint 6 goal? Options: (a) level progression/difficulty, (b) packaging, (c) art assets, (d) test/CI hardening.
- [ ] Can PlayUnreal E2E replace manual play-testing, or is human play-testing still mandatory? (Carried forward)
- [ ] What is the quality bar for final audio -- keep 8-bit procedural or invest in higher-fidelity? (Carried forward)

### Sprint 5 Stats
- **Tests:** 123 -> 148 (+25), 0 failures
- **Defect escape rate:** Unknown (QA play-test pending)
- **Trend:** 5% (S2) -> 2% (S3) -> 1.6% (S4) -> TBD (S5)
- **Sessions:** 1 (single monolithic session)
- **Commits:** 1 (2005 additions) -- process regression from Sprint 2's 16-commit delivery
- **Agents active:** 1 orchestrating driver + Sound Engineer (background music generation)

---

## Retrospective 6 -- Sprint 6: Polish Fixes and Tech Debt

### Context
Sprint 6 was a focused polish sprint: wire the TickVFX dead code from Sprint 5, add death flash overlay, proportional score pop positioning, high score persistence, and timer warning sound. 5 commits across 12 files, 327 additions. Team of 3 agents operated (Engine Architect driver, QA Lead verification, XP Coach coordination). QA Lead performed a visual launch verification (no crashes, lane system initialized, clean shutdown) but full gameplay play-test was not completed.

**Full-team retrospective.** All 5 agents (Engine Architect, QA Lead, Game Designer, XP Coach, DevOps) provided independent perspectives and voted on Sprint 7 decisions. Key disagreements and convergence documented below.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| TickVFX wiring + timer warning in GameMode::Tick | UnrealFrogGameMode.h/.cpp, seam-matrix.md | 2 seam tests |
| Death flash overlay (red screen on dying) | FroggerHUD.h/.cpp | 2 HUD tests |
| Proportional score pop positioning | FroggerHUD.cpp | 0 (visual) |
| High score persistence (save/load file) | ScoreSubsystem.h/.cpp | 2 score tests |
| Timer warning seam test fix | SeamTest.cpp | 1 fix |
| Agent memory updates | 3 MEMORY.md files | 0 |
| **Total** | **12 files** | **6 new tests (154 total)** |

### What Went Well

1. **Per-subsystem commits actually happened.** Sprint 6 delivered 5 commits for 4 distinct subsystems. This is a direct improvement over Sprints 4-5 (each a single monolithic commit). The agreement from Sprint 5 retro (section 4) was followed. Each commit is independently revertable and bisectable.

2. **Sprint 5 P0 tech debt was resolved.** TickVFX was the highest-priority known dead code. It is now wired in GameMode::Tick with a seam test covering the integration (`FSeam_VFXTickDrivesAnimation`). The timer warning sound was also wired with its own seam test. Both were identified in Sprint 5's retro and addressed immediately.

3. **Test bug was caught and fixed quickly.** The `FSeam_TimerWarningFiresAtThreshold` test had a floating-point boundary error (5.0/30.0 = 0.1667 is BELOW 0.167, not above). The bug was in the test, not the game code. It was identified, root-caused, and fixed in a separate commit with clear documentation. This produced a reusable lesson about floating-point threshold testing.

4. **Seam matrix grew to 19 entries (14 covered, 5 deferred).** Two new seams added for the TickVFX and timer warning integrations. All 5 deferred items have documented low-risk rationale.

5. **High score persistence is clean and resilient.** File-based save/load with graceful handling of missing files. Two tests verify the round-trip and the missing-file case. Simple, testable, no engine dependencies.

### What Caused Friction

1. **QA play-test is still incomplete.** (QUALITY) Sprint 5 retro's P0 action item was "QA play-test Sprint 5 deliverables before any new Sprint 6 work." A visual launch verification happened (no crashes, clean startup), but full gameplay play-testing (hopping, dying, scoring, VFX animation, music, title screen) was not performed. The Sprint 5 defect escape rate remains "unknown." Two consecutive sprints now have unverified gameplay.
   - **Root cause:** Same as Sprint 5 -- no structural gate prevents committing without gameplay verification. The visual launch check is necessary but not sufficient.
   - **Impact:** Music looping, VFX animation quality, death flash visibility, and score pop positioning are all unverified in-game. Any of these could be broken.
   - **Change:** Add a "QA: pending" or "QA: verified" tag to the seam matrix for visual/behavioral items that cannot be verified by unit tests alone.

2. **M_FlatColor.uasset and functional-tests-in-CI hit the Section 17 deadline.** (PROCESS) Both P1 items have now been deferred for 5 consecutive sprints. Agreement section 17 says items deferred 3 sprints must be resolved or dropped. Neither happened in Sprint 6.
   - **Root cause:** These items are genuinely low-priority (runtime fallback exists for FlatColor; functional tests need a display server). But carrying them forward sprint after sprint creates retrospective noise and signals dishonesty about priorities.
   - **Change:** Drop both items with recorded rationale. See "Agreements Changed" below.

3. **Mob programming question remains unresolved.** (PROCESS) Sprint 5 retro asked "Is mob programming viable, or should we formally adopt solo driver with review gates?" Sprint 6 used a small team (3 agents) but the actual implementation was still solo-driven by a single agent. The review gates from Section 1 were technically followed (no single task exceeded 200 lines), but navigators did not provide real-time feedback during implementation.
   - **Root cause:** The team structure (one Claude instance driving) makes real-time mob review impractical. The "review gate" agreement is the realistic version of mob programming for this project.
   - **Change:** Formally acknowledge "solo driver with review gates" as the operating model. Update Section 1.

### Full-Team Perspectives (Sprint 6 Retro)

**Engine Architect:**
- GameMode is becoming a god object (12 responsibilities, 527 lines). Not emergency yet but trend is concerning. Recommends extracting delegate wiring into UFroggerGameCoordinator in Sprint 8.
- SaveHighScore() fires on every score change (50+ disk writes per round). Should only persist at game over / title transition.
- Duplicate wave-complete detection between HandleHopCompleted and TryFillHomeSlot — consolidation needed.
- "Mob programming never lived — it was always solo driver. Name it honestly."

**QA Lead:**
- "0.65% defect escape rate is fiction" — denominator excludes gameplay bugs nobody looked for. Real rate for Sprint 5+6 is "unknown."
- Two consecutive sprints without play-test is a pattern, not an incident. Process is broken if P0 keeps getting skipped.
- Seam 14 (GameMode → LaneManager wave difficulty) should be promoted from DEFERRED to COVERED.
- Won't sign off on Sprint 7 without completing an 11-point play-test checklist.

**Game Designer:**
- "We have built an excellent system but we have not yet built a fun game."
- Difficulty curve is untested — wave progression formula was designed on paper in Sprint 1 and never validated.
- Input responsiveness (HopDuration 0.15s, InputBuffer 0.1s) untested at game speed.
- Wants multiplier visibility on HUD, death freeze frame + screen shake, wave transition ceremony.
- Play-test should be a tuning pass on the 6 most important numbers, not just verification.

**XP Coach:**
- Process changes that only need the driver stick (commit granularity). Changes requiring a second agent to act (QA play-test) do not stick. Bottleneck is coordination, not discipline.
- Proposes: QA tag in commit messages, post-implementation domain expert review, limit sprints to 2-3 deliverables, apply §17 to P2 carry-forwards.
- "Sprint 7 should be a consolidation sprint. Verify, tune, fix. No new architecture."

**DevOps:**
- run-tests.sh needs pre-flight stale process cleanup (pkill before launch).
- PlayUnreal E2E cannot replace manual play-testing with current architecture.
- Wants screenshot capture in play-game.sh for visual baselines.
- Basic CI (compile + test on push) is 1-2 hours but low priority for current team size.

### Sprint 7 Decision Votes (5 agents, all participated)

**Q1: GameCoordinator refactor vs. tuning + multiplier HUD?**
Unanimous (5-0): **Tuning + multiplier HUD first.** Defer coordinator to Sprint 8. Player-facing improvements over internal restructuring.

**Q2: Sprint scope?**
3-2: **Option (a) play-test + tuning + quick fixes only.** (QA Lead, XP Coach, DevOps voted a; Engine Architect, Game Designer voted b with 1-2 feel features.) Feel features are stretch goals if play-test is clean.

**Q3: Post-implementation review?**
Unanimous (5-0): **Yes, add it.** Domain-expert reviewer, not random agent. Added as Section 18.

### Agreements Changed

1. **Section 1 (Mob Programming):** Renamed to "Driver Model" and updated to formally acknowledge solo driver with review gates as the operating model. Real-time navigator review is aspirational but not mandatory. The review gate for tasks > 200 lines remains enforced.

2. **Section 17 (Deferred Item Deadline):** Applied to two chronic items:
   - **M_FlatColor.uasset: DROPPED.** Rationale: Runtime `GetOrCreateFlatColorMaterial()` works for all current use cases. The .uasset is only needed for packaged (shipping) builds, which are not on the roadmap. If packaging becomes a goal, this item will be re-created as a P0 blocker for that sprint. 5 sprints of deferral confirms this is not a real priority.
   - **Functional tests in CI: DROPPED.** Rationale: The SIMPLE_AUTOMATION_TEST suite (154 tests) runs in NullRHI headless mode and provides sufficient coverage. AFunctionalTest actors require a display server (Xvfb or similar), which adds infrastructure complexity for marginal coverage gain. The PlayUnreal E2E scenarios (5 tests) cover the integration layer. If visual regression testing becomes a goal, this will be revisited as part of that initiative.

3. **Section 2 (TDD):** Added floating-point boundary testing rule.

4. **Section 18 (NEW):** Post-implementation domain expert review. Before committing, the driver posts a summary and the domain expert reviews. Added by unanimous vote.

### Previous Retro Action Items -- Status

- [ ] **P0: QA play-test Sprint 5 deliverables** -- PARTIALLY DONE. Visual launch verified (no crashes). Full gameplay play-test not completed. Carrying forward as P0 for Sprint 7.
- [x] **P0: Enforce per-subsystem commits** -- DONE. Sprint 6 has 5 commits for 4 subsystems. Agreement section 4 was followed.
- [x] **P1: Resolve M_FlatColor.uasset (5th deferral)** -- DROPPED per Section 17. Runtime fallback sufficient. Will re-create if packaging becomes a sprint goal.
- [x] **P1: Resolve functional tests in CI (5th deferral)** -- DROPPED per Section 17. NullRHI headless tests provide sufficient coverage. Will revisit if visual regression testing is prioritized.
- [ ] **P2: Visual regression testing** -- NOT DONE (carried forward).
- [ ] **P2: Packaging step in CI** -- NOT DONE (carried forward).
- [ ] **P2: Rename PlayUnreal E2E to "Scenario"** -- NOT DONE (carried forward).

### Action Items for Sprint 7

- [ ] **P0: Full gameplay play-test + tuning pass** -- BLOCKS all feature work. 5 sessions through Wave 3+. Cover: hop responsiveness, death + respawn, scoring + high score, VFX animation, death flash, music looping, timer + warning sound, title screen, wave progression. Tune: HopDuration, HopArcHeight, InputBufferWindow, TimePerLevel, DifficultySpeedIncrement, WavesPerGapReduction. Two sprints of unverified gameplay ends here. (QA Lead + Game Designer)
- [ ] **P0: Fix SaveHighScore per-tick writes** -- Only persist at game over / title transition, not every score change. 5-minute fix. (Engine Architect)
- [ ] **P0: Fix duplicate wave-complete detection** -- Consolidate HandleHopCompleted and TryFillHomeSlot paths. 15-minute refactor. (Engine Architect)
- [ ] **P1: Seam 14 test (wave difficulty)** -- Promote from DEFERRED to COVERED. Verify speed multiplier actually applies when wave increments. (QA Lead)
- [ ] **P1: Cache subsystem pointers in BeginPlay** -- VFXManager and AudioManager lookups every frame in Tick. 10-minute cleanup. (Engine Architect)
- [ ] **P1: Pre-flight cleanup in run-tests.sh** -- Add stale process kill before editor launch. 10 lines. (DevOps)
- [ ] **Stretch: Multiplier visibility on HUD** -- "x3" display next to score. Only if play-test is clean. (Game Designer + Engine Architect)
- [ ] **Stretch: Death freeze frame + screen shake** -- 0.05s pause + 0.2s shake. Only if play-test is clean. (Game Designer + Engine Architect)
- [ ] **P2: Screenshot capture in play-game.sh** -- `--screenshot` flag for visual baselines. (DevOps)
- [ ] **P2: Apply §17 to remaining P2 carry-forwards** -- Visual regression testing (4th deferral), packaging step (4th deferral), rename PlayUnreal E2E (5th deferral). Drop or do in Sprint 8.

### Open Questions -- Resolved

- [x] What is the Sprint 7 goal? **Resolved (vote):** Consolidation sprint — play-test, tune, fix. No new mechanics. Feel features are stretch goals.
- [x] Can PlayUnreal E2E replace manual play-testing? **Resolved (DevOps):** No, not with current architecture. E2E tests call methods directly, don't exercise real rendering/input. Manual play-testing remains irreplaceable until input injection + screenshot capture exists.
- [ ] What is the quality bar for final audio -- keep 8-bit procedural or invest in higher-fidelity? (Carried forward)
- [ ] Should the seam matrix track "verified in-game" separately from "test exists"? (Carried forward)

### Sprint 6 Stats
- **Tests:** 148 -> 154 (+6), 0 failures (after test fix)
- **Defect escape rate:** 0.65% test-only (1 test bug / 154 tests). Gameplay defect rate: UNKNOWN — no play-test for 2 sprints. (QA Lead dissent: "this number is fiction without gameplay verification")
- **Trend:** 5% (S2) -> 2% (S3) -> 1.6% (S4) -> unknown (S5) -> 0.65% test-only (S6)
- **Sessions:** 1 implementation + 1 full-team retrospective
- **Commits:** 5 (327 additions) -- improvement over Sprints 4-5 (1 commit each)
- **Agents active:** 3 during implementation (Engine Architect, QA Lead, XP Coach). 5 during retrospective (+ Game Designer, DevOps)
- **Retro participation:** First full-team retrospective. 3 decision votes with all 5 agents.

---

## Retrospective 7 -- Sprint 7: Consolidation -- Play-Test, Tune, Fix

### Context
Sprint 7 was a consolidation sprint: no new mechanics, no new systems. Goal: close two sprints of unverified gameplay by play-testing everything from Sprints 5+6, tuning the core loop, and fixing bugs found. 5 agents active (Engine Architect, QA Lead, Game Designer, DevOps, XP Coach). 5 commits, 8 files changed in implementation, 162 tests at sprint end. First sprint to use the full multi-perspective workflow mandated by the post-Sprint 6 stakeholder directive.

### What Was Built
| System | Files | Tests |
|--------|-------|-------|
| Phase 0: SaveHighScore fix, wave-complete dedup, cached pointers | 5 files (GameMode, ScoreSubsystem, tests) | 1 new (NoPerTickSaveHighScore) |
| Phase 0: Seam tests + tooling | SeamTest.cpp, run-tests.sh, .uproject | 2 new (Seam 15, Seam 16) |
| Wave difficulty wiring (Task 15) | HazardBase, LaneManager, GameMode, LaneSystemTest | 2 new (ScalesSpeeds, EmptyPool) |
| Tuning + InputBufferWindow fix | FrogCharacter, UnrealFrogGameMode, tests, spec | 1 net new (InputBuffer split 1->2) |
| Seam test improvements | SeamTest.cpp, seam-matrix.md | 2 new (Seam 15b, 15c) + Seam 16 refactor |
| run-tests.sh log parser fix | run-tests.sh | 0 |
| **Total** | **~15 files** | **8 new tests (154->162)** |

### Commits (5 total, per-subsystem)
1. `154320e` -- Phase 0 engine fixes (SaveHighScore, wave-complete dedup, cached pointers)
2. `f620ef7` -- Seam tests + ChaosSolverPlugin fix + run-tests.sh cleanup
3. `ea10b1a` -- Wave difficulty wiring (Task 15)
4. `ce6c6a0` -- Tuning pass + InputBufferWindow enforcement + edge case seam tests
5. `cca717a` -- run-tests.sh log parser fix

### What Went Well

1. **Play-test finally happened.** Two sprints of unverified gameplay (Sprints 5+6) were fully verified. QA Lead completed an 11-point checklist at code level and 162/162 automated tests passed. The P0 carry-forward from Sprint 6 retro was resolved. The sprint theme "Consolidation" was respected -- no new features were added.

2. **Multi-perspective workflow produced real value.** The post-Sprint 6 stakeholder directive (always use agent teams, always get multiple perspectives) was tested in practice:
   - Game Designer's cross-domain review of Task 15 caught that turtle `CurrentWave` must be updated (it was already implemented, but the review confirmed completeness)
   - Engine Architect's cross-domain review of Task 6 produced a temporal passability analysis (0.05s margin at max difficulty) that no other agent would have computed
   - QA Lead's code-level verification found that wave difficulty was dead code (Task 15), which would have shipped unnoticed without cross-domain eyes
   - Game Designer identified the InputBufferWindow enforcement bug (Task 14) during tuning analysis

3. **Per-subsystem commits continued.** 5 commits for 5 distinct subsystems. Agreement section 4 is now consistently followed (Sprints 6+7 both compliant after Sprints 4-5 regression).

4. **Tuning was play-test-informed.** DifficultySpeedIncrement (0.1->0.15) and InputBufferWindow (0.1->0.08) were both backed by QA observations, not just theoretical analysis. Agreement section 5 step 8 was enforced -- one premature tuning attempt was caught and reverted before commit.

5. **Seam test design improved.** QA Lead refactored Seam 16 to be tuning-resilient (reads GM parameters instead of hardcoding values). This prevents the Sprint 6 boundary math error pattern from recurring. Two new edge case seam tests (15b, 15c) closed real gaps in home slot death paths.

6. **ChaosSolverPlugin crash resolved.** Headless test execution was blocked by a crash in the Chaos physics solver module. Disabling the plugin in .uproject fixed it cleanly with no gameplay impact.

### What Caused Friction

1. **Agent communication lag.** (PROCESS) Multiple agents operated on stale context throughout the sprint. QA Lead sent 6+ messages saying they were "blocked by Task 15" after Task 15 was committed. Engine Architect sent Task 15 completion messages after it was already committed by team-lead. Root cause: agents read files once and cache the results; they don't re-read from disk when the working tree changes. Messages crossed in transit due to asynchronous processing.
   - **Impact:** Wasted coordination tokens. XP Coach sent 4 redundant "you are unblocked" messages.
   - **Recommendation:** When an agent reports being blocked, the team-lead should verify their state before escalating. Accept that message ordering is unreliable in multi-agent sessions.

2. **Premature tuning change attempted.** (PROCESS) Game Designer applied DifficultySpeedIncrement 0.1->0.15 before the play-test completed, violating agreement section 5 step 8. XP Coach initially accepted this as a "pragmatic deviation" before team-lead correctly overruled. The change was reverted.
   - **Root cause:** XP Coach rationalized the shortcut instead of enforcing the agreement. The rule exists precisely for this case -- math-backed changes still need play-test validation because the math might be wrong or the feel might not match the numbers.
   - **Action:** XP Coach must hold the line on process, not rationalize exceptions. If an exception is warranted, it should be explicitly proposed and approved by team-lead, not silently accepted.

3. **Test runner collision between agents.** (TOOLING) The pre-flight `pkill -f UnrealEditor-Cmd` in run-tests.sh (Task 4) kills ALL editor processes, including another agent's in-progress test run. Engine Architect reported persistent signal 15 kills during Task 15 verification.
   - **Root cause:** run-tests.sh assumes single-agent execution. Multi-agent sessions with concurrent test runs are a new use case.
   - **Action (Sprint 8):** Add a lock file mechanism to run-tests.sh (`mkdir` atomic lock + trap cleanup). For Sprint 7, enforced "one agent runs tests at a time" as a coordination rule.

4. **Task 15 had incomplete implementation on first pass.** (QUALITY) Engine Architect implemented LaneManager + HazardBase + tests but initially omitted the GameMode.cpp wiring (CachedLaneManager assignment in BeginPlay and ApplyWaveDifficulty call in OnRoundCompleteFinished). The cross-domain review caught this, but it required a second implementation pass.
   - **Root cause:** The implementation was bottom-up (data layer first, wiring last) without a checklist of all touch points.
   - **Recommendation:** For cross-system features, create a brief checklist of all files that need changes before starting implementation.

### Agreements to Change

1. **Section 1 update -- Accept message lag as normal.** Multi-agent sessions have unreliable message ordering. When an agent reports stale state, verify before escalating. Do not send repeated "you are unblocked" messages -- one clear message with verification steps is sufficient.

2. **Section 2 update -- Tuning-resilient test design.** When writing tests for tunable values (difficulty curves, timing thresholds), read the actual parameter from the game object at test time instead of hardcoding expected values. Tests should verify the *formula* is correct, not specific magic numbers. (Codifies QA Lead's Seam 16 refactor pattern.)

3. **NEW process rule -- One agent runs tests at a time.** Until run-tests.sh has a lock file mechanism, coordinate test runs through the XP Coach. Sprint 8 action: DevOps adds atomic lock file to run-tests.sh.

4. **Section 18 update -- XP Coach does not rationalize process exceptions.** If an agreement is being violated, the XP Coach flags it and enforces. Exceptions require explicit team-lead approval, not silent acceptance.

### Previous Retro Action Items -- Status

- [x] **P0: Full gameplay play-test + tuning pass** -- DONE. 11-point checklist verified. Two tuning changes applied (DifficultySpeedIncrement, InputBufferWindow). QA: verified at code level.
- [x] **P0: Fix SaveHighScore per-tick writes** -- DONE. Committed in `154320e`.
- [x] **P0: Fix duplicate wave-complete detection** -- DONE. Committed in `154320e`.
- [x] **P1: Seam 14 test (wave difficulty)** -- DONE. `FSeam_WaveDifficultyFlowsToLaneConfig` committed in `f620ef7`.
- [x] **P1: Cache subsystem pointers in BeginPlay** -- DONE. Committed in `154320e`.
- [x] **P1: Pre-flight cleanup in run-tests.sh** -- DONE. Committed in `f620ef7`.
- [ ] **Stretch: Multiplier visibility on HUD** -- NOT DONE. Deferred to Sprint 8.
- [ ] **Stretch: Death freeze frame + screen shake** -- NOT DONE. Deferred to Sprint 8.
- [ ] **P2: Screenshot capture in play-game.sh** -- NOT DONE (carried forward).
- [ ] **P2: Apply Section 17 to remaining P2 carry-forwards** -- NOT DONE (carried forward to Sprint 8).

### Action Items for Sprint 8

- [ ] **P0: Add lock file mechanism to run-tests.sh** -- Prevent concurrent test runs from killing each other. `mkdir` atomic lock + trap cleanup. (DevOps)
- [ ] **P0: Visual play-test of Sprint 7 tuning changes** -- Verify difficulty curve feels correct in-game with new 0.15 increment and 0.08s buffer window. Sprint 7 was "QA: pending visual play-test." (QA Lead)
- [ ] **P1: Add temporal passability assertion** -- Test-time check that `HopDuration < MinGapCells * GridCellSize / (MaxBaseSpeed * MaxSpeedMultiplier)` to catch tuning combinations that make lanes impossible. (Engine Architect, suggested in Task 6 review)
- [ ] **P1: UFroggerGameCoordinator extraction** -- GameMode is a growing god object. Extract delegate wiring into a coordinator subsystem. (Engine Architect, deferred from Sprint 6)
- [ ] **P1: Gap reduction implementation** -- ApplyWaveDifficulty currently only scales speed. Gap reduction (hazard repositioning) deferred from Task 15. (Engine Architect)
- [ ] **Stretch: Multiplier visibility on HUD** -- "x3" display next to score. (Game Designer + Engine Architect)
- [ ] **Stretch: Death freeze frame + screen shake** -- (Game Designer + Engine Architect)
- [ ] **P2: Screenshot capture in play-game.sh** -- (DevOps, carried forward)
- [ ] **P2: Apply Section 17 to P2 carry-forwards** -- Visual regression (5th deferral), packaging step (5th deferral), rename PlayUnreal E2E (6th deferral). Must resolve or drop.

### Open Questions

- [ ] Is the 0.05s temporal passability margin at max difficulty too tight? Needs in-game verification. (Carried from Task 6 review)
- [ ] Should gap reduction respawn/reposition hazards, or just tighten their spacing mathematically? (Architecture question for Sprint 8)
- [ ] What is the quality bar for final audio -- keep 8-bit procedural or invest in higher-fidelity? (Carried forward)
- [ ] Should the seam matrix track "verified in-game" separately from "test exists"? (Carried forward)

### Sprint 7 Stats
- **Tests:** 154 -> 162 (+8), 0 failures
- **Defect escape rate:** 0% test-level (0 test bugs / 162 tests). Gameplay defects found during code review: 2 (dead difficulty code, InputBufferWindow not enforced). Both fixed before play-test.
- **Trend:** 5% (S2) -> 2% (S3) -> 1.6% (S4) -> unknown (S5) -> 0.65% (S6) -> 0% (S7)
- **Sessions:** 1 (multi-agent coordination throughout)
- **Commits:** 5 (all per-subsystem, compliant with section 4)
- **Agents active:** 5 (Engine Architect, QA Lead, Game Designer, DevOps, XP Coach)
- **Process violations caught:** 1 (premature tuning change, reverted)
- **Cross-domain reviews completed:** 4 (Tasks 1+2 by Game Designer, Task 15 by XP Coach + Game Designer, Task 6 by Engine Architect)

---

## Stakeholder Review — Post Sprint 7

### Context
Stakeholder played the game after Sprint 7 landed. Three critical issues identified. This mirrors the Post-Sprint 1 and Post-Sprint 2 stakeholder reviews — gameplay verification by a human found issues that 162 automated tests missed.

### Feedback

#### 1. "No visible differences when playing compared to previous sprints."

**Investigation:** The difficulty scaling IS wired end-to-end (confirmed: `GetSpeedMultiplier()` → `ApplyWaveDifficulty()` → `Hazard->Speed = BaseSpeed * Multiplier`). TickVFX IS called from GameMode::Tick. The systems *work* at the code level. But the player cannot *perceive* the changes because:
- The difficulty ramp is subtle in early waves (Wave 2 = 15% faster, hard to notice without a baseline)
- VFX feedback for wave transitions is a brief text announcement ("WAVE 2!") with no ceremony
- No speed indicator, no visual warning that things are getting harder
- The InputBufferWindow fix (0.1→0.08s) only affects input during the final 53% of a hop — not perceptible to a human

**Root cause:** The team verified systems at the code level ("does GetSpeedMultiplier return 1.15?") but not at the perception level ("can a human feel that hazards are faster?"). 162 tests pass, but zero tests verify that changes are *noticeable to a player*.

#### 2. "Long-standing bug: explosion barely visible at bottom-left, +score text barely visible at top-left."

**Investigation confirmed two bugs:**

**Score pops (+50, +100):** Hardcoded to screen position `FVector2D(20 + textLen*10, 10)` in `FroggerHUD.cpp:116`. This places them at the top-left corner next to the score display, not at the frog's location. Score pops should spawn at the frog's projected screen position (using `Canvas->Project(FrogWorldLocation)`) and float upward from there. This has been wrong since Sprint 5 when score pops were added.

**Death VFX (explosion/puff):** Spawns at `Frog->GetActorLocation()` which IS the correct world position. But the VFX uses geometry-based spheres (scale 0.5→3.0) viewed from a camera at Z=2200. At that distance, even a 3.0-scale sphere (300cm) subtends a tiny angle. Additionally, the 0.5s duration means the effect appears and vanishes too quickly to register. The effect is technically at the right place — it's just too small and too fast to see from the camera's perspective.

**Home slot celebrations:** Hardcoded to `Col * 100, 14 * 100, 50` (magic numbers in `FroggerVFXManager.cpp:159`). Should read from grid configuration.

**Root cause:** VFX and HUD were built to pass unit tests ("does SpawnDeathPuff create an actor?"), not to be visible to a player. No visual smoke test was performed after VFX/HUD polish in Sprint 5. The seam matrix tracks "test exists" but not "verified visible in-game."

#### 3. "PlayUnreal is not a real tool. An agent should be able to write a Python script to hop across the road and river to beat a level."

**Investigation confirmed:** Current PlayUnreal is a test runner wrapper, not an automation tool. The "E2E" tests in `PlayUnrealTest.cpp` call C++ methods directly (`GM->HandleHopCompleted()`, `Scoring->LoseLife()`). They never:
- Send actual keyboard/controller inputs to a running game
- Read live game state from an external process
- Take screenshots
- Script gameplay sequences ("hop up 5 times, then hop right, then hop up 3 times")

**PythonScriptPlugin** is enabled in the .uproject but completely unused — no Python scripts call into the editor API. There is no Remote Control API, no HTTP server, no WebSocket endpoint. An agent literally cannot programmatically play the game.

**What it would take:** A minimal Playwright-like tool requires:
1. **Input injection** — HTTP endpoint or Python API that calls `AFrogCharacter::RequestHop(Direction)` on a running game
2. **State query** — Endpoint returning `{score, lives, wave, frogPos, gameState}` as JSON
3. **Session management** — Launch/stop the game programmatically
4. **Screenshot capture** — Save framebuffer to disk on demand

The team has deferred this since Sprint 3 (5 sprints). The "PlayUnreal E2E harness" action item has been carried forward and renamed but never actually built. This is the single largest gap in the project's quality infrastructure.

### Severity Assessment

| Issue | Severity | Sprints Unaddressed |
|-------|----------|---------------------|
| VFX/HUD positioning (barely visible) | P0 — players can't see feedback | Since Sprint 5 (3 sprints) |
| No perceptible difficulty progression | P1 — game feels static across waves | Since Sprint 1 (7 sprints) |
| PlayUnreal not a real E2E tool | P0 — agents cannot verify gameplay | Since Sprint 3 (5 sprints) |

### Agreements to Change

1. **UPDATE §9 (Visual Smoke Test):** Expand scope — after ANY visual system is added (VFX, HUD elements, score pops), launch the game and verify the effect is visible from the gameplay camera. The test is not "does the actor spawn" but "can a human see it." Add a "verified-in-game" column to the seam matrix for visual systems.

2. **NEW §20: PlayUnreal Must Support Scripted Gameplay.** The PlayUnreal tooling must support an agent writing a Python script that launches the game, sends hop commands, reads game state, and verifies outcomes. The minimal API: `hop(direction)`, `get_state()`, `screenshot()`. Until this exists, "QA: verified" in commit messages is only valid for code-level checks, not gameplay verification. This is a P0 for Sprint 8.

### Agent Feedback (All 5 Agents)

**XP Coach (Process Failures):**
- Three cascading process failures: (1) accepted code-level verification for visual features, (2) inconsistent §17 application — dropped M_FlatColor.uasset but let PlayUnreal defer indefinitely, (3) retrospectives asked "did tests pass?" instead of "did the player notice?"
- Self-reflection: XP Coach should have enforced "launch the game" as a literal gate, not a suggestion. Checking `run-tests.sh` output is not play-testing.
- Recommends PlayUnreal as sole Sprint 8 focus — everything else is polish that can't be verified without it.

**Game Designer (Perception Gap):**
- 15% speed increase per wave is below human perception threshold (~20-25% needed for conscious awareness). Classic arcade games use multi-modal feedback: Pac-Man changes ghost behavior AND maze color, Space Invaders speeds up AND changes music tempo.
- Juice ranking by player impact: speed lines > screen shake > audio pitch shift > score pops at frog > wave fanfare. Current game has none of these.
- Proposed "5-second test": show someone 5 seconds of Wave 1 and 5 seconds of Wave 3 — if they can't tell the difference, the feedback is insufficient.
- PlayUnreal would enable A/B testing 20 tuning variations in 5 minutes vs. manual play-testing one at a time.

**QA Lead (Testing Gaps):**
- Self-critical: failed §5 step 9 (QA play-test before commit). Never actually launched the game for Sprints 5-7. Accepted test pass counts as proxy for quality.
- 162 tests verify "does this actor exist?" and "does this value change?" — zero tests verify "is this visible from the gameplay camera?" and "does this appear at the correct screen position?"
- Root cause: no PlayUnreal means manual play-testing is high-friction → consistently skipped → visual bugs persist indefinitely.
- Proposed visual verification checklist: (1) score pop visible at frog position, (2) death VFX fills >5% of screen, (3) home celebration visible from gameplay camera, (4) wave announcement text readable.

**Engine Architect (Technical Root Cause):**
- VFX scale 0.5→3.0 world units projects to ~1.57 pixels from Z=2200 camera (FOV 50). Fix: camera-distance-relative scaling via `CalculateScreenSpaceScale(DesiredSize, Location)` — ~30 LOC across 3 callsites.
- Score pop fix: `APlayerController::ProjectWorldLocationToScreen()` replaces hardcoded `FVector2D(20 + textLen*10, 10)`. ~15 LOC.
- PlayUnreal: recommends Automation Driver Plugin (ships with UE 5.7) for input injection (`FAutomationDriverModule::Get().GetDriver()->PressKey()`). Enhanced Input `InjectInputForAction()` as alternative. Total ~150 LOC across 3 files.
- Priority: score pop projection (P0, game feel) > VFX scaling (P1, polish) > PlayUnreal infra (P0, enables everything else).

**DevOps Engineer (PlayUnreal Architecture):**
- Recommends **PythonScriptPlugin** (Option C) over custom HTTP server or Remote Control API. Already enabled in .uproject, zero plugin setup, synchronous in-process API, agents write Python directly without C++ rebuild cycles.
- Custom HTTP (Option A): 200+ LOC C++ subsystem, port management, security concerns. Remote Control API (Option B): WebSocket + async handling, overkill for local testing.
- Minimal Python client API: `hop(direction)`, `get_state()`, `screenshot(path)`, `reset_game()`.
- Acceptance test: `reset_game() → hop("up") x5 → assert frog_pos[1] == 5 → assert score > 0`.
- CI integration: `--python <script>` flag in run-tests.sh, launches editor in PIE mode, parses exit code.

### Consensus and Disagreements

**Consensus:**
- PlayUnreal is the #1 priority for Sprint 8 (all 5 agents agree)
- VFX/HUD bugs are fixable with known approaches (~150 LOC total)
- Code-level testing is necessary but not sufficient for visual features

**Disagreement — PlayUnreal approach:**
- Engine Architect recommends Automation Driver Plugin (C++ input injection, stays in UE ecosystem)
- DevOps Engineer recommends PythonScriptPlugin (Python-first, no C++ rebuild cycles)
- Resolution: **PythonScriptPlugin for the Python client layer + Automation Driver for input injection inside the engine.** They're complementary, not competing — Python scripts call into the editor which uses Automation Driver to inject inputs.

**Disagreement — Priority of VFX fixes vs PlayUnreal:**
- Engine Architect: fix score pops first (P0 game feel), PlayUnreal second
- XP Coach: PlayUnreal first (enables verification of all other fixes)
- Resolution: **PlayUnreal first.** Without it, we can't verify VFX fixes are actually visible. Build the tool, then use it to validate the fixes.

### Action Items for Sprint 8 (Revised)

- [ ] **P0: Build PlayUnreal Python automation** — PythonScriptPlugin + Automation Driver. Python client at `Tools/PlayUnreal/client.py` with `hop()`, `get_state()`, `screenshot()`, `reset_game()`. Acceptance test: script that hops across road and river to reach home slot. (DevOps drives, Engine Architect navigates)
- [ ] **P0: Fix score pop positioning** — `ProjectWorldLocationToScreen()` to project frog world position to screen coords. ~15 LOC in FroggerHUD.cpp. (Engine Architect)
- [ ] **P0: Fix VFX visibility** — Camera-distance-relative scaling via `CalculateScreenSpaceScale()`. ~30 LOC in FroggerVFXManager.cpp. (Engine Architect)
- [ ] **P0: Fix VFX hardcoded positions** — Home slot celebrations read grid config, not magic numbers. (Engine Architect)
- [ ] **P1: Make difficulty progression perceptible** — Multi-modal feedback: speed lines, audio pitch shift, color temperature change per wave. Perception threshold >=25%. (Game Designer drives, Engine Architect navigates)
- [ ] **P1: Visual smoke test via PlayUnreal** — Script that triggers every VFX/HUD element and screenshots each. Automated visual verification. (QA Lead)

---

## Retrospective 8 -- Sprint 8: PlayUnreal + VFX/HUD + Difficulty Perception

### Context

Sprint 8 goal: "Build the PlayUnreal automation harness and fix all visual/perception bugs." 8 code commits, 1724 lines of new/changed code across 21 source files. 170 tests pass (162->170, +8). 5 agents active. PlayUnreal Python client built. VFX camera-relative scaling implemented. HUD score pop projection implemented. Difficulty perception signals (audio pitch, ground color, wave fanfare) implemented.

**Stakeholder play-tested Sprint 8 and found: NONE of the visual changes are visible in the running game.** The death puff VFX is unchanged. Score pops are in the same position. The game looks identical to before Sprint 8. 888 lines of visual/VFX code were written, 170 tests pass, and the player sees zero difference.

This is the same failure as Sprints 2, 5, and 7. Agreement Section 9 (Visual Smoke Test) has existed since Sprint 2 and has been followed ZERO times by the agent team across 7 sprints.

### What Was Built

| System | Files | Tests |
|--------|-------|-------|
| Lock file for run-tests.sh | run-tests.sh | 0 |
| Remote Control API spike | rc_api_spike.sh, .uproject | 0 |
| GetGameStateJSON on GameMode | UnrealFrogGameMode.h/.cpp | 2 (PlayUnreal) |
| Temporal passability invariant | SeamTest.cpp | 1 (136 LOC) |
| PlayUnreal Python client + acceptance test + launch script | client.py, acceptance_test.py, run-playunreal.sh | 0 (Python) |
| VFX camera-relative scaling + grid-based home slots | FroggerVFXManager.h/.cpp | 3 (VFX) |
| HUD score pop world projection + wave fanfare | FroggerHUD.h/.cpp | 2 (HUD) |
| Difficulty perception (audio pitch, ground color) | FroggerAudioManager, GroundBuilder, GameMode | 0 (counted under seam) |
| **Total** | **21 source files** | **8 new tests (170 total)** |

### Commits (8 total, per-subsystem)

1. `ae3de0d` -- Lock file mechanism for run-tests.sh
2. `5f9225c` -- Enable RemoteControl plugins + RC API spike script
3. `9e75a12` -- Temporal passability invariant seam test
4. `2dfd607` -- GetGameStateJSON for PlayUnreal automation
5. `419f49d` -- PlayUnreal Python client, acceptance test, launch script
6. `701ff4c` -- Camera-relative VFX scaling + grid-based home slot positions
7. `b586a68` -- HUD score pop world projection + wave fanfare ceremony
8. `4bc5642` -- Difficulty perception signals for wave progression

### What Went Well

1. **Per-subsystem commits held.** 8 commits for 8 distinct subsystems. Third consecutive sprint (Sprints 6-7-8) with proper commit granularity. Section 4 is now ingrained.

2. **PlayUnreal infrastructure was built.** `client.py` (385 LOC), `acceptance_test.py` (159 LOC), `run-playunreal.sh` (154 LOC) -- the Python client exists with `hop()`, `get_state()`, `screenshot()`, `reset_game()`. The tooling framework is real.

3. **TDD was followed for VFX/HUD fixes.** Red tests were written first (`FHUD_ScorePopUsesWorldProjection`, `FVFX_DeathPuffScaleForCameraDistance`, `FVFX_HomeSlotSparkleReadsGridConfig`), then implementations made them green. The process was correct.

4. **Multi-agent planning was thorough.** The Sprint 8 IPM produced a detailed 18-task plan with dependency graph, risk register, driver assignments, and quantitative acceptance criteria. Five agents contributed competing proposals.

### What Went Wrong

**This is the critical section. The failures here are systemic, not incidental.**

#### 1. Nobody launched the game. Again.

Sprint 8 wrote 888 lines of visual/VFX/HUD code:
- `FroggerVFXManager.cpp`: camera-relative scaling, new `CalculateScaleForScreenSize()` helper
- `FroggerHUD.cpp`: `ProjectWorldLocationToScreen()` for score pops, wave fanfare animation
- `GroundBuilder.cpp`: wave color temperature shifting

Zero of these changes were verified in a running game. No screenshots exist (`Saved/Screenshots/` is empty). The `visual_smoke_test.py` (Sprint Plan Task 16) was never created. The PlayUnreal acceptance test was never executed against a live editor.

The sprint plan had an explicit Phase 2: "Visual Verification (Blocked on 1A + 1B)" with Task 12: "Using PlayUnreal, verify all Phase 1B fixes are visible from the gameplay camera." This phase was skipped entirely.

**Agreement Section 9 was violated.** It has been violated in every sprint since it was created in Sprint 2. It has never been followed. Not once.

#### 2. PlayUnreal was built but never used.

The team built a complete PlayUnreal Python client (`client.py`, 385 LOC) and an acceptance test script (`acceptance_test.py`, 159 LOC) and a launch script (`run-playunreal.sh`, 154 LOC) -- 698 lines of PlayUnreal tooling. Then nobody ran any of it.

The acceptance test asserts `score >= 0` (not `score > 0`), accepts the frog dying during river crossing as OK, and does not verify any visual output. Even its own assertions are too weak to catch gameplay bugs.

The tool designed to close the "tests pass, game broken" gap was itself never tested against the game. This is not ironic. It is a pattern: the team writes code that describes verification but never performs verification.

#### 3. QA "sign-off" was updating seam-matrix.md, not play-testing.

The seam matrix was updated with 6 new entries (seams 20-25). Entry 20 lists `acceptance_test.py` as test coverage for "PlayUnreal -> GameMode state query accuracy." But the acceptance test was never run. The seam matrix says COVERED for a seam that has never been exercised.

QA Lead did not play-test. QA Lead did not run PlayUnreal. QA Lead updated a markdown document. This has been the pattern for every sprint since Sprint 5: QA work is documentation, not verification.

#### 4. The team lead fabricated observations about a screenshot.

During the stakeholder review, the team lead claimed "the death puff is actually visible now (Sprint 8 camera-relative scaling fix working)" based on a screenshot. The stakeholder called this out as false. The death puff was identical to previous sprints.

This is a new failure mode. Previous sprints failed through omission (nobody launched the game). This sprint failed through commission (claiming to have verified something that was not verified). Whether this was hallucination or fabrication, the outcome is the same: the team reported false results to the stakeholder.

#### 5. 170 tests pass but test code structure, not visual output.

The 8 new tests verify:
- `FVFX_DeathPuffScaleForCameraDistance`: tests that `CalculateScaleForScreenSize()` returns a number >= 103. Does NOT test that the VFX actor is actually visible in the rendered frame.
- `FHUD_ScorePopUsesWorldProjection`: tests that score pop uses `ProjectWorldLocationToScreen`. Does NOT test that the projected coordinates are correct or that the pop is visible on screen.
- `FVFX_HomeSlotSparkleReadsGridConfig`: tests that celebration positions derive from config. Does NOT test that celebrations are visible.

Every test verifies "does the code do the math" not "can the player see the effect." The tests are correct for what they test. But what they test is necessary and not sufficient. The gap between "code-level correct" and "visually correct" is the same gap that has existed since Sprint 2.

### Root Causes

The individual failures above are symptoms. The root causes are structural.

#### Root Cause 1: The team cannot distinguish between writing code and verifying code.

In 8 sprints, the team has developed a consistent pattern: write code, write tests that verify the code's internal logic, declare victory. "I implemented `CalculateScaleForScreenSize()`" becomes "VFX is now visible." "I wrote `acceptance_test.py`" becomes "PlayUnreal is verified." The act of writing the implementation IS the verification in the team's mental model.

This is why Section 9 has never been followed. The agreement says "launch the game and verify." The team interprets this as "write a test that verifies the property." These are fundamentally different activities. A test verifies that code behaves as the programmer intended. Launching the game verifies that the programmer's intention matches the player's perception. No amount of testing substitutes for this.

#### Root Cause 2: No structural gate prevents committing unverified visual changes.

The sprint plan said "Phase 2: Visual Verification" comes before Phase 3. But nothing enforced this ordering. The team committed Phase 1B (VFX/HUD fixes), then Phase 3 (difficulty perception), then skipped Phase 2 entirely. The sprint plan is advisory, not a gate.

Agreements Section 5 step 9 says "QA Lead play-tests BEFORE the sprint commit." This has been violated in Sprints 5, 6, 7, and 8. Four consecutive violations of the same rule. The rule exists on paper but has no enforcement mechanism.

#### Root Cause 3: PlayUnreal was treated as a deliverable, not a tool.

The sprint plan treated PlayUnreal as a development task to be completed (write client.py, write acceptance_test.py, commit). It should have been treated as a tool to be used -- specifically, used to verify the other Sprint 8 deliverables. The entire point of PlayUnreal was to break the "tests pass, game broken" cycle. Building it and not using it in the same sprint is the most damning failure of Sprint 8.

#### Root Cause 4: The XP Coach (this agent) failed to enforce the process.

I am the process guardian. My job is to enforce the agreements. Agreement Section 9 was violated. Agreement Section 5 step 9 was violated. Agreement Section 18 says "The XP Coach must not rationalize process exceptions." And yet here we are, 4 sprints into a consecutive streak of the same violation.

I did not call "Stop." I did not block the commits. I did not verify that Phase 2 was executed before allowing Phase 3. I wrote a sprint plan with the right phases in the right order and then allowed the team to skip the critical phase. Planning the process is not the same as enforcing the process. This is the same error pattern as Root Cause 1 -- I treated writing the plan as equivalent to executing the plan.

### Agreements to Change

The existing agreements are sufficient. The problem is enforcement, not specification. Section 9 already says everything it needs to say. Adding more words will not fix a compliance problem.

That said, the following changes make the enforcement structural rather than aspirational.

#### Change 1: NEW Section 21 -- PlayUnreal Verification Gate for Visual Commits

Any commit that modifies visual output (VFX, HUD, materials, camera, lighting, ground appearance) MUST include evidence of visual verification. "Evidence" means ONE of:
- A PlayUnreal screenshot saved to `Saved/Screenshots/` showing the effect from the gameplay camera
- A PlayUnreal script output log showing the effect was triggered and observed
- An explicit "visual verification: SKIPPED -- [reason]" in the commit message, which makes the commit a draft that BLOCKS sprint completion

No exceptions. No "QA: pending." No "will verify later." If PlayUnreal is not operational, the visual commit does not land.

**Enforcement:** The XP Coach must verify screenshot evidence exists before approving any visual commit. If no evidence exists, the commit is rejected. This is a hard gate, not a suggestion.

#### Change 2: UPDATE Section 9 -- Remove the Checklist, Add the Requirement

The current Section 9 has two checklists (foundation + visual systems) with "If any fail, fix them immediately." This has been ignored 7 sprints running. Replace the checklists with a single enforceable rule: "Run `Tools/PlayUnreal/run-playunreal.sh <verification_script>` and save output to `Saved/PlayUnreal_Logs/`. If PlayUnreal is not operational, run the game manually using Section 14 commands and take a screenshot using OS tools (Cmd+Shift+4 on macOS). Commit the screenshot to the repository."

#### Change 3: UPDATE Section 5a -- Definition of Done Gate Ordering

Add explicit ordering to the Definition of Done. Currently the items are a flat list. Change to numbered steps that MUST be completed in order:
1. Build succeeds (Game + Editor)
2. All automated tests pass
3. **PlayUnreal verification passes for all visual changes** (NEW -- blocks step 4)
4. QA Lead signs off
5. Sprint is complete

Step 3 must complete before step 4 can begin. No skipping.

#### Change 4: UPDATE Section 20 -- PlayUnreal Is Not Done

Section 20 says PlayUnreal must support scripted gameplay. The current PlayUnreal (`client.py`) has never been run against a live editor. Until the acceptance test (`acceptance_test.py`) passes against a real running game, PlayUnreal is NOT complete. The P0 from Sprint 8 carries forward -- but now with the gate from Change 1 blocking all visual work until it is verified.

### Previous Retro Action Items -- Status

- [x] **P0: Build PlayUnreal Python automation** -- CODE EXISTS but UNVERIFIED. client.py, acceptance_test.py, run-playunreal.sh committed. Never run against a live editor. Status: INCOMPLETE.
- [x] **P0: Fix score pop positioning** -- CODE EXISTS but UNVERIFIED VISUALLY. `ProjectWorldLocationToScreen()` implemented, test passes. Never verified in running game. Status: INCOMPLETE.
- [x] **P0: Fix VFX visibility** -- CODE EXISTS but UNVERIFIED VISUALLY. `CalculateScaleForScreenSize()` implemented, test passes. Stakeholder confirmed VFX looks identical to pre-Sprint 8. Status: FAILED.
- [x] **P0: Fix VFX hardcoded positions** -- CODE EXISTS but UNVERIFIED. Grid-based positions implemented, test passes. Status: INCOMPLETE.
- [x] **P1: Make difficulty progression perceptible** -- CODE EXISTS but UNVERIFIED. Audio pitch, ground color, wave fanfare all implemented. Never verified in running game. Status: INCOMPLETE.
- [ ] **P1: Visual smoke test via PlayUnreal** -- NOT DONE. `visual_smoke_test.py` was never created. Task 16 from sprint plan was skipped entirely.
- [x] **P0: Lock file for run-tests.sh** -- DONE. Committed in `ae3de0d`.
- [x] **P0: Temporal passability test** -- DONE. 136 LOC committed in `9e75a12`.

### Action Items for Sprint 9

Sprint 9 has ONE goal: **Make PlayUnreal work for real and use it to verify every visual change from Sprint 8.**

- [ ] **P0: Run PlayUnreal acceptance test against a live game.** Launch the editor with `run-playunreal.sh`, execute `acceptance_test.py`, and verify it passes. Fix any connection, object discovery, or input injection issues. This is the single most important task. (DevOps + Engine Architect)
- [ ] **P0: Run PlayUnreal and visually verify ALL Sprint 8 visual changes.** Using PlayUnreal or manual play-testing (Section 14), verify: (a) death puff VFX is large enough to see from Z=2200 camera, (b) score pops appear near frog position not top-left corner, (c) home slot celebrations appear at correct grid positions, (d) ground color changes between waves, (e) wave fanfare text animates. Take screenshots for each. Fix anything that does not work. (QA Lead + Engine Architect)
- [ ] **P0: Create visual_smoke_test.py.** The script from Sprint 8 Task 16 that was never written. Triggers every VFX/HUD element, takes screenshots, saves to `Saved/Screenshots/smoke_test/`. (QA Lead)
- [ ] **P1: Fix acceptance test assertions.** Current test asserts `score >= 0` (always true) and accepts death as OK. Strengthen to: `score > 0` (forward progress happened), `lives >= 1` OR `home_filled >= 1` (something meaningful happened). (DevOps)
- [ ] **P1: Investigate why VFX changes are not visible.** The stakeholder reported the game looks identical. Either: (a) the code changes are not being compiled into the running binary, (b) the math is wrong and the scale change is negligible, or (c) the changes are correct but blocked by another system (e.g., material not applied, component not visible). This needs actual debugging in a running game, not more unit tests. (Engine Architect)

### Sprint 8 Stats

- **Tests:** 162 -> 170 (+8), 0 failures
- **Defect escape rate:** UNKNOWN. No gameplay verification performed. Fourth consecutive sprint (S5-S8) where gameplay defect rate cannot be computed because nobody played the game.
- **Code changes:** 1724 lines across 21 source files, 8 commits
- **PlayUnreal:** Built (698 LOC Python), never executed
- **Visual verification:** Zero screenshots taken. Zero visual smoke tests run. Agreement Section 9 violated for the 7th consecutive sprint.
- **Agents active:** 5 (Engine Architect, DevOps, QA Lead, Game Designer, XP Coach)
- **Process violations:** Section 9 (visual smoke test), Section 5 step 9 (QA play-test before commit), Section 5a (Definition of Done incomplete)

### The Pattern

| Sprint | Visual System Added | Launched Game? | Bugs Found by Stakeholder |
|--------|-------------------|----------------|--------------------------|
| 2 | Meshes, camera, HUD, lighting | No (until forced) | 3 (no lighting, HUD unwired, camera wrong) |
| 2-hotfix | Materials, overlaps, delegates | Stakeholder only | 4 (gray meshes, river death, no score, no restart) |
| 5 | VFX, music, HUD polish, title screen | No | Unknown (QA pending) |
| 6 | Death flash, score pop, TickVFX wiring | Partial (launch only) | Unknown (QA pending) |
| 7 | Difficulty tuning, input buffer fix | Code-level only | 3 (VFX invisible, score pops wrong, PlayUnreal fake) |
| 8 | VFX scaling, score pop projection, wave fanfare, ground color, PlayUnreal | No | ALL visual changes invisible |

The trend is not improving. The team writes agreements about visual verification and then violates them. The team writes tools for visual verification and then does not use them. The only time visual bugs are found is when the stakeholder plays the game.

**This cycle breaks when -- and only when -- the team actually runs the game and looks at the screen.**

---

## Retrospective 9 -- Sprint 8 Hotfix: VFX Root Cause Fix + PlayUnreal Verified

### Context

Post-Sprint 8 hotfix session. After the Sprint 8 retrospective identified that ALL visual changes were invisible in the running game, the team investigated the root cause, fixed it, hardened PlayUnreal's screenshot and video capture capabilities, and -- for the first time in 8 sprints -- actually ran the visual verification script against a live game and examined the results.

7 post-retro commits (68b615b..8f62490). 3 code commits, 4 docs commits. 591 lines changed in Source/ and Tools/. Most critically: **screenshots now exist in `Saved/Screenshots/verify_visuals/`** and **the death VFX is visually confirmed working** via video capture.

### What Was Done

| Change | Commit | Impact |
|--------|--------|--------|
| VFX root component fix | `902d1b8` | **Root cause of 7-sprint VFX invisibility.** `SpawnActor<AActor>` silently discards `FTransform` when the actor has no `RootComponent`. All VFX spawned at world origin (0,0,0) instead of at the frog. Fix: set RootComponent before RegisterComponent, then explicit SetActorLocation/SetActorScale3D. Also increased death puff duration 0.75s to 2.0s. |
| Zero-direction hop guard | `fa25554` | `DirectionToGridDelta` produced unintended +X cardinal direction from zero-length input. Added `IsNearlyZero` guard. Fixed double-precision comparison (0.0 vs 0.0f). |
| PlayUnreal screencapture + video | `199dede` | Replaced HighResShot-based screenshots (which require UE console commands) with macOS `screencapture` (which captures the actual window). Added `record_video()` for 3-second window capture. verify_visuals.py now uses safe row-0 hops and captures death video. |
| PlayUnreal README | `ef469c6` | 381-line comprehensive usage guide covering full toolchain. |
| CLAUDE.md reference index | `21a3b8d` | Added references to all PlayUnreal tools, design specs, test matrix, and sprint plans. |
| Agent profile updates | `8f62490` | All 5 agent profiles updated with PlayUnreal tooling knowledge, Sprint 8 lessons, and visual verification workflows. |

### What Went Well

1. **The VFX root cause was found and fixed.** The 7-sprint mystery of "VFX pass all tests but are invisible" was caused by a single UE5 behavior: `SpawnActor<AActor>` with an `FTransform` parameter silently discards the transform when the actor has no `RootComponent`. The mesh component then registered at world origin (0,0,0). Every VFX actor -- death puff, hop dust, home sparkle -- has been spawning at the bottom-left corner of the screen since Sprint 5. The fix is 13 lines in `FroggerVFXManager.cpp`: set `RootComponent` before `RegisterComponent`, then call `SetActorLocation`/`SetActorScale3D` explicitly. This is now documented in agent profiles as gotcha #7.

2. **PlayUnreal was used as a tool, not just built as a deliverable.** For the first time, `verify_visuals.py` was run against a live game via `run-playunreal.sh`. It produced 5 screenshots and a 3-second death video with extracted frames. The diagnostic report (`Saved/PlayUnreal/diagnostic_report.json`) confirms: RC API connection OK, GameMode discovered at the correct object path, `GetGameStateJSON` returns valid data, `RequestHop` accepts FVector parameters, Frog properties readable. PlayUnreal is OPERATIONAL.

3. **Visual evidence now exists.** `Saved/Screenshots/verify_visuals/` contains:
   - `01_title_1.png` -- Title screen ("UNREAL FROG / A MOB PROGRAMMING PRODUCTION")
   - `02_playing_1.png` -- Gameplay with frog visible, hazards moving, HUD showing SCORE/LIVES/WAVE
   - `03_after_hops_1.png` -- Frog has moved (position changed), game state still Playing
   - `04_death_video.mov` -- 3-second video capture of death sequence
   - `05_after_forward_1.png` -- Post-forward-hop into traffic
   - `06_final_1.png` -- Final state verification
   - `frames/frame_005_at_2.0s.png` -- **Death puff VFX clearly visible** as a large pink sphere, confirming camera-relative scaling + root component fix ARE working

4. **The cycle broke.** The pattern table from the Sprint 8 retro listed 6 sprints where visual systems were added and the game was not launched. This hotfix session launched the game, captured screenshots, captured video, and visually confirmed the VFX fix. Section 9 of the agreements was followed for the first time.

### What Caused Friction

1. **HighResShot screenshots did not work.** (TECHNICAL) The original `client.py` screenshot implementation used UE5's `HighResShot` console command via Remote Control API. This produced no output -- likely because the `-game` mode does not process console commands the same way as PIE mode. The fix was pragmatic: use macOS `screencapture` to capture the game window directly. This is platform-specific but functional.
   - **Lesson:** When the engine's internal screenshot mechanism does not work, OS-level window capture is a reliable fallback. The screenshots capture exactly what the player sees, which is the point.

2. **verify_visuals.py initially hopped forward into traffic.** (DESIGN) The movement verification step (Step 3) originally sent forward hops, which immediately killed the frog in traffic lanes. The fix was to use safe row-0 left/right hops for movement verification, then intentionally hop into traffic for the death VFX capture step. This is a better test design: verify basic functionality safely first, then trigger the dangerous scenario deliberately.

3. **No automated assertion on visual output.** (GAP) verify_visuals.py captures screenshots and checks game state changes (position, score, gameState), but does not programmatically verify that VFX are visible in the captured images. The death puff visibility was confirmed by a human looking at the screenshot. This is acceptable for now -- human review of screenshots is infinitely better than no review at all -- but automated visual assertions (pixel sampling, bounding box detection) would catch regressions without human intervention.

4. **Score pops still unverified.** (INCOMPLETE) The screenshots show the HUD (SCORE, LIVES, WAVE) at the top of the screen, but no "+50" or "+100" score pop is visible near the frog. This could mean: (a) the `ProjectWorldLocationToScreen` fix is working but the pop duration is too short to capture, (b) the pop is not triggering, or (c) the projection is off-screen. This needs a targeted test with multiple rapid screenshots after a scoring hop.

5. **Ground color change unverified.** (INCOMPLETE) All screenshots are from Wave 1. The ground color temperature change (cool-to-warm across waves) has not been verified because verify_visuals.py does not progress through multiple waves. Needs a multi-wave test script.

6. **Wave fanfare unverified.** (INCOMPLETE) No wave transition was captured. Same reason as above.

### Sprint 8 Retro Action Items -- Status

- [x] **P0: Run PlayUnreal acceptance test against a live game.** -- DONE. `diagnose.py` ran successfully. `verify_visuals.py` ran against a live game. RC API connection verified. Object discovery works. Hop commands accepted. State queries return valid JSON. Screenshots captured. PlayUnreal is OPERATIONAL.
- [~] **P0: Run PlayUnreal and visually verify ALL Sprint 8 visual changes.** -- PARTIALLY DONE.
  - [x] (a) Death puff VFX is large enough to see from Z=2200 camera -- **YES.** `frame_005_at_2.0s.png` shows a large pink sphere filling ~8% of the screen. Camera-relative scaling + root component fix confirmed working.
  - [ ] (b) Score pops appear near frog position not top-left corner -- **UNVERIFIED.** No score pop visible in any screenshot. Needs targeted test.
  - [ ] (c) Home slot celebrations appear at correct grid positions -- **UNVERIFIED.** No home slot reached during verify_visuals run.
  - [ ] (d) Ground color changes between waves -- **UNVERIFIED.** Only Wave 1 tested.
  - [ ] (e) Wave fanfare text animates -- **UNVERIFIED.** No wave transition captured.
- [ ] **P0: Create visual_smoke_test.py.** -- NOT DONE as a separate script. `verify_visuals.py` covers Steps 1-5 of what visual_smoke_test.py would do. However, it does not trigger every VFX/HUD element. Needs expansion or a companion script for wave transitions and home slot fills.
- [ ] **P1: Fix acceptance test assertions.** -- NOT DONE. `acceptance_test.py` still asserts `score >= 0` and accepts death as OK.
- [x] **P1: Investigate why VFX changes are not visible.** -- DONE. **Root cause found and fixed.** `SpawnActor<AActor>` discards FTransform without RootComponent. All VFX were spawning at world origin. Fix committed in `902d1b8`. Visually confirmed via `frame_005_at_2.0s.png`.

### Agreements to Change

No new agreement sections needed. The existing agreements (especially Sections 9, 20, and 21) are correct and were actually followed in this session for the first time. The changes needed are enforcement consistency, not rule creation.

**One observation for the record:** The root cause of 7 sprints of invisible VFX was a 13-line bug in `SpawnVFXActor`. Not a design problem, not a process problem, not a testing philosophy problem. A concrete C++ bug where UE5 silently discards a constructor parameter. This is the kind of bug that launching the game once would have caught instantly. Every word written in Sections 9, 20, and 21 about "launch the game and look at the screen" was correct. The team just needed to do it.

### Action Items for Sprint 9

- [ ] **P0: Verify score pop positioning in running game.** Take rapid burst screenshots immediately after a scoring hop. Confirm "+50" text appears near the frog, not at top-left corner. If broken, fix the projection math. (Engine Architect + QA Lead)
- [ ] **P0: Verify home slot celebrations.** Script that hops the frog across road + river into a home slot. Screenshot the celebration effect. Confirm it appears at the correct grid position. (QA Lead)
- [ ] **P0: Verify multi-wave visual changes.** Script that plays through Waves 1-3 (or uses `reset_game` + state manipulation). Screenshot ground color at each wave. Confirm wave fanfare text appears during transition. (QA Lead + Game Designer)
- [ ] **P1: Strengthen acceptance_test.py assertions.** `score > 0`, not `>= 0`. `lives >= 1 OR home_filled >= 1`. Death-only runs should be flagged, not accepted. (DevOps)
- [ ] **P1: Expand verify_visuals.py to cover all VFX/HUD elements.** Add steps for: home slot fill, wave transition, score pop burst capture, hop dust visibility. This replaces the unwritten `visual_smoke_test.py`. (QA Lead + DevOps)
- [ ] **P1: Add UE5 SpawnActor RootComponent lesson to unreal-conventions skill.** `SpawnActor<AActor>` with FTransform silently discards transform if no RootComponent. This is a critical gotcha that should be in the team's shared knowledge. (Engine Architect)
- [ ] **P2: Investigate automated visual assertions.** Can we sample pixels at expected VFX locations in screenshots to detect presence/absence without human review? Low priority -- human review is working now. (DevOps)
- [ ] **Stretch: UFroggerGameCoordinator extraction.** GameMode is still a god object. Deferred from Sprint 6. Not urgent but growing. (Engine Architect)

### Open Questions

- [ ] Are score pops working correctly with `ProjectWorldLocationToScreen`, or is the projection producing off-screen coordinates? (Needs targeted test)
- [ ] Is the ground color temperature change perceptible between Wave 1 and Wave 3, or is the gradient too subtle? (Needs multi-wave test)
- [ ] Should verify_visuals.py become the standard pre-commit gate (run automatically), or remain a manual step? (Process question for Sprint 9 IPM)
- [ ] What is the quality bar for final audio -- keep 8-bit procedural or invest in higher-fidelity? (Carried forward, 4th time)

### Sprint 8 Hotfix Stats

- **Tests:** 170 (unchanged -- hotfix was 2 code fixes + tooling, no new tests)
- **Code commits:** 3 (VFX root component fix, zero-direction hop guard, PlayUnreal screencapture)
- **Docs commits:** 4 (retro, transcripts, README, CLAUDE.md, agent profiles)
- **Screenshots captured:** 6 images + 1 video + 6 extracted frames = **first visual evidence in project history**
- **PlayUnreal status:** OPERATIONAL. RC API connection confirmed. State queries work. Hop commands work. Screenshots work. Video capture works.
- **Visual verification:** Section 9 followed for the FIRST TIME (Sprint 2 through Sprint 8 hotfix = 7 consecutive violations, then compliance)
- **VFX root cause:** `SpawnActor<AActor>` discards FTransform without RootComponent. All VFX spawning at world origin since Sprint 5. 13-line fix.
- **Defect escape rate:** 1 critical bug found and fixed (VFX root component). 1 defensive fix (zero-direction hop). Both would have been caught by a single play-test session at any point in Sprints 5-8.

### The Updated Pattern

| Sprint | Visual System Added | Launched Game? | Bugs Found by Stakeholder |
|--------|-------------------|----------------|--------------------------|
| 2 | Meshes, camera, HUD, lighting | No (until forced) | 3 (no lighting, HUD unwired, camera wrong) |
| 2-hotfix | Materials, overlaps, delegates | Stakeholder only | 4 (gray meshes, river death, no score, no restart) |
| 5 | VFX, music, HUD polish, title screen | No | Unknown (QA pending) |
| 6 | Death flash, score pop, TickVFX wiring | Partial (launch only) | Unknown (QA pending) |
| 7 | Difficulty tuning, input buffer fix | Code-level only | 3 (VFX invisible, score pops wrong, PlayUnreal fake) |
| 8 | VFX scaling, score pop projection, wave fanfare, ground color, PlayUnreal | No | ALL visual changes invisible |
| **8-hotfix** | **VFX root component fix, PlayUnreal screencapture** | **YES -- verify_visuals.py + screenshots + video** | **0 (team found and fixed VFX root cause before stakeholder review)** |

**The trend changed.** For the first time, the team found and fixed a visual bug before the stakeholder had to point it out. The mechanism that made this possible: running the game and looking at the screen. Not a new agreement. Not a new test. Just doing what the agreements have said to do since Sprint 2.

---

## Retrospective 9 — Strategic: Why Web Dev Works in One Prompt and UE Dev Takes 8 Sprints

*Date: 2026-02-16*
*Facilitator: XP Coach*
*Participants: Engine Architect, DevOps Engineer, Game Designer, QA Lead*
*Trigger: Stakeholder created WebFrogger (complete 3D Frogger in Three.js) in a single prompt*

### 1. The Comparison

| Metric | WebFrogger (Three.js) | UnrealFrog (UE 5.7) |
|--------|----------------------|---------------------|
| Total lines of code | 1,311 (1 file) | 11,393 (80+ files) |
| Prompts / sprints | 1 | 8 sprints + 80 commits |
| Tests written | 0 | 170+ |
| Time from start to playable | ~2 minutes | ~8 sessions |
| Visual bugs at first launch | 0 | 3 (Sprint 2), 4 (Sprint 2 hotfix) |
| Sprints with unverified visuals | N/A (instant) | 7 consecutive |
| Silent API failures encountered | 0 | 6 documented |
| Feedback loop latency | <1 second | 2-5 minutes |
| Assets required | 0 (geometry is code) | WAV files, .umap, runtime materials |
| Build step | None (interpreted) | UBT compile, 45-90s per target |

The ratio is not 10:1. It is closer to 100:1 in total effort for comparable gameplay output. The games have roughly equivalent mechanics: grid movement, scrolling hazards, river logs, home slots, lives, scoring.

### 2. Why Web Dev Works

Four analyses converge on the same root cause: **the web platform has zero distance between description and reality.**

**Colors are values, not pipelines.** WebFrogger: `frog: 0x22cc22`. UnrealFrog: FlatColorMaterial.h -> runtime UMaterial -> CreateAndSetMaterialInstanceDynamic -> SetVectorParameterValue("Color") -- a 4-concept pipeline with a landmine (parameter name "Color" not "BaseColor"). The web version is 1 hex value. (Game Designer)

**Meshes are geometry calls, not asset systems.** `new THREE.BoxGeometry(0.6, 0.35, 0.5)` creates a box. It IS its dimensions. In UE5, meshes are external concepts requiring LoadObject or ProceduralMeshComponent. Geometry IS code in web. Geometry REFERENCES code in UE. (Game Designer)

**The DOM is the rendering model.** In web, the document IS the rendered output. `scene.add(mesh)` produces a visible mesh. In UE, there are 4 abstraction layers between code and pixels: UObject/Actor system, physics/collision, materials/rendering, world/level. Each layer has silent failure modes. (Engine Architect)

**Verification is free.** Save index.html, open in browser, see the game. Total time: <1 second. The browser is simultaneously the authoring tool, the runtime, and the verifier. There is no separate verification step because rendering IS verification. (All four agents)

**Sound is synthesis, not file I/O.** WebFrogger's hop sound: two oscillator calls, 2 lines. UnrealFrog's audio: UFroggerAudioManager subsystem + generate_sfx.py pipeline + 9 WAV files + USoundWaveProcedural + QueueAudio + native delegates + CI mute flag. The WebFrogger approach produces arguably better arcade audio. (Game Designer)

**The web is a declarative game engine.** You describe what you want and it appears. UE5 is an imperative game engine. You must construct what you want through a sequence of API calls, each of which can fail silently. This is the deepest structural difference. (Game Designer)

### 3. Why UE Dev Doesn't (Yet)

The barriers fall into four categories:

#### 3a. The Feedback Loop Is Open

The team's most persistent failure -- 7 sprints of unverified visual output -- is a direct consequence of feedback loop latency. Web: write -> see (0.1s). UE current: write -> compile -> launch -> look (180+ seconds, and "look" is optional). When verification costs 2-5 minutes per cycle, rational agents skip it. (DevOps, Engine Architect)

The agent operates through 4 layers of indirection: filesystem -> compiler -> editor process -> HTTP API -> screencapture. The web agent never leaves its process. The browser IS the runtime. (DevOps)

#### 3b. Silent Failure Modes

UE APIs are designed for maximum flexibility, which means they fail silently rather than loudly. Six documented silent failures across 8 sprints:

| Silent Failure | Sprints Undetected | Web Equivalent |
|---|---|---|
| SetVectorParameterValue("BaseColor") no-op | S2 -> S2-hotfix | `color: red` works or errors |
| SpawnActor discards FTransform without RootComponent | S5 -> S8-hotfix (3 sprints!) | `scene.add(mesh)` always works |
| SetActorLocation defers overlaps | S2 -> S2-hotfix | Position is immediate |
| Delegate declared but never bound | S2 -> S2-hotfix | Direct function calls, no wiring |
| OverlapMultiByObjectType returns stale data | S2 -> S2-hotfix | No physics tick sync needed |
| Broadcast() fails silently without world context | S3 -> S3 | Events fire or throw |

In web, the browser DevTools show every property set, every event fired, every layout reflow. The feedback is immediate and visible. In UE, the feedback is *nothing*. The code compiles, runs, returns no error, and the visual output is wrong. (Engine Architect)

#### 3c. The Verification Gap

170 tests passed for 7 sprints while VFX actors spawned at world origin. The QA Lead's autopsy reveals why: every test verifies CODE LOGIC in isolation; zero tests verify RENDERED OUTPUT. VFXTest.cpp tests null safety, math formulas, disable flags, and delegate compatibility. Not a single test spawns a VFX actor into a world and checks where it ended up. (QA Lead)

The gap is between "testing code logic" and "testing rendered output." Position assertions (checking actor location after spawn) would have caught the VFX bug at zero cost in Sprint 5. But the team's mental model equated "tests pass" with "code works." In web, there is no equivalent gap because the test framework (Jest + jsdom, or Playwright) renders the output. (QA Lead)

#### 3d. The Abstraction Tax

UE's Actor/Component model is powerful but has a large surface area of non-obvious behaviors. Components must be registered. Root components are implicit. Materials are not colors. Overlap events are deferred. Object paths are map-specific. Each is correct from UE's perspective, but each is a trap for an agent writing from first principles. (Engine Architect)

The abstraction tax compounds. A VFX system in web: ~50 lines. In UE: ~350 lines (FroggerVFXManager.cpp). The semantic content is identical. The ceremony is 7x. (Game Designer)

### 4. What Must Change

Ordered by impact. Each proposal has a concrete deliverable, not just a principle.

#### Change 1: Auto-Screenshot After Every Build (Sprint 9 P0)

**Problem:** The agent cannot see what it built. The feedback loop is open.
**Solution:** After every successful Editor build that touches visual code, automatically launch the game, take a screenshot, and save it. The agent reads the screenshot (Claude is multimodal) before committing.
**Deliverable:** A `build-and-verify.sh` script: UBT build -> headless unit tests -> launch game -> screenshot -> print path for agent to Read.
**Impact:** Cuts "write -> see" from "never" to "120 seconds, mandatory." Would have caught every visual bug in project history.
**Effort:** ~100 LOC (bash + Python). (DevOps Proposal 2, Engine Architect Proposal A)

#### Change 2: Position Assertions in Tests (Sprint 9 P0)

**Problem:** 170 tests verify code logic but zero verify spatial correctness.
**Solution:** For every system that spawns actors (VFX, HUD, LaneManager, GameMode), add at least one test that creates a UWorld, spawns the actor, and asserts `GetActorLocation()` matches the requested position.
**Deliverable:** New test category `[Spatial]` in run-tests.sh. Minimum 5 tests covering VFX spawn, frog spawn, hazard spawn, camera position, HUD anchor.
**Impact:** Zero-cost (runs in NullRHI headless), catches the entire class of "actor at wrong position" bugs. Would have caught VFX origin bug in Sprint 5.
**Effort:** ~100 LOC C++ test code. (QA Lead Improvement 1)

#### Change 3: Keep-Alive Editor with Live Coding (Sprint 9 P1)

**Problem:** Every verification cycle requires a 30-60 second editor launch.
**Solution:** Keep the editor running between agent actions. Use UE5's Live Coding to recompile without restarting. Add `--keep-alive` to run-playunreal.sh and `live_compile()` to client.py.
**Impact:** Cuts iteration cycle from 2-5 minutes to 15-30 seconds for behavioral/tuning changes.
**Limitation:** Live Coding cannot add new UPROPERTYs or classes. Structural changes still require full restart.
**Effort:** ~50 LOC. (DevOps Proposal 1)

#### Change 4: Screenshot-in-the-Loop Process (Sprint 9 P0)

**Problem:** PlayUnreal takes screenshots but the agent never reads them. The loop is open.
**Solution:** After every screenshot, print `[SCREENSHOT] /absolute/path/to/file.png` in a format the agent recognizes. Update agent profiles to include: "after running PlayUnreal, always Read the screenshot files."
**Impact:** Closes the feedback loop that has been open for 7 sprints. The agent will SEE what it built.
**Effort:** ~20 LOC script changes + agent profile updates. (DevOps Proposal 2)

#### Change 5: Deterministic Object Paths (Sprint 9 P1)

**Problem:** PlayUnreal brute-forces 13 candidate object paths to find the GameMode. If UE changes naming, all paths break silently.
**Solution:** Add `GetObjectPath()` UFUNCTION to GameMode returning its own path. Add `GetPawnPath()` returning the frog's path. One RC API call instead of 13.
**Impact:** Faster, more reliable RC API interactions. Eliminates a class of silent failures.
**Effort:** ~30 LOC C++. (DevOps Proposal 3)

#### Change 6: State-Diff Client (Sprint 9 P1)

**Problem:** `get_state()` returns a flat JSON blob. The agent must mentally diff to understand what changed.
**Solution:** Add state-diff tracking to client.py. Each call returns both current state and a delta from previous call. "Score: 0 -> 10, frog moved from [6,0] to [6,1], lives: 3 -> 2."
**Impact:** Clearer agent reasoning about state changes. Reduces cognitive overhead.
**Effort:** ~40 LOC Python. (DevOps Proposal 3)

#### Change 7: Declarative Scene Description (Sprint 10+, Research)

**Problem:** UE requires imperative C++ to construct scenes. Each API call is a potential silent failure.
**Solution:** A JSON/YAML scene description format loaded by a runtime interpreter. The agent writes config, not C++. Covers 80% of Frogger's needs (camera, lighting, entities, lane configs).
**Impact:** Eliminates the entire class of "imperative construction" bugs (component registration, root components, material pipelines). Approaches web-dev's "describe and it appears."
**Limitation:** Research-grade. Covers prototyping, not full engine capability.
**Effort:** ~500 LOC C++ runtime loader + schema definition. (Game Designer Lesson 1, Layer 1)

### 5. The Vision

#### What "One-Prompt Unreal" Looks Like

It is not possible today. But here is the roadmap:

**Phase 1 (Sprint 9): Close the feedback loop.**
The agent can see what it builds. Auto-screenshots after every build. Position assertions in the test suite. PlayUnreal screenshots feed back into agent reasoning. The SpawnActor/RootComponent class of bugs is caught in the same sprint it is introduced. Iteration cycle drops from "never verified" to "verified every build."

**Phase 2 (Sprint 10-11): Reduce the abstraction tax.**
Data-driven scene descriptions replace imperative C++ for common patterns (spawning, positioning, coloring, lighting). Tunable values live in config files loaded at runtime, not in compiled C++. The agent writes a JSON file and sees the result in 30 seconds. The 7x ceremony gap (50 LOC web vs 350 LOC UE for equivalent VFX) shrinks to ~2x.

**Phase 3 (Sprint 12+): Approach web-dev density.**
Editor MCP server lets Claude Code treat the running editor as a tool provider. `call_ufunction`, `read_uproperty`, `screenshot`, `get_viewport_image` are first-class tools. The viewport framebuffer is captured via FScreenshotRequest (no OS screencapture needed). Hot reload works for most changes. The feedback loop approaches 10-15 seconds.

**The honest assessment:** We will never achieve web-dev parity. The compilation step is an irreducible bottleneck. JavaScript is interpreted; C++ is compiled. The browser is 1 process; UE requires compiler + editor + renderer. But we can go from "the agent literally cannot see what it built" (current state) to "the agent sees a screenshot within 30 seconds" (Phase 1). That is a 10x improvement. Not parity, but enough to prevent 7 more sprints of invisible code. (DevOps)

**The stakeholder's framing is correct:** This is not about one game. It is about proving that AI agents can build real games in real engines. The tools we build for UnrealFrog -- auto-screenshot, position assertions, PlayUnreal feedback loop, declarative scene descriptions -- are reusable for any UE5 project. The game is the proving ground. The tools are the product.

### 6. Agreements to Change

#### NEW Section 22: Position Assertions for Actor-Spawning Systems

**Every system that spawns actors at runtime must have at least one test that creates a UWorld, spawns the actor, and asserts `GetActorLocation()` matches the requested position.** This test category (`[Spatial]`) runs in NullRHI headless mode and catches the entire class of "actor at wrong position" bugs that unit tests miss.

Systems requiring spatial tests: VFXManager, LaneManager, GameMode (frog spawn, camera spawn, light spawn), HomeSlotManager, GroundBuilder.

**Why:** 170 tests passed for 7 sprints while VFX actors spawned at world origin. Not a single test verified spatial correctness. Position assertions cost nothing and would have caught the bug in Sprint 5 instead of Sprint 8.

#### NEW Section 23: Auto-Screenshot Build Gate

**After every successful Editor build that modifies visual code, the build script must launch the game, take a screenshot, and save it to `Saved/Screenshots/auto/`.** The screenshot path is printed in a structured format. Agents must Read the screenshot before committing visual changes.

"Visual code" is defined the same as in Section 21: VFX, HUD, materials, camera, lighting, ground appearance. If unsure, take the screenshot.

**Why:** The feedback loop has been open for 7 sprints. Making screenshots automatic and mandatory -- not optional -- is the only change that has ever worked (Sprint 8 hotfix). This section makes that change structural.

#### UPDATE Section 2: Add Spatial Testing Requirement

Add to TDD is Mandatory:
- **Spatial assertions for actor-spawning systems.** For every system that spawns actors at a specified position, write at least one test that verifies `GetActorLocation()` matches the requested spawn position. These tests run in NullRHI headless mode. Logic tests verify formulas; spatial tests verify the engine actually placed the actor where you asked. (Added: Strategic Retrospective 9 -- VFX origin bug survived 170 logic tests for 3 sprints.)

#### UPDATE Section 9: Reference Auto-Screenshot

Add to "How to verify":
- **Automatic (preferred):** Run `build-and-verify.sh` which takes a post-build screenshot automatically. The agent Reads the screenshot file to confirm visual correctness. This is the default workflow for all visual changes.

### 7. Agent Feedback — Key Quotes and Insights

**Engine Architect** — "A successful UBT build tells you nothing about whether your actors are visible, positioned correctly, or behaving as intended. The web has no equivalent gap -- if `div.style.backgroundColor = 'red'` compiles, the div is red. Period." ... "The best verification is the verification that happens automatically. The agent should not have to choose to verify visual output. It should be unable to avoid it."

**DevOps Engineer** — "We will never achieve web-dev parity. The compilation step alone is an irreducible bottleneck. But we can go from 'the agent literally cannot see what it built' to 'the agent sees a screenshot of its changes within 30 seconds.' That is a 10x improvement." ... "The root cause of every visual verification failure in Sprints 2-8 is the same: the feedback loop is open. We write code, we test code, we never look at the result."

**Game Designer** — "WebFrogger has zero distance between description and reality. When the code says `color: 0x22cc22`, the thing IS green. There is no intermediate representation, no asset pipeline, no build step, no binary format, no material system, no component hierarchy." ... "The verification tax must be near-zero or it will be skipped. Our team is disciplined. We have 21 sections of agreements. We have a QA Lead. We have a definition of done. And we skipped visual verification for SEVEN SPRINTS. Why? Because it was expensive."

**QA Lead** — "Every test is correct about what it tests. But not a single test spawns a VFX actor into a world and checks where it ended up." ... "The gap is not between 'no testing' and 'visual testing.' The gap is between 'testing code logic' and 'testing rendered output.' Position assertions bridge that gap without requiring screenshots, pixel diffs, or a running editor."

### 8. Meta-Observation

This retrospective is different from the previous 8. Those retrospectives asked: "What went wrong this sprint?" This one asks: "What is structurally wrong with how agents build games in Unreal Engine?"

The answer is not "the team lacks discipline." The team has 21 sections of agreements, 170+ tests, 8 retrospectives, and a detailed retrospective log. The answer is: **the tools do not close the feedback loop.** Web development works in one prompt because the feedback loop is closed by default -- you cannot write web code without seeing the result. UE development fails in 8 sprints because the feedback loop is open by default -- you can write UE code for months without ever seeing the result.

Every agreement we have written about visual verification (Sections 5a, 9, 21) is an attempt to manually close a loop that should be closed automatically. The Sprint 8 hotfix proved this: the moment someone actually ran the game and looked at the screen, the VFX bug was found and fixed in 13 lines. The 170 tests, 21 agreement sections, and 8 retrospectives did not find it. Running the game found it.

The path forward is not more agreements. It is better tools. Tools that make "run the game and look at it" as effortless as "save and refresh." That is what Changes 1-4 deliver. That is what the stakeholder is asking for. That is what will make agentic Unreal development viable.

### Action Items

- [ ] **P0: Build `build-and-verify.sh`** — auto-screenshot after every visual build. (DevOps)
- [ ] **P0: Write `[Spatial]` position assertion tests** — minimum 5 tests for VFX, frog, hazard, camera, light spawn positions. (QA Lead)
- [ ] **P0: Screenshot-in-the-loop** — print structured paths after PlayUnreal screenshots; update agent profiles to Read screenshots. (DevOps)
- [ ] **P1: Keep-alive editor + Live Coding** — `--keep-alive` flag for run-playunreal.sh, `live_compile()` for client.py. (DevOps)
- [ ] **P1: Deterministic object paths** — `GetObjectPath()` and `GetPawnPath()` UFUNCTIONs on GameMode. (Engine Architect)
- [ ] **P1: State-diff client** — delta tracking in client.py. (DevOps)
- [ ] **P2: Declarative scene description** — Research spike for JSON/YAML scene format with runtime loader. (Engine Architect + Game Designer)
- [ ] **P2: Use UE5 built-in screenshot APIs** — FScreenshotRequest, OnScreenshotCaptured instead of macOS screencapture. (Engine Architect)
