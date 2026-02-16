# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `623ed273-2316-4fd8-a700-c10fd07213ff` |
| Model | claude-opus-4-6 |
| Tokens | ~48,915 input, ~4 output |
| Duration | 7s |

---

## User (03:32 UTC)

<teammate-message teammate_id="engine-architect" color="blue" summary="Technical barriers analysis with 5 concrete tooling proposals">
## Engine Architect Analysis: Technical Barriers to Single-Shot UE Development

### 1. Why Web Dev Works in One Shot

The WebFrogger result (1,311 lines, single prompt, fully playable) is not lucky -- it is structurally inevitable given the web platform's feedback loop properties:

**Zero compilation.** The agent writes text. The browser interprets text. There is no intermediate compilation step that can succeed while the visual output is wrong. In UE, the compiler is a correctness oracle for *syntax and type safety*, not for *visual output*. A successful UBT build tells you nothing about whether your actors are visible, positioned correctly, or behaving as intended. The web has no equivalent gap -- if `div.style.backgroundColor = "red"` compiles, the div is red. Period.

**The DOM is the rendering model.** In web, the document IS the rendered output. `createElement("mesh")` with position attributes directly describes what the player sees. There is no engine abstraction layer between the code and the pixels. In UE, there are at minimum 4 abstraction layers between code and pixels:
1. The UObject/Actor system (spawning, component attachment, root components)
2. The physics/collision system (overlap events, sweep results, deferred physics sync)
3. The material/rendering system (material parameters, dynamic instances, shader compilation)
4. The world/level system (persistent levels, object paths, map loading)

Each layer has silent failure modes. The WebFrogger `SpawnActor` equivalent is `scene.add(mesh)` -- one call, zero silent failures, immediate visual result.

**Visual output is text.** CSS colors are strings. Positions are numbers. The agent can reason about them because they are the same type as the code it writes. In UE, the visual output is a rendered framebuffer that the agent cannot see. The agent writes `SetVectorParameterValue("Color", FLinearColor(1,0,0))` and has no way to verify that the rendered pixel is red without launching the game and capturing a screenshot. The BasicShapeMaterial gotcha (no "BaseColor" parameter, call silently does nothing) is impossible in CSS because `color: red` either works or produces a parser error.

### 2. Why UE Dev Fails Despite More Effort

Let me catalog the specific technical barriers, ordered by severity of their impact on this project:

#### Barrier 1: Silent Failure Modes (CRITICAL)

UE's C++ APIs are designed for maximum flexibility, which means they fail silently rather than loudly. Our project hit at least 6 silent failures across 8 sprints:

| Silent Failure | Sprint Introduced | Sprint Detected | Root Cause |
|---|---|---|---|
| `SetVectorParameterValue("BaseColor")` does nothing on BasicShapeMaterial | S2 | S2-hotfix | Material has no parameter with that name |
| `SpawnActor<AActor>(FTransform)` discards transform with no RootComponent | S5 | S8-hotfix | Transform has nowhere to be stored |
| `SetActorLocation()` defers overlap events | S2 | S2-hotfix | Physics sync happens next tick |
| `DECLARE_DYNAMIC_MULTICAST_DELEGATE` with no `AddDynamic` binding | S2 | S2-hotfix | Handler exists but is never called |
| `OverlapMultiByObjectType` returns stale results after teleport | S2 | S2-hotfix | Physics bodies haven't updated |
| `Broadcast()` on NewObject without world context silently fails | S3 | S3 | No multicast dispatch without world |

In web development, the equivalent operations throw exceptions or produce visible errors in the browser console. `element.style.nonExistentProperty = "red"` is a no-op, but the browser DevTools show the property was set and the Computed Styles panel shows it had no effect. The feedback is immediate and visible.

In UE, the feedback is *nothing*. The code compiles, runs, returns no error, and the visual output is wrong. The only way to detect these failures is to look at the rendered game -- which is exactly the step the team has skipped for 7 sprints.

**The SpawnActor/RootComponent bug is the canonical example.** Line 300 of `FroggerVFXManager.cpp` originally did:
```cpp
AActor* Actor = World->SpawnActor<AActor>(AActor::StaticClass(), &FTransform(Location));
// FTransform silently discarded -- Actor is at (0,0,0)
```
This compiled, ran, created an actor, applied a material, set a color -- and the VFX was invisible because it was 2200 world units below the camera, at world origin. 170 tests passed because they tested "does the actor exist" and "does the math compute correctly." Nobody tested "is the actor at the location I specified."

The fix was 13 lines: create the mesh component first, `SetRootComponent()`, then `RegisterComponent()`, then `SetActorLocation()`. Trivial -- but invisible without launching the game.

#### Barrier 2: Compile-Test Cycle Time (HIGH)

Web: Save file -> browser auto-reloads -> visual result. Total: <1 second.

