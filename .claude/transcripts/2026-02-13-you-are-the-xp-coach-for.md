# Conversation: 2026-02-13

| Field | Value |
|-------|-------|
| Date | 2026-02-13 |
| Session ID | `2676fd16-2f8c-492a-af3b-1239ff04855a` |
| Model | claude-opus-4-6 |
| Tokens | ~754,941 input, ~183 output |
| Duration | 4m 18s |

---

## User (04:54 UTC)

You are the XP Coach for UnrealFrog. Based on the IPM discussion, draft a Sprint 2 plan. Write it to /Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint2-plan.md

The Sprint 2 goal is: "Make UnrealFrog playable from the editor."

Definition of Done: Press Play, see a frog on a 13x15 grid, hop with arrow keys, cars kill you, logs carry you, score/lives/timer visible, game over when lives=0, home slots fillable.

Here is the synthesized input from all 7 team members. Organize this into a structured sprint plan with:

## Sprint 2 Plan Structure:

### Phase 0: Spec Alignment (MUST DO FIRST)
Resolve these 10 spec-vs-implementation mismatches before any integration work:
1. EGameState enum: 4 states → 6 states (Title, Spawning, Playing, Dying, RoundComplete, GameOver). Keep Paused too.
2. MultiplierIncrement: 0.5 → 1.0 (integer increments per spec)
3. MaxLives: 5 → 9
4. Time bonus: change from percentage-based to `RemainingSeconds * 10`
5. Round completion bonus: add 1000 pts + implement AddBonusPoints()
6. WavesPerGapReduction: 3 → 2
7. Speed cap: add 2.0x cap enforcement
8. GridToWorld: KEEP current convention (origin at 0,0), update spec
9. BackwardHopMultiplier: 1.5 → 1.0 (same speed all directions)
10. Lane direction: verify and fix alternation formula to match spec

All existing tests must be updated to match the corrected values.
Driver: Engine Architect (owns the C++ code + tests)

### Phase 1: Foundation (can parallelize)
- Task 1: Camera system — fixed top-down camera, -72 deg pitch, FOV 60, shows full grid
- Task 2: Enhanced Input — PlayerController with arrow+WASD → RequestHop, Start/Pause
- Task 3: Placeholder meshes — engine primitives on FrogCharacter + HazardBase (cubes=danger, cylinders=platforms, sphere=frog)
- Task 4: Ground plane geometry — 15 rows of colored planes (green=safe, gray=road, blue=river, amber=goal)
- Task 5: Build.cs updates — add UMG/Slate/SlateCore, guard FunctionalTesting

### Phase 2: Integration (sequential, depends on Phase 1)
- Task 6: Collision wiring — physics overlaps for road hazards, grid query for river landings
- Task 7: Game orchestration — connect FrogCharacter death → ScoreSubsystem → GameMode state transitions, hop scoring, timed transitions for Spawning/Dying/RoundComplete
- Task 8: HUD — UMG widget with score, high score, lives (icons), timer bar, wave number, game over text

### Phase 3: Polish & Verification
- Task 9: Create FroggerMain map — minimal .umap with PlayerStart, LaneManager, camera, ground builder
- Task 10: Integration tests — latent automation tests in temp UWorld for collision, scoring, state transitions
- Task 11: PlayUnreal shell script — headless test runner
- Task 12: Play-test — QA Lead executes 18-item checklist
- Task 13: Audio infrastructure stubs — UAudioComponent on frog, folder structure, empty PlayHopSound/PlayDeathSound methods

### Driver Assignments (per team agreements — domain expert drives, same agent writes test AND implementation)
- Engine Architect: Phase 0, Tasks 1-3, 5-8, 10
- Level Designer: Task 4, Task 9 (navigates on camera/mesh tasks)
- Art Director: Navigates on Tasks 3, 4 (validates colors, shapes, visual hierarchy)
- Game Designer: Navigates on Tasks 2, 7, 8 (validates feel, flow, HUD layout)
- DevOps Engineer: Task 5, 10, 11
- QA Lead: Task 12, validates integration tests
- Sound Engineer: Task 13
- XP Coach: Coordination, task sequencing, retrospective

