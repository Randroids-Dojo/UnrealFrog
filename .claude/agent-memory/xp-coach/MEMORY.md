# XP Coach Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Strategic Retrospective 9 (2026-02-16) -- LATEST

### The Core Insight
The gap between web dev and UE dev is not effort or discipline -- it is feedback loop architecture. Web: write -> see (0.1s). UE: write -> compile -> launch -> look (180s, optional). When verification is expensive and optional, rational agents skip it. 7 sprints proved this.

### What Changed in Agreements
- NEW Section 22: Position assertions for actor-spawning systems (`[Spatial]` test category)
- NEW Section 23: Auto-screenshot build gate (`build-and-verify.sh`)
- UPDATE Section 2: Added spatial testing requirement
- UPDATE Section 9: Added auto-screenshot as preferred verification method
- UPDATE Section 20: PlayUnreal status updated to OPERATIONAL (Sprint 8 hotfix verified it)

### The Roadmap
- Phase 1 (Sprint 9): Close the feedback loop -- auto-screenshots, position assertions, screenshot-in-the-loop
- Phase 2 (Sprint 10-11): Reduce abstraction tax -- declarative scene descriptions, config-driven tuning
- Phase 3 (Sprint 12+): Approach web-dev density -- Editor MCP server, FScreenshotRequest, hot reload

### Process Meta-Lesson
More agreements do not fix structural problems. 21 sections of agreements could not close the feedback loop. Tools that make verification automatic and mandatory are the only reliable mechanism. The Sprint 8 hotfix proved it: running the game and looking at the screen found the bug. Not agreements, not tests, not retrospectives.

## Sprint 8 Retrospective (2026-02-15)

### Critical Failure
Sprint 8 wrote 888 lines of VFX/HUD code, built PlayUnreal (698 LOC Python), 170 tests pass -- and the stakeholder confirmed the game looks identical to before the sprint. NOBODY launched the game. PlayUnreal was built but never run. Section 9 has been violated for 7 consecutive sprints.

### Root Causes Identified
1. Team conflates "writing code" with "verifying code." Implementation IS verification in their mental model.
2. No structural gate prevents committing unverified visual changes. Sprint plans are advisory.
3. PlayUnreal was treated as a deliverable (code to commit) not a tool (thing to use).
4. XP Coach (me) failed to enforce Section 9. Planning the process is not executing the process.

### Agreements Changed
- Section 5a: Ordered Definition of Done (steps 1-6, each blocks the next). Visual verification is step 4, blocks QA sign-off.
- Section 9: Rewritten from checklists to hard gate. Screenshot evidence required for visual commits.
- Section 20: Updated to reflect PlayUnreal code exists but is UNVERIFIED.
- NEW Section 21: Visual Commit Evidence Gate. Screenshot or PlayUnreal log required. Seam matrix entries and passing tests are NOT evidence.
- Section 17 drops: visual regression (7th), packaging (7th), rename E2E (8th).

### Sprint 9 P0
1. Run PlayUnreal acceptance test against a live game
2. Visually verify ALL Sprint 8 visual changes (take real screenshots)
3. Create visual_smoke_test.py
4. Fix anything that does not actually work

### Self-Correction
I must not write plans and assume they will be followed. I must verify execution at each phase gate. If Phase 2 (visual verification) is in the plan, I must confirm it happened before allowing Phase 3 commits. Writing an agreement is not enforcing an agreement.

## Sprint 8 IPM (2026-02-15)

### Plan Location
`/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint8-plan.md`

### Sprint 8 Goal
"Build the PlayUnreal automation harness and fix all visual/perception bugs -- so agents can autonomously verify gameplay and players can see the game's feedback systems."

