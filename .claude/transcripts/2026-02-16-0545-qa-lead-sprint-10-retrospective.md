# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-opus-4-6 |
| Tokens | ~4,359,237 input, ~197 output |
| Duration | 4m 41s |

---

## User (05:45 UTC)

<teammate-message teammate_id="qa-lead" color="yellow" summary="Sprint 10 QA retro: 5 observations, 5 proposals">
## QA Lead — Sprint 10 Retrospective Analysis

### Observation 1: Collision change committed without PlayUnreal verification — the process failed at every gate.

The commit `15e51d1` (tighten platform detection) changed `FindPlatformAtCurrentPosition()` to subtract `FrogRadius` from the detection zone. This is a **collision/movement/death** code change — arguably the highest-risk category in the entire codebase. Yet:

- No `test_crossing.py` run against a live game (Section 5a step 4 violation)
- No cross-domain review (Section 18 violation — C++ change affected Python planner, nobody flagged it)
- No QA sign-off (Section 5a step 5 violation)

The planner broke because `half_w - 20` (old tolerance) was more generous than `half_w - FrogRadius(34)`. The fix commit `aa4f904` came 4 minutes later, but the damage was already done: we shipped a gameplay-breaking change without testing it.

**Proposal**: New agreement Section 24: **Mandatory PlayUnreal gate for collision/movement/death code.** Any commit touching `FindPlatformAtCurrentPosition`, `CheckRiverDeath`, `Die()`, `StartHop`, `FinishHop`, `UpdateRiding`, or `RequestHop` in FrogCharacter.cpp MUST include a `test_crossing.py` run log showing the frog successfully crosses the river. No exceptions, no "pending" notes. If PlayUnreal can't run, the commit doesn't ship.

### Observation 2: Zero unit tests cover FindPlatformAtCurrentPosition with edge-distance scenarios.

I grepped the entire test suite. No test calls `FindPlatformAtCurrentPosition()` directly. The collision tests use `HandleHazardOverlap` (the overlap-event path), which is a completely different code path. The synchronous position-based detection — the one that actually determines if the frog survives on river — has no dedicated test.

**Proposal**: Write a parameterized regression test: `[Collision.FindPlatform.EdgeDistance]`. It should:

1. Spawn a frog and a rideable HazardBase in a UWorld
2. Place the frog at various X distances from platform center: `center`, `halfWidth - capsuleRadius - 1` (just inside), `halfWidth - capsuleRadius + 1` (just outside), `halfWidth` (old boundary), `halfWidth + 10` (clearly off)
3. Call `FindPlatformAtCurrentPosition()` 
4. Assert `CurrentPlatform` is non-null for inside positions, null for outside positions

This is a 30-line test that would have caught `15e51d1`'s breakage instantly. The old tolerance (no radius subtraction) would have given a different result than the new one — the test would fail on the "just outside new boundary but inside old boundary" case.

### Observation 3: 183 unit tests passed but the frog drowned. Our test strategy has a systematic blind spot.

This is the same pattern from Sprint 2 (visual bugs), Sprint 5 (VFX invisible), Sprint 8 (VFX at origin), and now Sprint 10 (collision tolerance mismatch). The common thread: **our tests verify code paths, not gameplay outcomes.**

- Collision tests verify "if HandleHazardOverlap is called, CurrentPlatform is set" (wiring test)
- But no test verifies "if a frog lands 30 UU from platform center, it survives" (behavioral test)
- The path planner tests (if any) would verify path planning math, not "does the planned path result in survival"

The gap is between **unit-level correctness** and **integration-level behavior**. We have 183 tests proving individual functions work. We have zero tests proving the game is playable.

**Proposal**: Introduce a `[Gameplay]` test category with tests that simulate multi-step gameplay sequences in a real UWorld:
1. Spawn GameMode, frog, lane with platforms
2. Execute a hop sequence that lands the frog at various platform positions
3. Verify frog survives/dies based on actual position math, not mocked overlaps

### Observation 4: Cross-domain review was the cheapest gate and it was skipped.

The C++ change (`15e51d1`) and the Python planner (`path_planner.py`) share a mathematical contract: `|frog_x - platform_x| <= threshold`. When the C++ side changed `threshold` from `halfWidth` to `halfWidth - capsuleRadius`, the Python side's `half_w - 20` became incorrect. 

A cross-domain reviewer (anyone who knew path_planner.py existed) would have asked: "What about the planner?" This is exactly what Section 18 was designed to catch.

**Proposal**: Formalize the C++ <-> Python contract. Add a comment block at the top of `FindPlatformAtCurrentPosition()`:

```cpp
// PLATFORM DETECTION CONTRACT (shared with Tools/PlayUnreal/path_planner.py)
// Frog is "on platform" iff: |FrogX - PlatformX| <= HalfWidth - FrogRadius
// If you change this formula, update PLATFORM_INSET in path_planner.py
```

And a mirror comment in `path_planner.py`. Shared contracts should be documented at both ends.

### Observation 5: test_crossing.py exists but was never run before committing.

The team built `test_crossing.py` (329 lines of logged navigation with per-hop verification) and then committed a collision change without running it. This is the Sprint 8 pattern repeating: **tools are built as deliverables, not used as tools.**

This is now the second time (Sprint 8: PlayUnreal built but not run; Sprint 10: test_crossing.py built but not run for the collision change). The pattern is clear: building a testing tool does not create the habit of using it.

**Proposal**: `run-tests.sh --gameplay` flag that:
1. Requires a live editor (fails fast if RC API unreachable)
2. Runs test_crossing.py
3. Prints PASS/FAIL
4. Is mandatory for any commit touching FrogCharacter.cpp collision code

This makes the tool's usage automatic rather than discretionary.

---

### Summary of Proposals

