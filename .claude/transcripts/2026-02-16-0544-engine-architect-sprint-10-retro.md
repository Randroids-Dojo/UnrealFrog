# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-opus-4-6 |
| Tokens | ~184,410 input, ~3 output |
| Duration | 5s |

---

## User (05:44 UTC)

<teammate-message teammate_id="engine-architect" color="green" summary="Engine Architect Sprint 10 retro analysis">
## Engine Architect — Sprint 10 Retro Analysis

### 1. Was capsule radius the right margin for FindPlatformAtCurrentPosition?

**Yes, but for the wrong reason.** Using `GetScaledCapsuleRadius()` (34 UU) makes the platform detection geometrically honest — the frog's body must genuinely be ON the platform, not dangling off the edge into water. That is correct physics. However, the *purpose* of tightening was to fix the planner, not to improve collision fidelity. The real problem was that the planner's margin was too generous (20 UU inset), so the frog was landing on platform edges where the game considered it "in water." The C++ fix happened to fix the symptom, but the root cause was that two independent systems (C++ collision, Python planner) had independently chosen margin constants with no shared source of truth. If we had only tightened the planner, the game collision would still be "generous" and the frog would survive edge-cases it shouldn't — so tightening both was correct. But the fact that we needed TWO commits (15e51d1 then aa4f904) to fix what is fundamentally one parameter mismatch is the real signal.

### 2. Duplicated constants — how do we prevent future mismatches?

This is the biggest structural issue. Right now:
- **C++**: `CollisionComponent->InitCapsuleSize(34.0f, 44.0f)` hardcoded in FrogCharacter constructor (line 19), then `GetScaledCapsuleRadius()` read at runtime (line 412).
- **Python**: `FROG_CAPSULE_RADIUS = 34.0` hardcoded at top of path_planner.py (line 28), `PLATFORM_INSET = FROG_CAPSULE_RADIUS + 10.0` (line 35).

The constants are duplicated across language boundaries with no mechanism to keep them in sync. This will break again whenever anyone tunes collision.

**Proposal: Expose collision constants via GetGameStateJSON (or a new GetConfigJSON endpoint).** Add a one-time `/config` response from the GameMode that returns `{ cellSize, capsuleRadius, gridCols, hopDuration, ... }`. The Python planner reads these on startup instead of hardcoding them. This is cheap (one extra RC API call at init), eliminates the duplication, and means the C++ side is the single source of truth for all geometric constants. No shared config files, no code generation — just one API call.

Specifically:
```cpp
UFUNCTION(BlueprintCallable, Category = "PlayUnreal")
FString GetGameConfigJSON() const;
```
Returns `{ "cellSize": 100.0, "capsuleRadius": 34.0, "gridCols": 13, "hopDuration": 0.15 }`. Python reads this once at connection time and uses it for all calculations.

### 3. Was frogWorldX the right API addition?

**Yes, unambiguously.** `frogWorldX` (commit 44f64e8) solves a real problem: on river rows, the frog drifts with its platform, so `GridPosition.X` (snapped to integer columns) diverges from reality. The planner needs the actual world X to predict timing for the next platform. Without it, every river-to-river hop would use a stale column-snapped position, accumulating error.

One concern: `frogWorldX` returns only X. If the game ever needs lateral river drift or Y-axis platform movement, we'll need `frogWorldY` too. But for now, X is sufficient — river platforms only move on the X axis. Pragmatic.

### 4. Should there be a shared config?

**Yes — but not a file, an API.** A shared JSON/TOML config file that both C++ and Python read sounds clean but is fragile in UE land (path resolution, packaging, runtime loading). The better approach is what I described in point 2: a `GetGameConfigJSON()` UFUNCTION. The C++ side already KNOWS its own constants (they're UPROPERTYs). Exposing them via the same RC API mechanism we use for state queries costs nearly nothing and creates a proper single-source-of-truth.

### 5. Process observation

The stakeholder's retro notes nail the real failure: **cross-domain review was skipped.** The C++ change (collision tightening) was committed without anyone checking the Python planner. This is exactly the scenario Section 18 was written for — Engine Architect changes affect DevOps tooling, and a DevOps reviewer would have immediately flagged the constant mismatch.

**Concrete proposal for agreements.md:** Add to Section 18: "Changes to collision geometry, grid constants, or game physics parameters REQUIRE review from the agent owning the PlayUnreal tooling (currently DevOps). These parameters cross the C++/Python boundary."

---

**Summary of proposals:**
1. Add `GetGameConfigJSON()` UFUNCTION — single source of truth for all geometric constants
2. Python planner reads config at init, eliminates all hardcoded game constants
3. Add cross-boundary review rule to Section 18 for collision/grid/physics parameters

</teammate-message>

## Assistant (05:44 UTC)

Good, Engine Architect's perspective is in. Let me check if the others have arrived.

- **Used TaskList**

## Assistant (05:44 UTC)

Task #2 is completed. Still waiting on QA Lead (#3), DevOps (#4), and Game Designer (#5). I have Engine Architect's perspective captured. Waiting for the remaining three.
