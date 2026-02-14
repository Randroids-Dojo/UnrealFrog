# Team Working Agreements

*This is a living document. Updated during retrospectives by the XP Coach.*
*Last updated: Post-Sprint 2 Hotfix Retrospective*

## Day 0 Agreements

These are our starting agreements. They WILL evolve through retrospectives.

### 1. Mob Programming Rules

- **One driver per file at a time.** Multiple drivers are OK if they touch zero overlapping files. (Updated Sprint 1: strict single-driver was unnecessarily slow for independent systems.)
- **Driver rotation**: The XP Coach assigns the driver for each task based on domain expertise.
- **Navigators review in real-time.** Don't wait until the end — flag issues as you see them.
- **Any agent can call "Stop"** if they see a problem. The driver must pause and discuss.

### 2. TDD is Mandatory

- Write the failing test FIRST (Red)
- Implement the minimum code to pass (Green)
- Refactor while tests still pass (Refactor)
- No code merges without passing tests
- **Seam tests are mandatory for interacting systems.** For every pair of systems that interact (e.g., frog + moving platform, frog + submerging turtle, pause + hazard movement), write at least one test that exercises their boundary. Isolation tests verify each system alone; seam tests verify the handshake between them. (Added: Post-Sprint 2 Hotfix Retrospective — 33% of commits were fixes at system boundaries that isolation tests missed.)

### 3. Communication Protocol

- Before implementing, post a 3-sentence plan and wait for acknowledgment
- Use the shared task list for work coordination
- Use direct messages for design discussions
- Disagreements are resolved by the domain expert (Engine Architect for C++, Game Designer for mechanics, etc.)

### 4. Commit Standards

