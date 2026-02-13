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
- [ ] Build a PlayUnreal automation harness (DevOps Engineer + QA Lead)
- [ ] Create a minimal playable level with placeholder geometry
- [ ] QA Lead must play-test every sprint deliverable
- [ ] Verify build before every commit (no exceptions)