| # | Proposal | Addresses |
|---|----------|-----------|
| 1 | Section 24: Mandatory PlayUnreal gate for collision/movement/death code | Obs 1 |
| 2 | `[Collision.FindPlatform.EdgeDistance]` parameterized regression test | Obs 2 |
| 3 | `[Gameplay]` test category with multi-step UWorld sequences | Obs 3 |
| 4 | Documented C++ <-> Python platform detection contract | Obs 4 |
| 5 | `run-tests.sh --gameplay` flag requiring live editor | Obs 5 |

</teammate-message>

## Assistant (05:45 UTC)

All four perspectives are in. Let me verify the blockers are cleared and start the synthesis.

- **Used TaskList**

## Assistant (05:45 UTC)

All blockers cleared. Let me start the retro.

- **Used TaskUpdate** on task 1

## Assistant (05:45 UTC)

Now let me gather the git data per the retrospective protocol.

- **Used Bash**: Commits since last retro

- **Used Bash**: Failed builds or reverted commits since last week

- **Used Bash**: Files with most churn this sprint

## Assistant (05:45 UTC)

Good data. Now let me read the last retro entry's end to know what Sprint 9 stats looked like, and then I'll write the synthesis.

- **Used Read** on `.team/retrospective-log.md` (offset=880, limit=200)

## Assistant (05:45 UTC)

Let me see the rest of the most recent retro entry.

- **Used Read** on `.team/retrospective-log.md` (offset=1070, limit=200)

## Assistant (05:45 UTC)

I have all the data I need. Now let me synthesize the four perspectives and write the retro. Let me also read the end of the strategic retro 9 quickly for action item status.

- **Used Read** on `.team/retrospective-log.md` (offset=1260, limit=150)

## Assistant (05:45 UTC)

I now have everything I need. All four perspectives are in (tasks #2-#5 completed), I've reviewed the git history, the retro notes, the agreements, and the retrospective protocol. Time to synthesize.

Let me now write the retrospective entry and the agreements updates. I'll start with the retro log entry, then update agreements.md.

- **Used Read** on `.team/retrospective-log.md` (offset=1395, limit=30)

## Assistant (05:45 UTC)

Good, that's the end of the file. Now let me write the retrospective log entry and agreements updates.

- **Used Edit** on `.team/retrospective-log.md`

## Assistant (05:47 UTC)

Now let me update agreements.md with the three changes. First, update the "Last updated" line and add the new sections.

- **Used Edit** on `.team/agreements.md`

- **Used Read** on `.team/agreements.md`

## Assistant (05:47 UTC)

Good. Now let me apply the three changes to agreements.md.

**Change 1**: Update Section 18 with cross-boundary review pairings.

- **Used Edit** on `.team/agreements.md`

- **Used Edit** on `.team/agreements.md`

## Assistant (05:48 UTC)

Now add the new sections (24 and 25) before "Things We Will Figure Out Together".

- **Used Edit** on `.team/agreements.md`

## Assistant (05:48 UTC)

Now let me clear the retro-notes.md since the retro has consumed its contents.

- **Used Write** on `.team/retro-notes.md`

## Assistant (05:48 UTC)

Good. Now let me update my agent memory with Sprint 10 lessons.

- **Used Read** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` (offset=1, limit=20)

## Assistant (05:48 UTC)

The memory is already very long (the system prompt says it's truncated at line 200). Let me find the Sprint 9 section and add Sprint 10 lessons after it, keeping it concise.

- **Used Grep** for `## Sprint 9 Lessons` in `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md`

- **Used Read** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` (offset=125, limit=50)

## Assistant (05:48 UTC)

Sprint 10 lessons already exist (lines 137-141). They were written earlier in the sprint. Let me update them with the retro findings.

- **Used Edit** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md`

## Assistant (05:49 UTC)

Now let me verify the agreements file looks correct before reporting to the team lead.

- **Used Bash**: Count lines in updated files

## Assistant (05:49 UTC)

Let me do a quick sanity check on the new sections in agreements.md.

- **Used Grep** for `### 2[0-9]"` in `.team/agreements.md`

- **Used Grep** for `^### [0-9]` in `.team/agreements.md`

## Assistant (05:49 UTC)

All 25 sections are present, numbered correctly. Sections 18, 24, and 25 are the Sprint 10 changes. Let me verify the new retro entry in the log.

- **Used Grep** for `^## Retrospective"` in `.team/retrospective-log.md`

- **Used Grep** for `^## Retrospective [0-9]` in `.team/retrospective-log.md`

## Assistant (05:49 UTC)

Retrospective 10 is appended correctly at line 1402. Everything looks good. Let me now send the summary to the team lead.

- **Used TaskUpdate** on task 1

- **Used SendMessage**

## Assistant (05:49 UTC)

The Sprint 10 retrospective is complete. Here is a summary of what was done:

**Three files updated:**

1. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/agreements.md` -- Updated Section 18 (cross-boundary review pairings for collision changes), added Section 24 (cross-boundary constant synchronization via `GetGameConfigJSON()`), added Section 25 (retro notes living document).

2. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/retrospective-log.md` -- Full Sprint 10 retro entry appended at line 1402, covering the platform detection mismatch incident, all four agent perspectives with synthesis, disagreement resolutions, and 8 action items for Sprint 11.

3. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/retro-notes.md` -- Cleared after consuming Sprint 10 notes into the retro entry.

The retro identified **three top friction points**: (1) duplicated constants across C++/Python with no sync mechanism, (2) cross-domain review skipped for a change touching three domains, (3) zero test coverage on `FindPlatformAtCurrentPosition` boundary conditions. Each has a concrete agreement change and a P0 action item for Sprint 11.

Waiting for team lead's go-ahead to commit.