UE (our project):
- UBT build (Game target): 45-90 seconds (incremental)
- UBT build (Editor target): 45-90 seconds (incremental, sequential -- cannot run concurrently with Game build due to UBT mutex)
- Kill stale processes: 5-10 seconds
- Launch editor with -game: 15-30 seconds
- Game reaches Playing state: ~2 seconds
- **Total: 2-4 minutes minimum per visual verification cycle**

Over 8 sprints, we have made approximately 80 commits. If each commit required one visual verification cycle, that is 160-320 minutes (2.5-5.3 hours) of pure wait time. The team optimized for throughput by batching: write code, write tests, commit, move on. The visual verification step was the bottleneck, so it was skipped.

This is rational behavior given the incentive structure. The compile-test cycle is the fundamental constraint.

#### Barrier 3: The Agent Cannot See (HIGH)

A web agent writes code and can immediately inspect the DOM, read computed styles, query element positions, and even screenshot the page via Puppeteer/Playwright. The visual state is programmatically accessible.

A UE agent writes code and cannot see anything. The rendered framebuffer is in GPU memory on the local machine. The agent has no access to it until:
1. The editor is launched (15-30 seconds)
2. The Remote Control API is available (requires specific plugin + flag)
3. A screenshot is captured via OS-level tools (screencapture on macOS)
4. The screenshot is saved to disk and read by the agent

This is a 4-step pipeline just to *see* what the player sees. Compare to web: `page.screenshot()` in Playwright. One call.

Our PlayUnreal client (`client.py`) was built to bridge this gap, but it is 707 lines of Python to accomplish what Playwright does in a single function call. And even then, PlayUnreal captures the OS window, not the UE framebuffer -- it gets window chrome, title bars, and whatever is on screen, not a clean game render.

#### Barrier 4: The Thick Abstraction Layer (MEDIUM)

UE's Actor/Component model is powerful but has a large surface area of non-obvious behaviors:

- **Components must be registered** before they participate in rendering or physics
- **Root components are implicit** -- actors without explicit root components have no spatial presence
- **Materials are not colors** -- you cannot set a color on a mesh; you must create a material, create a dynamic instance, find the correct parameter name, and set it on the instance
- **Overlap events are deferred** -- they happen on the physics tick, not on `SetActorLocation`
- **Object paths are map-specific** -- the same actor has a different path depending on which map loaded it

Each of these is documented and correct from UE's perspective. But for an agent writing code from first principles, each is a trap. The web equivalent is trivially simpler: `mesh.position.set(x,y,z)` -- done. `mesh.material.color.set(r,g,b)` -- done.

The abstraction tax is not constant -- it compounds. A VFX system in web is ~50 lines (create geometry, set color, animate with requestAnimationFrame). In UE it is ~350 lines (FroggerVFXManager.cpp) because every step requires explicit component lifecycle management, material pipeline setup, and physics-aware spatial positioning.

### 3. Specific Tooling That Would Close the Gap

Here are concrete proposals, ordered by expected impact:

#### Proposal A: Auto-Screenshot on Build (HIGHEST IMPACT)

**Concept:** After every successful Editor build, automatically launch the game in headless mode, advance to Playing state, take a screenshot, and save it to `Saved/Screenshots/auto/build_<timestamp>.png`. The agent (or developer) sees the screenshot alongside the build output.

**Implementation:**
```bash
# In run-tests.sh or a new build-and-verify.sh
Build.sh UnrealFrogEditor Mac Development ...
if [ $? -eq 0 ]; then
    # Launch editor, wait for RC API, take screenshot, kill editor
    run-playunreal.sh auto_screenshot.py
fi
```

**auto_screenshot.py** would be ~30 lines:
```python
pu = PlayUnreal()
pu.reset_game()
time.sleep(2)  # Wait for Playing state
pu.screenshot("Saved/Screenshots/auto/latest.png")
```

**Why this matters:** The feedback loop becomes: write code -> build (90s) -> see screenshot (30s). Total ~2 minutes. Still slower than web, but the visual output is no longer optional. Every build produces a screenshot. The SpawnActor/RootComponent bug would have been caught in Sprint 5 instead of Sprint 8 because the screenshot would show VFX at world origin.

**Estimated effort:** 50 LOC Python + 20 LOC bash.

#### Proposal B: PlayUnreal as CI Gate (HIGH IMPACT)

**Concept:** The `run-tests.sh` script should have a `--visual` flag that, after running unit tests, launches the game via PlayUnreal, runs `verify_visuals.py`, and reports whether visual verification passed.

**Implementation:** Add a phase to `run-tests.sh`:
```bash
if [ "$VISUAL" = "true" ]; then
    run-playunreal.sh verify_visuals.py
    # Parse exit code and screenshot count
fi
```

**Why this matters:** Visual verification becomes part of the test suite, not a separate step. The team already runs `run-tests.sh --all` before every commit. Adding `--visual` to the pre-commit workflow makes visual verification as automatic as unit testing.

#### Proposal C: Programmatic State Assertions (MEDIUM IMPACT)