- Conventional Commits format: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:`
- Each commit is one logical change
- Commit message explains WHY, not WHAT (the diff shows what)
- Never commit broken builds
- **Build verification is mandatory before every commit.** Run the UBT build for BOTH Game AND Editor targets and confirm `Result: Succeeded` before `git commit`. Never build just one target — the `-game` flag uses Editor binaries. (Updated Sprint 2: building only the Game target caused debugging confusion.)

### 5. Feature Workflow

1. XP Coach breaks down the feature into tasks with dependency graph
2. Team discusses approach (all agents)
3. **Same agent writes test AND implementation** (prevents API mismatch — learned Sprint 1)
4. Domain expert drives implementation
5. All agents review
6. **Build verification** — UBT build must succeed (Game + Editor targets)
7. Tests pass → merge
8. **QA Lead play-tests** using PlayUnreal automation harness (Added: Stakeholder Review)
9. XP Coach runs retrospective

### 5a. Definition of Done (Added: Stakeholder Review)

A sprint is NOT done until:
- The project builds successfully (Game + Editor targets)
- All automated tests pass
- **Seam tests exist** for all new system interactions introduced this sprint (Added: Post-Sprint 2 Hotfix Retrospective)
- **Visual smoke test passes** after foundation phase: game launches, ground visible, player visible, camera correct, HUD renders (Added Sprint 2: three visual bugs were invisible to unit tests)
- The game is playable — you can press Play and interact with the game
- QA Lead has verified gameplay via PlayUnreal or manual play-testing
- The project is left in a working state for the next sprint

### 6. Asset Pipeline

- All generated assets go through the validation pipeline before import
- Art Director approves visual assets
- Sound Engineer approves audio assets
- DevOps Engineer verifies import automation works

### 7. Retrospectives

- Mandatory after each completed feature
- XP Coach facilitates using the retrospective skill
- Produces concrete changes to THIS document and/or agent profiles
- Changes are committed as part of the retrospective

### 8. PlayUnreal Test Harness (Added: Stakeholder Review)

- Agents must be able to launch the game and interact with it programmatically
- Build a PlayUnreal automation tool (similar to PlayGodot) that can:
  - Launch the editor with a test map
  - Send input commands (hop, pause, etc.) via Remote Control API or Automation Driver
  - Capture game state (frog position, score, lives, game state)
  - Take screenshots for visual verification
- QA Lead and Game Designer use PlayUnreal to validate gameplay feel
- DevOps Engineer owns the PlayUnreal infrastructure

### 9. Visual Smoke Test After Foundation (Added: Sprint 2 Retrospective)

After foundation systems are in place (actors, meshes, camera, ground), **launch the game before writing integration code** and verify:
- [ ] Can you see the ground plane?
- [ ] Can you see the player actor?
- [ ] Does the camera show the full play area?
- [ ] Does the HUD render text?
- [ ] Is there lighting in the scene?

If any fail, fix them immediately. Do NOT proceed to integration tasks with an invisible game.

**Why:** Sprint 2 built 12 tasks before launching. Three critical visual bugs (no lighting, unwired HUD, wrong camera angle) were invisible to unit tests and required a full extra debugging session.

### 10. Scene Requirements for New Maps (Added: Sprint 2 Retrospective)

When creating a new map or empty scene via code, always ensure:
- A directional light exists (sun)
- Ambient lighting exists (sky light or fill light)
- Camera view target is explicitly set
- HUD is wired to game state (polling or delegates)
- Ground/floor geometry is present and lit

**Why:** An empty .umap has NO lights, NO sky, NO atmosphere. C++ auto-spawned meshes are invisible without lighting.

### 11. UE5 Material Coloring (Added: Post-Sprint 2 Hotfix)

**Never call `SetVectorParameterValue("BaseColor", ...)` on engine primitive meshes.** UE5's `BasicShapeMaterial` has no exposed "BaseColor" parameter — the call silently does nothing and meshes render gray.

To tint engine primitives (Cube, Sphere, Cylinder):
1. Use `GetOrCreateFlatColorMaterial()` from `Core/FlatColorMaterial.h` — creates a runtime UMaterial with a "Color" VectorParameter wired to BaseColor
2. Call `Component->SetMaterial(0, FlatColor)` FIRST
3. Then `CreateAndSetMaterialInstanceDynamic(0)` to get a DynMat
4. Then `DynMat->SetVectorParameterValue(TEXT("Color"), DesiredColor)`

**Note:** `FlatColorMaterial.h` uses `WITH_EDITORONLY_DATA` — it returns nullptr in packaged builds. For shipping, create a persistent `.uasset` material.

**Why:** Sprint 2 shipped with all meshes gray. All actors called SetVectorParameterValue but nothing was visible because the base material had no parameter to set.

### 12. Overlap Detection Timing (Added: Post-Sprint 2 Hotfix)

**Never rely on deferred overlap events for same-frame logic.** `SetActorLocation()` without sweep defers overlap events to the next physics tick. Frame-counting grace periods (e.g., `bJustLanded`) are unreliable across different tick rates and physics sync timings.

For immediate platform/collision detection after teleporting an actor:
- Use **direct position-based checks** via `TActorIterator<T>` and distance comparisons
- Compare actor positions and known extents — no physics engine dependency
- This is fast for small actor counts (<50 hazards) and avoids physics sync race conditions
- Reserve `OverlapMultiByObjectType` for queries where physics bodies are guaranteed to be synced (e.g., during physics tick callbacks)

**Why:** The frog always died on river logs because `FinishHop()` queried overlaps at the landing position, but physics bodies hadn't synced yet from the current frame's `SetActorLocation` calls.

### 13. Delegate Wiring Verification (Added: Post-Sprint 2 Hotfix)

**Defining a delegate method is not the same as binding it.** Every delegate (`OnHopCompleted`, `OnFrogDied`, etc.) must have an explicit `AddDynamic`/`AddLambda`/`AddUObject` call somewhere in the codebase. Having a handler method exist does nothing if nobody calls `AddDynamic`.

Verification rule: For every `DECLARE_DYNAMIC_MULTICAST_DELEGATE`, grep for at least one `AddDynamic` binding. If zero bindings exist, the delegate is dead code.

**Why:** Sprint 2 had `HandleHopCompleted` and `HandleFrogDied` methods that existed but were never bound. Score never updated, deaths weren't tracked.

### 14. Game Launch Commands — macOS (Added: Post-Sprint 2 Hotfix)

For visual play-testing on macOS:
```
"/Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor" "<project>.uproject" -game -windowed -resx=1280 -resy=720 -log
```

Key rules:
- **Always use `-windowed`** — fullscreen crashes on macOS (SkyLight framework bug in UE 5.7)
- **Use `UnrealEditor.app`** (GUI) not `UnrealEditor-Cmd` (headless, no window)
- **`-game` uses the Editor binary** — always rebuild the Editor target before play-testing
- **Kill existing editor instances** before launching — shared memory conflicts cause silent startup failures
- Logs are at `~/Library/Logs/Unreal Engine/UnrealFrogEditor/`, NOT `Saved/Logs/`

### 15. Moving Platform Hop Convention (Added: Post-Sprint 2 Hotfix Retrospective)

When the frog is riding a moving platform (river log or turtle), the hop origin must be the frog's **actual world position**, not its stored `GridPosition`. After the hop completes, `FinishHop` must update `GridPosition` by back-calculating from the actual landing location.

Rules:
- `StartHop()`: if `CurrentPlatform != nullptr`, use `GetActorLocation()` as origin, not `GridToWorld(GridPosition)`
- `FinishHop()`: always update `GridPosition` from `WorldToGrid(GetActorLocation())`
- Never assume `GridPosition` is accurate while the frog is on a moving platform — the platform drifts the frog between hops

**Why:** The frog teleported sideways when hopping from a moving log because `StartHop` used the stale grid position (where the frog originally landed) instead of the current position (where the log had carried it). The frog would land in empty river and die instantly.

## Things We Will Figure Out Together

- ~~Optimal driver rotation timing~~ → Resolved: per-task, not time-based
- How to handle disagreements between agents
- When to bring in a specialist agent (Art Director, Sound Engineer, Level Designer not needed for Sprint 1)
- ~~How to balance speed vs. quality~~ → Resolved: quality gate is "playable and building" (Stakeholder Review)

## Resolved Questions (Sprint 1)

- **8 agents right?** Only 3 were needed for Sprint 1 (XP Coach, Engine Architect, DevOps Engineer). Art/Sound/Level agents are for later sprints with content.
- **Opus for all?** Engine Architect benefits from opus. DevOps scaffolding could use sonnet for speed. Evaluate per-task.
- **Driver rotation timing?** Per-task rotation works well. Engine Architect drove most implementation since it's all C++ systems.
- **WIP limits?** One feature at a time is sufficient. Allow parallel non-overlapping systems within a feature.
