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
- [ ] Do we need a CI pipeline before adding more systems?
