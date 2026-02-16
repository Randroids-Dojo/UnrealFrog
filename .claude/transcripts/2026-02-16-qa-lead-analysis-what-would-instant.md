# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `623ed273-2316-4fd8-a700-c10fd07213ff` |
| Model | claude-opus-4-6 |
| Tokens | ~2,246,668 input, ~137 output |
| Duration | 5m 0s |

---

## User (03:32 UTC)

<teammate-message teammate_id="qa-lead" color="purple" summary="Instant verification analysis for UE development">
## QA Lead Analysis: What Would "Instant Verification" Look Like for UE Development?

### 1. THE FUNDAMENTAL QA PROBLEM

In web development, verification is a byproduct of working. You write HTML/CSS/JS, the browser renders it, and you see the result. The feedback loop is: write -> save -> see. Cost: zero seconds.

In Unreal Engine development, verification is a separate expensive activity. The feedback loop is: write -> compile (30-120s) -> launch editor (15-30s) -> load map (5-10s) -> navigate to the effect -> observe. Cost: 2-5 minutes per cycle.

For AI agents, it is even worse. Agents cannot see the screen at all. They operate in a fundamentally different mode from human UE developers. A human developer compiles, launches, and *looks*. An agent compiles, launches, and... writes a passing test. The test verifies logic, not visual output. The agent has zero sensory feedback about what the game actually looks like.

This is not a process problem. It is an architecture problem. The web development stack was designed around hot-reload and visual feedback. The UE development stack was designed around a visual editor that a human sits in front of. Agents sit in front of neither.

### 2. WHY 170 TESTS MISSED A 7-SPRINT BUG

The specific bug: VFX actors spawned at world origin (0,0,0) instead of the frog's position. Root cause: `AActor` has no `RootComponent` by default. Passing `FTransform` to `SpawnActor` is silently discarded. The mesh renders at origin.

Here is what the tests verified, and why every check passed:

**VFXTest.cpp** (9 tests):
- `SubsystemInitializes` -- verifies VFXManager can be created. Does NOT test spawning.
- `SpawnDeathPuff_NullSafe` -- verifies SpawnDeathPuff does not crash without a world. There IS no world, so no actor spawns at all.
- `SpawnHopDust_NullSafe` -- same pattern. Null safety, not functional correctness.
- `DisabledSuppressesVFX` -- verifies bDisabled flag. No actor spawning.
- `WiringDelegateCompatibility` -- verifies method signatures are compatible. No rendering.
- `DeathPuffScaleForCameraDistance` -- verifies the MATH of `CalculateScaleForScreenSize()`. The formula is correct. But the actor's position is wrong.
- `HomeSlotSparkleReadsGridConfig` -- verifies no-world fallback returns ZeroVector. Does NOT test actual spawn position.

Every test is correct about what it tests. But not a single test spawns a VFX actor into a world and checks where it ended up. The tests verify: (1) the subsystem initializes, (2) null contexts do not crash, (3) the math formulas are correct, (4) the disable flag works. They do NOT verify: (5) an actor spawned by the VFX system actually appears at the requested location.

**HUDTest.cpp** (13 tests):
- Tests create `AFroggerHUD` via `NewObject` -- no Canvas, no screen, no rendering.
- `ScorePopUsesWorldProjection` -- verifies the pop Y-coordinate is not hardcoded to 10. But it tests the struct data, not whether the pop is visible on screen.
- `WaveFanfareParameters` -- verifies scale math (2.0 -> 1.0 interpolation). Correct math, but untested visual output.

**PlayUnrealTest.cpp** (6 tests):
- These are the closest to integration tests. They call `GM->HandleHopCompleted()`, `GM->HandleFrogDied()`, etc.
- But they use `NewObject<>()` -- no world, no rendering, no physics.
- They verify state machine transitions (Playing -> Dying -> Spawning) and score math.
- They do NOT verify that anything is visible on screen.

**The pattern**: Every test verifies CODE LOGIC in isolation. Zero tests verify RENDERED OUTPUT. The gap between "code is correct" and "game looks right" was never bridged.

What kind of testing WOULD have caught this? Exactly one of these:

