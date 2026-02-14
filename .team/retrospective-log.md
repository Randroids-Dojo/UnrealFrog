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