### Dependency Graph
```
Phase 0 (spec fixes) ──→ Phase 1 (foundation) ──→ Phase 2 (integration) ──→ Phase 3 (polish)
                         Task 1 (camera)     ─┐
                         Task 2 (input)      ─┤
                         Task 3 (meshes)     ─┼──→ Task 6 (collision) ──→ Task 7 (orchestration) ──→ Task 8 (HUD)
                         Task 4 (ground)     ─┤                                                         │
                         Task 5 (build.cs)   ─┘                                                         ▼
                                                                                              Task 9 (map) ──→ Task 12 (play-test)
                                                                                              Task 10 (tests)
                                                                                              Task 11 (PlayUnreal)
                                                                                              Task 13 (audio stubs)
```

### Acceptance Criteria (from QA Lead)
AC-1 through AC-8 as defined by QA Lead

### Risks
- Map creation requires editor (can't script binary .umap)
- Enhanced Input assets (InputAction, InputMappingContext) also need editor
- First-time integration will surface bugs hidden by unit test isolation
- State machine rebuild touches every system

### What's Playable After Sprint 2
When you press Play in the Unreal Editor:
- You see a colored grid with 15 rows: green start zone, gray road lanes with colored boxes (cars) moving across, green median, blue river lanes with brown cylinders (logs) floating by, and amber/green goal zone with 5 home slots
- A bright green sphere (the frog) sits at center-bottom
- Arrow keys hop the frog one cell at a time with a visible arc animation
- Red boxes (cars) crossing your path kill you — you see the frog disappear, lose a life on the HUD, and respawn at start after 1 second
- Brown cylinders (logs) in the river carry you — land on one and ride it across. Miss and you die
- Top of screen shows score, high score, and a timer bar draining from green to red
- Bottom shows frog-icon lives and wave number
- Reach a home slot at the top: it fills, you score 200 points, frog respawns
- Fill all 5: round complete, 1000 bonus, wave 2 begins faster
- Lose all lives: "GAME OVER" appears center screen

Write this complete plan to the file. Make it clean, actionable, and specific.

## User (04:54 UTC)

<command-message>retrospective</command-message>
<command-name>retrospective</command-name>
<skill-format>true</skill-format>
Base directory for this skill: /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/retrospective

## Retrospective Protocol

Run this protocol after each feature completion, or when the team encounters significant friction.

### Step 1: Gather Data

Review the evidence since the last retrospective:

```bash
# Commits since last retro (look for the [retro] tag)
git log --oneline --since="$(git log --all --grep='[retro]' -1 --format='%ci' 2>/dev/null || echo '1 week ago')"

# Failed builds or reverted commits
git log --oneline --all --grep='revert\|fix\|broken\|oops'

# Files with the most churn (potential design issues)
git log --pretty=format: --name-only | sort | uniq -c | sort -rn | head -20
```

### Step 2: Identify Friction

Categorize observations into:

**Process Issues** (how we work)
- Driver rotation unclear?
- Tasks too large or too vague?
- Communication gaps between agents?
- Agreements not being followed?

**Technical Issues** (what we build)
- Repeated build failures?
- Flaky tests?
- Asset pipeline errors?
- Merge conflicts?

**Quality Issues** (what we ship)
- Bugs found after "done"?
- Missing tests?
- Inconsistent code style?
- Performance regressions?

### Step 3: Propose Changes

For each issue, write a specific, actionable change:

**Bad**: "We should communicate better"
**Good**: "Before any agent starts implementing, post a 3-sentence plan in the team channel and wait for acknowledgment from at least one other agent"

**Bad**: "Tests should be better"
**Good**: "Every gameplay test must verify: input registered, state changed, visual feedback triggered, audio feedback triggered"

### Step 4: Apply Changes

1. Update `.team/agreements.md` with new or modified rules
2. Update agent profiles in `.claude/agents/` if role boundaries need adjustment
3. Update skills in `.claude/skills/` if technical patterns need refinement
4. Add hooks in `.claude/settings.json` if automated enforcement is needed

### Step 5: Log the Retrospective

Append to `.team/retrospective-log.md`:

```markdown
## Retrospective [DATE] - After [FEATURE/MILESTONE]

### What Went Well
- [Specific positive observation]

### What Caused Friction
- [Issue]: [Root cause]

### Changes Made
- [File modified]: [What changed and why]

### Action Items
- [ ] [Specific follow-up task]
```

### Step 6: Commit

```bash
git add .team/ .claude/
git commit -m "refactor: update team agreements from retrospective [retro]"
```

## When to Trigger

- After completing any feature (mandatory)
- After 3+ failed builds in a session
- After any reverted commit
- When any agent reports feeling "stuck" or confused about process
- At minimum, after every 2 hours of work

## Anti-Patterns to Watch For

- **Retrospective theater**: Going through motions without making real changes
- **Blame-oriented**: Focus on what went wrong in the process, not who did it
- **Too many changes at once**: Pick the top 1-3 friction points. Don't rewrite everything.
- **Ignoring previous retro items**: Check that last retro's action items were completed

## User (04:54 UTC)

<command-message>unreal-build</command-message>
<command-name>unreal-build</command-name>
<skill-format>true</skill-format>
Base directory for this skill: /Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/skills/unreal-build

## Build System

### Module Setup (Build.cs)

```csharp
// Source/UnrealFrog/UnrealFrog.Build.cs
using UnrealBuildTool;

public class UnrealFrog : ModuleRules
{
    public UnrealFrog(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core", "CoreUObject", "Engine", "InputCore",
            "EnhancedInput", "UMG"
        });
    }
}
```

### Target Files

- `UnrealFrog.Target.cs` — Game target (shipping build)
- `UnrealFrogEditor.Target.cs` — Editor target (development)

## Testing Framework

### Automation Tests

```cpp
#include "Misc/AutomationTest.h"

// Simple unit test
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FScoreSystemTest,
    "UnrealFrog.Gameplay.ScoreSystem",
    EAutomationTestFlags::ApplicationContextMask | EAutomationTestFlags::ProductFilter
)

bool FScoreSystemTest::RunTest(const FString& Parameters)
{
    // Arrange
    UScoreSubsystem* Score = NewObject<UScoreSubsystem>();

    // Act
    Score->AddPoints(100);

    // Assert
    TestEqual("Score should be 100", Score->GetCurrentScore(), 100);
    return true;
}
```

### Functional Tests (In-Game)

```cpp
// AFroggerFunctionalTest inherits from AFunctionalTest
// Place in test maps, run via automation framework
UCLASS()
class AFrogHopTest : public AFunctionalTest
{
    GENERATED_BODY()
public:
    virtual void StartTest() override;
    virtual void Tick(float DeltaTime) override;
};
```

### Running Tests

```bash
# Run all project tests
UnrealEditor-Cmd UnrealFrog.uproject -ExecCmds="Automation RunTests UnrealFrog" -NullRHI -NoSound

# Run specific test group
UnrealEditor-Cmd UnrealFrog.uproject -ExecCmds="Automation RunTests UnrealFrog.Gameplay"

# Run with Gauntlet (CI-friendly)
RunUAT.bat RunUnrealTests -project=UnrealFrog.uproject -test=UnrealFrog
```

## Asset Validation

### Data Validation Plugin

Enable `DataValidation` plugin. Create validators:

```cpp
UCLASS()
class UAssetNamingValidator : public UEditorValidatorBase
{
    GENERATED_BODY()
public:
    virtual EDataValidationResult IsValid(TArray<FText>& ValidationErrors) override;
};
```

### Naming Convention Checks

| Asset Type | Prefix | Example |
|-----------|--------|---------|
| Static Mesh | SM_ | SM_Frog |
| Skeletal Mesh | SK_ | SK_FrogAnimated |
| Material | M_ | M_Road |
| Material Instance | MI_ | MI_Road_Wet |
| Texture | T_ | T_Frog_Albedo |
| Sound Wave | SW_ | SW_Hop |
| Sound Cue | SC_ | SC_HopWithVariation |
| Blueprint | BP_ | BP_FrogCharacter |
| Widget Blueprint | WBP_ | WBP_HUD |
| Level | LVL_ | LVL_MainGame |

## Python Editor Scripting

```python
import unreal

# Import an asset
task = unreal.AssetImportTask()
task.filename = "path/to/model.fbx"
task.destination_path = "/Game/Meshes/Characters"
task.automated = True
task.replace_existing = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])

# Validate assets
validator = unreal.EditorValidatorSubsystem()
validator.validate_assets_with_settings(assets, settings)
```

## Assistant (04:54 UTC)

Let me start by reading the relevant existing files to understand the current state of the project.

- **Used Read** on `.claude/agent-memory/xp-coach/MEMORY.md`

- **Used Read** on `.team/retrospective-log.md`

- **Used Read** on `.team/roster.md`

## Assistant (04:54 UTC)

Now let me look at the existing source files and the gameplay spec to ensure the plan references the correct current state.

- **Used Glob** for `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/**/*.md` in `.`

- **Used Glob** for `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/**/*.h` in `.`

- **Used Glob** for `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Source/UnrealFrog/**/*.cpp` in `.`

- **Used Read** on `Docs/Design/sprint1-gameplay-spec.md`

- **Used Read** on `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/LaneTypes.h`

## Assistant (04:54 UTC)

Now let me read the current implementation files to understand the exact state of ScoreSubsystem and FrogCharacter.

- **Used Read** on `Source/UnrealFrog/Public/Core/ScoreSubsystem.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/FrogCharacter.h`

- **Used Read** on `Source/UnrealFrog/UnrealFrog.Build.cs`

## Assistant (04:54 UTC)

Now let me read the existing implementation files to understand the current state fully.

- **Used Read** on `Source/UnrealFrog/Private/Core/ScoreSubsystem.cpp`

- **Used Read** on `Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp`

## Assistant (04:54 UTC)

Now I have a thorough understanding of the current codebase. Let me verify the Planning directory exists and then write the sprint plan.

- **Used Bash**: List Docs directory contents

- **Used Bash**: Create Planning directory

- **Used Write** on `Docs/Planning/sprint2-plan.md`

## Assistant (04:58 UTC)

Now let me update my memory file with Sprint 2 context.

- **Used Read** on `.claude/agent-memory/xp-coach/MEMORY.md`

- **Used Write** on `.claude/agent-memory/xp-coach/MEMORY.md`

## Assistant (04:58 UTC)

The Sprint 2 plan has been written. Here is a summary of what was created:

**File:** `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Planning/sprint2-plan.md`

The plan is structured into 4 phases with 13 tasks total, synthesizing input from all 7 team members:

**Phase 0: Spec Alignment (10 items)** -- Resolves every mismatch between the Sprint 1 implementation and the gameplay spec before any new work begins. The biggest item is expanding `EGameState` from 4 states (`Menu, Playing, Paused, GameOver`) to 7 states (`Title, Spawning, Playing, Paused, Dying, RoundComplete, GameOver`), which requires timed state transitions. Other items include correcting `MultiplierIncrement` from 0.5 to 1.0, `MaxLives` from 5 to 9, time bonus formula from percentage-based to `RemainingSeconds * 10`, adding round completion bonus (1000 pts), fixing `WavesPerGapReduction` from 3 to 2, adding a 2.0x speed cap, and correcting `BackwardHopMultiplier` from 1.5 to 1.0.

**Phase 1: Foundation (5 tasks, parallelizable)** -- Camera system (fixed top-down, -72 deg pitch), Enhanced Input (arrow/WASD to RequestHop), placeholder meshes (green sphere frog, red boxes for cars, brown cylinders for logs), ground plane geometry (15 colored row planes), and Build.cs updates (add UMG/Slate dependencies).

**Phase 2: Integration (3 tasks, sequential)** -- Collision wiring (physics overlaps for road hazards, grid query for river landings), game orchestration (connecting death->score->state transitions with timed delays), and the UMG HUD (score, high score, timer bar, lives icons, wave number, game over overlay).

**Phase 3: Polish and Verification (5 tasks)** -- FroggerMain map creation (editor-required), integration tests (7 cross-system test cases), PlayUnreal shell script (headless test runner), QA play-test (18-item acceptance checklist), and audio infrastructure stubs (UAudioComponent + empty methods for Sprint 3).

The plan includes a full dependency graph, driver assignments per team agreements, 6 identified risks with mitigations, and clear exit criteria. The critical path runs through spec alignment, meshes, collision, orchestration, HUD, map creation, and play-testing.