a. **A spawn-position assertion test**: Spawn a VFX actor into a real UWorld (via a functional test or integration test with `CreateWorld()`), then check `Actor->GetActorLocation() == RequestedLocation`. This would have caught the origin bug immediately because the position would be (0,0,0) instead of the requested location.

b. **A screenshot comparison**: Take a screenshot after triggering VFX, compare against a baseline. If the baseline shows a pink sphere near the frog and the current frame shows it at the corner of the screen, fail.

c. **A PlayUnreal visual probe**: After hopping, query the game for all actors within 200 UU of the frog's position. If no VFX actor is found, fail. This is like a headless "visual" test -- it checks spatial correctness without needing to see pixels.

### 3. WHAT IDEAL VERIFICATION WOULD LOOK LIKE

The goal: make "launch and look" as natural as "save and refresh." Here is the workflow, ranked from most impactful to least.

**Level 1: Post-Compile Screenshot (the minimum viable loop)**

After every successful build that touches visual code, the agent should:
1. Launch the game headless (already possible with `UnrealEditor -game`)
2. Wait for the map to load (detect via RC API `get_state()`)
3. Take a screenshot via `screencapture`
4. Analyze the screenshot (the agent IS multimodal -- it can look at images)
5. Decide if the visual change is present

This is the web-dev equivalent of "save and refresh." It takes ~45 seconds instead of ~0 seconds, but it IS automatable. The key insight: **Claude agents can already look at images.** We have screenshot capability in PlayUnreal. The missing piece is making this automatic, not optional.

Proposed integration:
```
# In run-tests.sh, after tests pass:
if [[ "$VISUAL_VERIFY" == "1" ]]; then
    ./Tools/PlayUnreal/run-playunreal.sh --quick-screenshot
    echo "Screenshot saved to Saved/Screenshots/post-build.png"
    echo "AGENT: Review this screenshot before committing."
fi
```

**Level 2: Visual Assertions (spatial correctness without pixels)**

PlayUnreal can already query game state. Extend it to query actor positions:

```python
# Proposed: PlayUnreal spatial assertions
def assert_actor_near(self, class_name, expected_pos, tolerance=200):
    """Assert that an actor of the given class exists near expected_pos."""
    actors = self._list_actors_of_class(class_name)
    for actor in actors:
        pos = self._read_property(actor, "RelativeLocation")  # or GetActorLocation
        dist = math.sqrt(sum((a-b)**2 for a,b in zip(pos, expected_pos)))
        if dist < tolerance:
            return True
    raise AssertionError(f"No {class_name} found within {tolerance} UU of {expected_pos}")

# Usage in verify_visuals.py:
pu.hop("up")
time.sleep(0.5)
frog_pos = pu.get_state()["frogPos"]
world_pos = grid_to_world(frog_pos)
pu.assert_actor_near("StaticMeshActor", world_pos, tolerance=200)  # VFX actor
```

This catches the origin bug without needing pixel analysis. If the VFX actor is at (0,0,0) but the frog is at (600, 100, 0), the assertion fails. This is fast (one RC API call), reliable (no image analysis ambiguity), and automatable.

**Level 3: Screenshot Regression Baselines**

For known-good states, save baseline screenshots:
```
Saved/Screenshots/baselines/
    title_screen.png
    playing_state.png
    death_vfx.png
    wave_complete.png
```

After each visual change, take a new screenshot and compare. This does NOT need pixel-perfect diffing -- just a rough "does this look similar" check. Since the agent is multimodal, it can compare two images and say "the death effect is missing from the new screenshot."

The workflow:
1. First time: take baseline screenshots, agent reviews and approves
2. Every subsequent build: take comparison screenshots
3. Agent reviews side-by-side: "baseline has pink sphere near frog, current does not -> FAIL"

**Level 4: Continuous Visual Monitoring (the dream)**

The ultimate state: the editor is always running in the background. After each hot-reload (UE supports this), a screenshot is taken automatically. The agent sees every change in real time.

This requires:
- Live code reloading (UE hot reload is unreliable for structural changes, but works for value tweaks)
- Background screenshot daemon (screencapture in a loop)
- Agent integration to receive and analyze screenshots mid-session

This is aspirational for now, but it is where we should aim.

