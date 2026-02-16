# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-opus-4-6 |
| Tokens | ~190,298 input, ~3 output |
| Duration | 6s |

---

## User (05:44 UTC)

<teammate-message teammate_id="devops-engineer" color="purple" summary="DevOps Sprint 10 retro analysis with 4 proposals">
## DevOps Retro Perspective — Sprint 10

### 1. Duplicated Constants: C++ capsule radius (34) vs Python FROG_CAPSULE_RADIUS (34)

**The problem**: FrogCharacter.cpp uses `CollisionComponent->InitCapsuleSize(34.0f, 44.0f)` at line 19 and `CollisionComponent->GetScaledCapsuleRadius()` at line 412. Python hardcodes `FROG_CAPSULE_RADIUS = 34.0` at path_planner.py:28. When C++ tightened `FindPlatformAtCurrentPosition` to subtract `FrogRadius`, Python's old `20.0` margin was wrong. Two commits were needed (15e51d1 then aa4f904) instead of one atomic change.

**Proposal: Generated constants file + pre-commit validation.**

Concretely:
- Add a `GetCollisionConstantsJSON()` UFUNCTION on the GameMode that emits `{"capsuleRadius": 34, "cellSize": 100, "gridCols": 13}`. We already have `GetGameStateJSON()` and `GetLaneHazardsJSON()` — this is the same pattern.
- At startup, `client.py` queries this endpoint and exposes `pu.constants`. The path planner reads from `pu.constants["capsuleRadius"]` instead of hardcoding.
- For offline testing (no running game), a fallback file `Tools/PlayUnreal/game_constants.json` is committed and updated by `build-and-verify.sh` after each successful build.
- A pre-commit hook (or run-tests.sh --all check) greps for raw numeric literals in path_planner.py that match known game constants and warns if they don't reference the constant name.

This eliminates the duplication entirely at runtime. The fallback JSON handles the offline case.

### 2. Auto-running test_crossing.py on relevant file changes

**Should build-and-verify.sh auto-run test_crossing.py when FrogCharacter.cpp or path_planner.py change?**

No — but not for the reason you might think. `test_crossing.py` requires a LIVE editor with RC API (60-120s startup). That makes it too slow for a build gate. Instead:

**Proposal: Two-tier approach.**
- **Tier 1 (fast, in run-tests.sh)**: A new C++ unit test `[Seam] PlatformDetection.MarginMatchesPythonExpectation` that verifies `FindPlatformAtCurrentPosition` accepts a frog at `HalfWidth - CapsuleRadius - 10` (the Python buffer) and rejects at `HalfWidth - CapsuleRadius + 1`. This catches margin mismatches in <5 seconds as part of the existing headless test suite.
- **Tier 2 (slow, explicit)**: `test_crossing.py` remains a manual PlayUnreal test run via `run-playunreal.sh test_crossing.py`. It's the integration-level confirmation.

This way, every `run-tests.sh --all` would catch the margin mismatch that broke Sprint 10, without the 120s editor startup cost.

### 3. Retro-notes.md as a living document

The stakeholder's request is sound. However, relying on agents to remember to write notes mid-sprint is exactly the kind of "aspirational process" that fails (see: 7 sprints of visual verification skipping).

**Proposal: A `.claude/hooks/post-commit-retro-reminder` hook.**

After every commit that touches `Source/` or `Tools/PlayUnreal/`, print a one-line reminder:
```
[Reminder] Add sprint notes to .team/retro-notes.md if this change involved a surprising discovery, a workaround, or a process violation.
```

This is a nudge, not a gate. It keeps the living document in the agent's attention without blocking work. If we want a harder gate: `build-and-verify.sh` could check whether retro-notes.md was modified since the last commit to `Source/` and print a warning (not a failure) if not.

I lean toward the light nudge. The retro-notes file works best when agents genuinely want to record something, not when they're forced to produce boilerplate.

### 4. Tooling that would have caught the margin mismatch automatically

The Sprint 10 failure chain was: C++ changed collision math -> Python planner still used old margin -> frog died in river -> nobody ran test_crossing.py before committing.

Three tooling improvements that each independently would have caught it:

1. **Seam unit test** (Tier 1 above): A C++ test asserting the platform detection boundary matches the expected inset. Runs in headless mode, <5s.

2. **Cross-file dependency annotation**: A comment in `FrogCharacter.cpp:FindPlatformAtCurrentPosition` saying `// SYNC: path_planner.py:PLATFORM_INSET depends on this formula`. grep-able. A pre-commit check could scan for `// SYNC:` annotations and warn if the referenced file wasn't also modified in the same commit.

3. **Runtime constant query** (proposal #1 above): Eliminates the duplication entirely so there's nothing to get out of sync.

Of these, option 1 (seam unit test) is the cheapest to implement and gives the fastest feedback. Option 3 (runtime constants) is the most robust long-term. I'd do both.

### Summary

| Issue | Root Cause | Proposal | Effort |
|-------|-----------|----------|--------|
| Duplicated constants | No single source of truth | GetCollisionConstantsJSON() + pu.constants | 1-2 hours |
| No auto-detection of margin mismatch | Missing seam test | [Seam] PlatformDetection boundary test | 30 min |
| Retro notes forgotten | Process, not tooling | Post-commit reminder hook | 15 min |
| Cross-file dependency invisible | No annotation convention | `// SYNC:` comments + pre-commit grep | 30 min |

</teammate-message>

## Assistant (05:45 UTC)

Good, DevOps perspective is in. That's 2 of 4. Let me check the task list.

- **Used TaskList**

## Assistant (05:45 UTC)

Engine Architect (#2) and DevOps (#4) are done. Still waiting on QA Lead (#3) and Game Designer (#5). I have strong perspectives from two agents so far — good convergence on the `GetGameConfigJSON` API as the structural fix, and divergence on the retro-notes enforcement approach (nudge vs. gate). I'll synthesize once all four are in.
