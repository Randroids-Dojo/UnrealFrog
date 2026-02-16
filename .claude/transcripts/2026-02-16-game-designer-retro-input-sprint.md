# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `486a1fce-113f-4722-9497-667c8c9ccddd` |
| Model | claude-opus-4-6 |
| Tokens | ~196,222 input, ~3 output |
| Duration | 7s |

---

## User (05:45 UTC)

<teammate-message teammate_id="game-designer" color="orange" summary="Game Designer retro: collision too strict, spec violated, proposals">
## Game Designer Retro Input -- Sprint 10

### 1. Platform Landing Feel: Too Strict, Violates the Spec

The Sprint 10 collision change in `FindPlatformAtCurrentPosition` (FrogCharacter.cpp:398-436) subtracts the full capsule radius (34 UU) from the platform half-width. For a SmallLog (2 cells = 200 UU):

- **Before Sprint 10**: Landing zone = 200 UU (full visual width)
- **After Sprint 10**: Landing zone = 132 UU (66% of visual width)

This is a **34% reduction** in the landing zone. The gameplay spec (Section 4.2) explicitly states: *"Platform bounds are slightly larger than visual bounds (slightly generous landing). The player should feel skillful, not cheated."*

The current implementation does the opposite -- it makes the landing zone *smaller* than the visual, meaning the frog can appear to be ON a log but die anyway. That violates the core design principle that **every death must feel fair** (Miyamoto principle #5 in my agent profile).

**Classic Frogger comparison**: In the original arcade, the frog is ~16px wide and a small log is ~48px. The landing zone is roughly 80% of the visual log width (~38px), and the frog can overhang slightly. In UnrealFrog, 132/200 = 66% is stricter than the original.

**Proposal**: The margin should be `FrogRadius * 0.5` (17 UU), not the full capsule radius (34 UU). This gives a landing zone of 166 UU (83% of visual width) -- close to classic Frogger feel while still preventing the frog from floating in mid-air. This is a feel parameter and should be a `UPROPERTY` named `PlatformLandingMargin` on FrogCharacter, not hardcoded to capsule geometry.

### 2. Path Planner PLATFORM_INSET: Smart but Creates an Unrealistic Player Model

The planner uses `PLATFORM_INSET = 44 UU` (34 capsule + 10 buffer), making the AI's effective landing zone 112 UU -- only 56% of the visual log width. This is extremely conservative. A human player would aim for the center of logs but accept edge landings. The AI never lands on an edge, which means:

- It sometimes waits for a "better" platform when a human would just land
- It can't exploit the game's intended forgiveness to cross faster
- It creates an AI that plays "correctly" but not "naturally"

**Proposal**: Once we fix the game-side margin (proposal #1), the planner's PLATFORM_INSET should be `FrogRadius * 0.75` (~25 UU). This gives the AI a small safety buffer beyond the game's requirement while still playing in the generous landing zone that the design intends.

### 3. One-Hop-at-a-Time: Good Strategy, Unnaturally Slow

35 hops with 1 death in 19.9s is solid AI play, but it's notably different from how a human plays Frogger:

- A skilled human player chains hops through the road section in ~3-4 seconds using input buffering and rhythm. The AI takes one hop, polls state, decides, takes another hop -- no rhythm, no flow.
- On rivers, a human reads the pattern at a glance and commits to 2-3 hops in quick succession. The AI's per-hop query introduces artificial caution.
- The 19.9s crossing means the AI uses ~66% of the 30s timer for one home slot fill. A human in Wave 1 fills all 5 in 30s.

This is fine for a safety-first navigation bot, but it's not a model of good Frogger play. I am NOT proposing we make the AI faster right now -- the one-hop strategy is correct for reliability. But it reveals a feel insight: **our game should reward rhythmic, committed play, and our AI doesn't test that path.**

**Future proposal (not Sprint 11)**: Add a "confident mode" to the planner that chains 2-3 road hops using input buffering when the path is clearly safe. This would test the input buffer system under realistic conditions.

### 4. The Cross-Domain Gap Was the Real Bug

The retro-notes nail it: the C++ collision change was made without updating the Python planner. This is a textbook case for why Section 18 (cross-domain review) exists. The collision detection math is a **game feel parameter** -- it's in my domain. If the Engine Architect had posted "I'm changing FindPlatformAtCurrentPosition to subtract capsule radius from platform width," I would have flagged that it violates the spec's "generous landing" intent.

**Process proposal**: Any change to `FindPlatformAtCurrentPosition`, `CheckRiverDeath`, `HandleHazardOverlap`, or any collision margin should require Game Designer sign-off. These are feel parameters, not engine internals. Add to Section 18 cross-domain pairings: **Game Designer reviews Engine Architect collision/landing changes.**

### 5. The PlatformLandingMargin Should Be a UPROPERTY

The collision margin is currently derived from `CollisionComponent->GetScaledCapsuleRadius()` -- a physics geometry value controlling a gameplay feel parameter. This violates the data-driven design principle (Appendix A of the spec). It should be an explicit `UPROPERTY(EditAnywhere)` float on FrogCharacter with a default of ~17 UU (half capsule radius), so designers can tune it without understanding capsule geometry. The capsule radius should define physics behavior; the landing margin should define gameplay forgiveness. They are different things.

---

**Summary of proposals (priority order):**
1. **P0**: Change landing margin from `FrogRadius` to `FrogRadius * 0.5` (or better: explicit UPROPERTY). Fix the spec violation.
2. **P0**: Add Game Designer to cross-domain review for collision changes (Section 18 update).
3. **P1**: Update path planner PLATFORM_INSET to match new game-side margin + small buffer.
4. **P2**: Future: "confident mode" for planner to test input buffering and rhythm play.

</teammate-message>

## Assistant (05:45 UTC)

Excellent, that's 3 of 4. Game Designer raises an important design concern I hadn't considered -- the collision tightening may have gone too far and actually violates the gameplay spec's "generous landing" principle. Let me check if QA Lead's perspective is in.

- **Used TaskList**

## Assistant (05:45 UTC)

Just waiting on QA Lead (#3) now. Three strong perspectives received:

- **Engine Architect**: `GetGameConfigJSON()` API as single source of truth, cross-boundary review rule
- **DevOps**: Two-tier testing (fast seam test + slow PlayUnreal), `// SYNC:` annotations, post-commit retro reminder hook
- **Game Designer**: Collision tightening violates spec's "generous landing" intent, margin should be a UPROPERTY, Game Designer must review collision changes