### 4. TOP 3 VERIFICATION IMPROVEMENTS (would have caught the VFX bug in Sprint 5)

**Improvement 1: Mandatory post-spawn position assertion in VFXTest.cpp**

Add a test that spawns a VFX actor into a real UWorld and checks its position:

```cpp
// This test would have caught the bug in Sprint 5
IMPLEMENT_SIMPLE_AUTOMATION_TEST(...)
bool RunTest(...)
{
    UWorld* World = UWorld::CreateWorld(EWorldType::Game, false);
    UFroggerVFXManager* VFX = ...;
    FVector RequestedPos(600.0, 400.0, 0.0);
    VFX->SpawnDeathPuff(RequestedPos, EDeathType::Squish);
    
    // Find the spawned actor
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        FVector ActualPos = It->GetActorLocation();
        TestTrue("VFX actor at requested position",
            FVector::Dist(ActualPos, RequestedPos) < 50.0);
    }
    World->DestroyWorld(false);
}
```

This is a CODE-LEVEL test, not a visual test. It runs in NullRHI headless mode. It would have caught the origin bug because the actor position would be (0,0,0) instead of (600,400,0). Cost: 10 minutes to write, runs in every test suite.

**Improvement 2: PlayUnreal spatial probe after every visual change**

Add to `verify_visuals.py` (or a new `verify_positions.py`):

```python
# After triggering death:
pu.hop("up")  # Into traffic
time.sleep(1.0)
state = pu.get_state()
# Query: is there any actor near where the frog died?
# If VFX spawned at origin, this fails.
```

This requires the RC API to support actor enumeration (currently it does not -- `SearchActor` returns 501). The workaround: add a `GetVFXActorPositions()` UFUNCTION to VFXManager that returns a JSON array of active effect positions. Then PlayUnreal can assert spatial correctness.

**Improvement 3: Screenshot-in-the-loop as a build step**

Make `run-tests.sh` optionally launch the game and take a screenshot after tests pass:

```bash
# run-tests.sh --visual
# After all tests pass:
# 1. Launch editor with -game
# 2. Wait for RC API
# 3. Call reset_game + hop + screenshot
# 4. Save screenshot to Saved/Screenshots/ci_visual_check.png
# 5. Print: "VISUAL CHECK: screenshot saved. Agent must review."
```

This makes visual verification part of the normal development loop, not a separate activity that gets skipped. Even if the agent does not analyze the screenshot automatically, its mere existence creates a record that someone can review.

### SUMMARY: THE HIERARCHY OF VERIFICATION

| Level | What | Cost | Catches |
|-------|------|------|---------|
| Unit tests (current) | Code logic | 0s per run | Logic bugs |
| Position assertions (Improvement 1) | Spatial correctness | 0s per run | Origin bugs, transform bugs |
| PlayUnreal spatial probes (Improvement 2) | Live game spatial correctness | 45s setup + 2s per probe | Wiring bugs, spawn failures |
| Screenshots + agent review (Improvement 3) | Actual visual output | 45s setup + 5s per screenshot | Everything visual |
| Regression baselines (Level 3) | Change detection | 45s + comparison | Regressions |
| Continuous monitoring (Level 4) | Real-time visual | Always running | Everything, immediately |

The team has been operating at Level 0 (unit tests only) for 8 sprints. The minimum viable improvement is Level 1 (position assertions) -- it costs nothing, runs in CI, and would have caught the VFX bug in Sprint 5.

The key insight: **the gap is not between "no testing" and "visual testing." The gap is between "testing code logic" and "testing rendered output." Position assertions bridge that gap without requiring screenshots, pixel diffs, or a running editor.**

Then, separately, PlayUnreal screenshots let the agent actually SEE the game -- which is the UE equivalent of the web developer looking at their browser. We already have the code. We just need to actually run it.
</teammate-message>

## Assistant (03:33 UTC)

All four analyses received. Let me synthesize.