**Concept:** The agent should be able to assert actor positions, scales, and visibility programmatically after every spawn. Not via unit tests (which don't render), but via PlayUnreal queries against the live game.

**What exists today:** `get_state()` returns score, lives, wave, frog position, game state. This is useful but limited.

**What is needed:**
```python
# Query any actor's world transform
pu.get_actor_transform("FroggerVFXManager_DeathPuff_0")
# -> {"location": [600, 300, 0], "scale": [8.5, 8.5, 8.5], "visible": true}

# Query screen-space bounds of any actor
pu.get_screen_bounds("FroggerVFXManager_DeathPuff_0")
# -> {"min": [400, 200], "max": [600, 400], "screen_fraction": 0.08}
```

**Implementation:** Add a `GetActorTransformJSON(FString ActorName)` UFUNCTION to GameMode that iterates world actors and returns transforms. Add a `GetScreenBounds` variant that uses `ProjectWorldLocationToScreen`. ~80 LOC C++.

**Why this matters:** The agent could verify "death puff occupies 8% of screen" programmatically, catching the scale bug without requiring a human to look at a screenshot.

#### Proposal D: Hot Reload for Visual Iteration (MEDIUM IMPACT)

**Concept:** UE supports Live Coding (Ctrl+Alt+F11 in editor). If the editor is already running, code changes can be hot-reloaded without restarting. This cuts the visual verification cycle from 2-4 minutes to 30-60 seconds.

**Current blocker:** We use `-game` mode (no editor UI) which does not support Live Coding. We would need to either:
1. Use PIE (Play-In-Editor) mode instead of `-game`, or
2. Use the editor with a minimized editor UI

**Why this matters:** Halving the iteration time doubles the number of visual verification cycles the team can afford per sprint.

#### Proposal E: UE5 APIs We Are Not Using

Several UE5 subsystems are designed for exactly this kind of agent-driven development:

1. **FAutomationTestFramework ScreenshotCompare** -- UE has built-in screenshot comparison for automation tests. We use `SIMPLE_AUTOMATION_TEST` but never use the screenshot comparison feature. This could compare "expected" vs "actual" renders.

2. **UGameViewportClient::OnScreenshotCaptured** -- Delegate that fires when `HighResScreenshot` completes. We could wire this to save to disk automatically.

3. **FScreenshotRequest** -- Programmatic screenshot capture from C++ without OS-level screencapture. Cleaner than our current macOS `screencapture -l <windowID>` approach.

4. **Remote Control API /remote/object/property SET** -- We use property reads but not writes. We could set actor positions, scales, and colors via PlayUnreal without rebuilding.

5. **Automation Driver Plugin** -- Input injection without going through the OS input pipeline. Cleaner than our RC API function call approach for `RequestHop`.

### 4. The Fundamental Insight

The gap between web development and UE development is not about lines of code or number of sprints. It is about the **feedback loop latency for visual correctness**.

- Web: write -> see (0.1 seconds)
- UE (current): write -> compile -> launch -> look (180+ seconds, and "look" is optional)
- UE (with Proposal A): write -> compile -> auto-screenshot (120 seconds, "look" is mandatory)

The web agent cannot write incorrect visual code because the browser immediately shows the result. The UE agent can write incorrect visual code for 7 sprints because nothing forces it to look at the result. The tooling fix is not "better tests" -- it is "mandatory visual output at every build."

Tim Sweeney said something relevant: "The best code is the code you don't have to write." Applied here: the best verification is the verification that happens automatically. The agent should not have to choose to verify visual output. It should be unable to avoid it.

### Summary of Proposals by Priority

| # | Proposal | Impact | Effort | Closes Which Gap |
|---|---|---|---|---|
| A | Auto-screenshot on build | HIGHEST | 70 LOC | Agent cannot see |
| B | PlayUnreal as CI gate | HIGH | 30 LOC bash | Visual verification is optional |
| C | Programmatic state assertions | MEDIUM | 80 LOC C++ | Gap between code-correct and visual-correct |
| D | Hot reload for visual iteration | MEDIUM | Config change | Compile-test cycle time |
| E | Use UE5's built-in screenshot/automation APIs | MEDIUM | 100 LOC C++ | Replaces OS-level workarounds |

The common theme: every proposal makes visual feedback mandatory and automatic rather than optional and manual. That is the only thing that will close the gap with web development.

</teammate-message>

## Assistant (03:32 UTC)

Excellent analysis from Engine Architect. That's 2 of 4.

**Engine Architect key insights captured:**
- 6 documented silent failure modes across 8 sprints -- UE APIs fail silently where web APIs throw/show errors
- The 4-layer abstraction tax: UObject system, physics, materials, world/level -- each with its own silent failure modes
- Compile-test cycle (2-4 min) creates rational incentive to skip visual verification
- "The best verification is the verification that happens automatically" -- agent should be unable to avoid seeing output
- Top proposals: auto-screenshot on build (70 LOC), PlayUnreal as CI gate, programmatic state assertions via UFUNCTION
- Key UE5 APIs we're not using: FScreenshotRequest, OnScreenshotCaptured, Automation Driver Plugin

Still waiting for: **game-designer**, **qa-lead**.