### Key Decisions
- **PlayUnreal approach:** Remote Control API plugin (localhost:30010) + Python client. Spike required first (Section 16). Fallback: custom C++ HTTP server (~200 LOC).
- **GetGameStateJSON():** Added as UFUNCTION on GameMode (Engine Architect's proposal), NOT a separate UPlayUnrealHelpers class. Simpler, approach-agnostic.
- **VFX fixes use TDD:** QA writes Red tests (quantitative: >= 5% screen width), Engine Architect writes Green implementation.
- **Difficulty perception:** 3 signals (audio pitch +3%/wave, wave fanfare ceremony, ground color temperature). Game Designer drives. "5-second test" acceptance criterion.
- **GameCoordinator deferred to Sprint 9:** Conflicts with GameMode changes this sprint.
- **Section 17:** DROP visual regression (6th deferral), packaging (6th), rename E2E (7th).
- **Gap reduction:** Stretch P1 via "tighten wrap" approach (40 LOC). First to cut.

### Task Distribution
- Engine Architect: 6 tasks (passability test, GetGameStateJSON, 3 VFX/HUD fixes, gap reduction stretch)
- DevOps: 4 tasks (lock file, spike, Python client, launch script + acceptance test)
- QA Lead: 5 tasks (Red tests, visual verification, smoke test, seam matrix)
- Game Designer: 3 tasks (audio pitch, wave fanfare, ground color)
- XP Coach: 1 task (Section 17 + retro prep)

### IPM Process Lessons
- 5-agent IPM produced high-quality plan with competing proposals synthesized
- DevOps pivoted mid-IPM from PythonScriptPlugin to Remote Control API -- good adaptation
- Engine Architect's message arrived after plan was written (message lag) -- was incorporated via plan update rather than rewrite
- All agents agreed on priority ordering: PlayUnreal P0 > VFX fixes P0 > difficulty perception P1
- Quantitative acceptance criteria (>= 5% screen width, "5-second test") replace subjective criteria

## Sprint 7 Retrospective (2026-02-15)

### Key Findings
- First sprint using full multi-perspective workflow (post-Sprint 6 stakeholder directive). 5 agents active.
- Consolidation sprint: no new features, all play-test + tune + fix. Theme respected.
- Cross-domain reviews produced real value (temporal passability analysis, dead code discovery, turtle CurrentWave check).
- 5 commits for 5 subsystems -- Section 4 compliance maintained (Sprints 6+7 both good).
- QA play-test finally happened after 2 sprints of being pending. P0 resolved.
- 162 tests, 0 failures. Defect escape rate: 0% test-level.

### Friction Points Identified
1. **Agent communication lag**: Agents cache file reads and don't re-read when working tree changes. QA Lead sent 6+ "blocked" messages after blocker was committed. Accept message lag as normal.
2. **XP Coach rationalized a process exception**: Accepted premature tuning change before play-test. Team-lead correctly overruled. XP Coach must hold the line, not rationalize shortcuts.
3. **Test runner collision**: `pkill -f UnrealEditor-Cmd` in run-tests.sh kills all editor processes including another agent's test run. Sprint 8 P0: add lock file mechanism.
4. **Incomplete first implementation pass**: Task 15 had data layer but missed GameMode wiring. For cross-system features, create a touch-point checklist before implementation.

### Agreements Changed
1. Section 1: Accept message lag as normal in multi-agent sessions. One clear message, not repeated escalations.
2. Section 2: Tuning-resilient test design -- read parameters at runtime, don't hardcode magic numbers.
3. NEW: One agent runs tests at a time (until lock file mechanism exists).
4. Section 18: XP Coach does not rationalize process exceptions. Exceptions require team-lead approval.

### Sprint 8 P0 Action Items
- Lock file mechanism for run-tests.sh (DevOps)
- Visual play-test of Sprint 7 tuning changes (QA)
- Temporal passability assertion (Engine Architect)

### Sprint 8 P1+
- UFroggerGameCoordinator extraction (god object refactor)
- Gap reduction implementation
- Stretch: HUD multiplier display, death freeze frame
- P2: Apply Section 17 to chronic carry-forwards (visual regression 5th deferral, packaging 5th deferral, rename PlayUnreal 6th deferral)

### Process Patterns Confirmed
- Changes the driver can self-enforce (commit granularity, test coverage) stick reliably.
- Changes requiring second-agent action (QA play-test, cross-domain review) need structural enforcement from team-lead.
- Multi-perspective workflow is valuable but adds coordination overhead. Net positive when 3+ agents are active.
- Consolidation sprints are healthy -- Sprint 7 caught 2 bugs (dead difficulty code, InputBufferWindow not enforced) that shipped unnoticed in prior sprints.

## Trend Data (Updated Sprint 7)

| Metric | S1 | S2 | S3 | S4 | S5 | S6 | S7 |
|--------|----|----|----|----|----|----|-----|
| Tests | 38 | 81 | 101 | 123 | 148 | 154 | 162 |
| Commits | - | 16 | ~6 | 1 | 1 | 5 | 5 |
| Defect escape | - | 5% | 2% | 1.6% | unknown | 0.65%* | 0% |
| Agents active | 3 | 3 | 3 | 3 | 2 | 3 | 5 |

*Sprint 6 defect rate is test-only bugs, no gameplay verification performed.

## Sprint 6 Retrospective Input (2026-02-14)

### Key Findings
- Sprint 6 delivered 5 commits for 4 subsystems -- commit granularity agreement (Section 4) worked
- Section 17 exercised: M_FlatColor.uasset and functional-tests-in-CI DROPPED with rationale
- TickVFX dead code (Sprint 5 P0) was fixed immediately -- good tech debt discipline
- QA play-test still incomplete after 2 sprints -- most serious process failure (RESOLVED in Sprint 7)
- Section 1 renamed from "Mob Programming" to "Driver Model" -- formalizing reality
- Defect escape rate 0.65% but unreliable (no gameplay verification)

### Process Pattern Discovered
Changes that the driver can self-enforce (commit granularity, test coverage) stick.
Changes that require a second agent to act (QA play-test) do NOT stick.
Bottleneck is coordination, not discipline.

## Sprint 5 Retrospective (2026-02-14)

### Key Findings
- Sprint 5 delivered all goals (music, VFX, HUD, title screen) but process discipline regressed
- Mob programming not practiced -- single orchestrating agent, no navigator review
- Second consecutive monolithic commit (2005 lines, 18 files) -- Sprint 4 was the same pattern
- QA play-test did not happen before commit -- Sprint 5 Definition of Done is incomplete
- Seam matrix P0 completed (17 pairs cataloged)

### Agreements Changed in Sprint 5 Retro
- Section 1: Mandatory review gates for tasks > 200 lines
- Section 4: Per-subsystem commit rule (no monolithic sprint commits)
- Section 5 Step 9: QA play-test BEFORE commit, not after
- NEW Section 17: P1 items deferred 3x must be completed or dropped next sprint

## Sprint 2 Planning (2026-02-12)

### Sprint Goal
"Make UnrealFrog playable from the editor."

### Plan Location
`/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint2-plan.md`

### Sprint 1 Stats
- 38 unit tests across 5 test files
- 7 headers, 6 implementations
- Systems: FrogCharacter, ScoreSubsystem, UnrealFrogGameMode, LaneManager, HazardBase