**QA Lead key insights captured:**
- Detailed autopsy of WHY 170 tests missed a 7-sprint bug: every test verifies code logic, zero tests verify rendered output
- The gap is between "testing code logic" and "testing rendered output" -- position assertions bridge this without screenshots
- Hierarchy of verification: unit tests -> position assertions -> spatial probes -> screenshots -> regression baselines -> continuous monitoring
- Team has been at Level 0 for 8 sprints; Level 1 (position assertions) costs nothing and would have caught the VFX bug
- Concrete test code showing what a spawn-position assertion looks like
- "Claude agents can already look at images" -- the missing piece is making screenshot review automatic, not optional

All four analyses are in. Let me now write the retrospective entry.

- **Used Read** on `.team/retrospective-log.md` (offset=1180)

## Assistant (03:33 UTC)

Now let me also read the agreements file to plan what changes to propose.

- **Used Read** on `.team/agreements.md` (offset=1, limit=50)

## Assistant (03:33 UTC)

Now I have everything I need. Let me write the retrospective entry. This is the synthesis of all four analyses into a strategic retrospective.

- **Used Edit** on `.team/retrospective-log.md`

## Assistant (03:35 UTC)

Good. Now update agreements.md with the new sections.

- **Used Read** on `.team/agreements.md` (offset=170)

## Assistant (03:35 UTC)

Now I need to update the agreements. I'll update Section 2 (add spatial testing), update Section 9 (add auto-screenshot reference), and add Sections 22 and 23. I also need to update the "Last updated" line.

- **Used Edit** on `.team/agreements.md`

## Assistant (03:35 UTC)

Now add spatial testing to Section 2.

- **Used Edit** on `.team/agreements.md`

## Assistant (03:35 UTC)

Now update Section 9 to reference auto-screenshot.

- **Used Edit** on `.team/agreements.md`

## Assistant (03:36 UTC)

Now add Sections 22 and 23 before "Things We Will Figure Out Together".

- **Used Edit** on `.team/agreements.md`

## Assistant (03:36 UTC)

Now update the Section 20 current state to reflect the Sprint 8 hotfix verified PlayUnreal.

- **Used Edit** on `.team/agreements.md`

## Assistant (03:36 UTC)

Good. Now let me update my XP Coach agent memory with the key learnings from this strategic retrospective.

- **Used Read** on `.claude/agent-memory/xp-coach/MEMORY.md` (offset=1, limit=10)

- **Used Edit** on `.claude/agent-memory/xp-coach/MEMORY.md`

## Assistant (03:36 UTC)

Now update the shared project memory with the trend data.

- **Used Read** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` (offset=1, limit=10)

## Assistant (03:37 UTC)

I should add the strategic retro key finding to the shared memory. Let me find a good place.

- **Used Grep** for `Sprint 8 Lessons` in `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md`

- **Used Read** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` (offset=110, limit=20)

- **Used Read** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` (offset=124)

## Assistant (03:37 UTC)

The file is exactly 124 lines. I'll add the strategic retro findings at the end.

- **Used Edit** on `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md`

## Assistant (03:37 UTC)

Now mark the task as completed and notify the team lead.

- **Used TaskUpdate** on task 1

- **Used SendMessage**

## Assistant (03:37 UTC)

Task #1 is complete. Here is a summary of what was delivered:

**Files modified:**

1. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/retrospective-log.md` -- Appended Retrospective 9 (~200 lines). Synthesizes analyses from all 4 agents into a strategic retrospective covering: the 100:1 effort ratio between WebFrogger and UnrealFrog, why web dev's zero-distance-between-description-and-reality enables single-prompt games, the 4 structural barriers in UE (open feedback loop, silent failure modes, verification gap, abstraction tax), 7 concrete proposals ranked by impact, a 3-phase roadmap toward closing the gap, and 8 action items.

2. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.team/agreements.md` -- 5 changes:
   - NEW Section 22: Position assertions for actor-spawning systems
   - NEW Section 23: Auto-screenshot build gate
   - Updated Section 2: Spatial testing added to TDD requirements
   - Updated Section 9: Auto-screenshot added as preferred verification method
   - Updated Section 20: PlayUnreal status corrected to OPERATIONAL

3. `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/agent-memory/xp-coach/MEMORY.md` -- Added strategic retro findings

4. `/Users/randroid/.claude/projects/-Users-randroid-Documents-Dev-Unreal-UnrealFrog/memory/MEMORY.md` -- Added strategic retro summary and Sprint 9 priorities
