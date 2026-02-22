# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `9cad10df-a2ee-4650-adec-4a8034993a77` |
| Model | claude-opus-4-6 |
| Tokens | ~662,414 input, ~7 output |
| Duration | 51s |

---

## User (16:29 UTC)

<teammate-message teammate_id="game-designer" color="orange" summary="Tuning analysis: 6 gameplay values evaluated, 2 changes + 1 bug found">
I've completed my code analysis for the tuning pass (Task #6). My task is still blocked by #1 and #2, but here's my full evaluation of the 6 key gameplay numbers so we're ready to move the moment those land. This is based on reading all the source files, the original design spec (sprint1-gameplay-spec.md), and doing the math on how the numbers interact at different wave levels.

## Tuning Analysis: 6 Key Gameplay Numbers

### 1. HopDuration = 0.15s — KEEP (no change)

This is correct for the arcade Frogger feel. The spec (Appendix C) explicitly states "0.15s is fast" and warns against changing it without design review. At 0.15s, the player can execute ~6.67 hops per second, which gives the rapid-fire responsive feel that classic Frogger demands. The movement code at `FrogCharacter.cpp:82` uses linear interpolation with a parabolic Z arc — the math is clean. No ease-out curve is needed because 0.15s is fast enough that the linear lerp reads as snappy.

One concern: `BackwardHopMultiplier = 1.0f` means backward hops are the same speed. The spec says "Forward and backward hop speed are identical" so this is correct by design. However, I'd flag this as a future tuning candidate — making backward hops slightly slower (1.2x = 0.18s) would subtly discourage retreat without being punishing. Defer to Sprint 8+.

### 2. HopArcHeight = 30.0 UU — KEEP (no change)

30 UU is 30% of a grid cell (100 UU). This produces a visible arc that reads well from a top-down camera at Z=2200. The parabolic formula `4.0 * Alpha * (1.0 - Alpha)` gives a clean symmetric arc peaking at the midpoint. At 0.15s total, the frog spends only 0.075s ascending — this is barely perceptible but adds the "alive" feeling that differentiates hopping from sliding.

The key question is whether 30 UU is visible at the camera distance. Camera is at Z=2200 looking straight down. A 30 UU vertical displacement at 2200 UU distance subtends ~0.78 degrees of visual angle. This is marginal but still perceptible, especially with the mesh shadow moving. If play-testing reveals the hop feels flat, bump to 40-50 UU. But 30 UU is a safe starting point.

### 3. InputBufferWindow = 0.1s — RECOMMEND CHANGE: 0.1s is too generous relative to HopDuration

This is the most important number to get right. Currently `InputBufferWindow = 0.1s` and `HopDuration = 0.15s`. That means the buffer window opens at 0.05s into a 0.15s hop — that's the FIRST THIRD of the hop! The player can buffer input almost immediately after starting a hop.

**Problem**: This creates a de facto "turbo mode" where mashing produces near-instant chained hops. The frog will chain 6+ rapid hops with no perceptible gap between them. This is:
- Good for expert players who want to chain through traffic gaps
- Bad for legibility — the frog becomes a blur at 0.15s per cell
- Potentially masking river platform detection issues (FindPlatformAtCurrentPosition runs but the frog is already hopping away)

**However**: Looking at the actual implementation in `FrogCharacter.cpp:121-145`, the buffer is simpler than the spec suggests — it buffers ANY input during a hop (not just the last 0.1s). The `InputBufferWindow` property exists on the header but is never checked in `RequestHop()`. The code at line 129 simply stores the direction if `bIsHopping` is true, regardless of timing.

**This is a bug, not a tuning issue.** The InputBufferWindow is defined but not enforced. Currently it acts as an always-on buffer. The correct implementation should check `(CurrentHopDuration - HopElapsed) <= InputBufferWindow` before accepting buffered input.

Once the buffer window is properly enforced:
- **Recommendation**: Change to `InputBufferWindow = 0.08s` (53% of hop duration). This still allows responsive chaining but requires the player to time their input to the back half of the hop animation, creating a skill ceiling for rapid traversal.

### 4. TimePerLevel = 30.0s — KEEP for Wave 1, CONSIDER dynamic timer for Wave 3+

The spec says "30 seconds per round is tight by design" and "skilled players will finish with 10-15 seconds remaining." Let's verify this mathematically:

- Grid rows to traverse: 14 (row 0 to row 14)
- Hops per traversal: 14 forward + ~4 sideways = ~18 hops minimum
- Time per traversal: 18 * 0.15s = 2.7s minimum
- 5 traversals needed: 5 * 2.7s = 13.5s best-case
- Realistic with waiting for gaps: add 50-100% overhead = 20-27s
- River riding adds time for platform alignment

So 30s is indeed tight but achievable. A skilled player finishes in 20-25s. A novice will frequently time out in Wave 1 until they learn to commit forward. This is the correct "tough but fair" arcade tension.

For Wave 3+, the timer does NOT scale with difficulty — it stays at 30s even as gaps shrink and speeds increase. This is correct behavior (harder waves should be harder), but it means the difficulty ramp is steeper than it appears from speed alone. The timer becomes the chokepoint around Wave 5-6. No change needed — this is emergent difficulty design.

### 5. DifficultySpeedIncrement = 0.1 per wave — RECOMMEND CHANGE: 0.15

Current formula: `SpeedMultiplier = 1.0 + (Wave - 1) * 0.1`, capped at 2.0.

At 0.1 per wave:
- Wave 1: 1.0x (base)
- Wave 3: 1.2x 
- Wave 5: 1.4x
- Wave 7: 1.6x
- Wave 11: 2.0x (cap)

The problem: 10% speed bumps are barely perceptible. The Frogger arcade original increased speed by roughly 15-20% per wave. At 10%, a player who clears Wave 1 will breeze through Waves 2-4 with almost no new challenge. The difficulty "wall" doesn't appear until Wave 7+ when speed AND gap reduction combine.

**Recommendation**: Change to `DifficultySpeedIncrement = 0.15`. This gives:
- Wave 1: 1.0x
- Wave 3: 1.3x (noticeably faster)
- Wave 5: 1.6x (challenging)
- Wave 7: 1.9x (near cap)
- Wave 8: 2.0x (cap reached)

This creates a tighter difficulty curve where the "I can't keep up" moment arrives at Wave 5-6 rather than Wave 8-9. Combined with the gap reduction, this produces the classic Frogger feeling of "just one more try" at the wave where most players plateau.

### 6. WavesPerGapReduction = 2 — KEEP (no change)

Current formula: `GapReduction = (Wave - 1) / 2` (integer division).

- Wave 1: 0 reduction (base gaps)
- Wave 3: 1 cell less
- Wave 5: 2 cells less
- Wave 7: 3 cells less

This interacts well with the lane configs. The tightest lane is Row 3 (Car, gap 2) and Row 10 (SmallLog, gap 2). At Wave 5, gap reduction of 2 would take these to gap 0 — but the minimum gap enforcement in ValidateGaps ensures at least 1 cell. This means by Wave 5, the tightest lanes become single-cell timing windows. That's the right difficulty wall for an arcade game.

With my recommended speed increase to 0.15, Wave 5 becomes the "peak difficulty" zone: 1.6x speed AND 2-cell gap reduction. This is where the game should kill most players.

## Summary Table

| Parameter | Current | Recommended | Rationale |
|-----------|---------|-------------|-----------|
| HopDuration | 0.15s | **0.15s** (keep) | Correct arcade feel |
| HopArcHeight | 30.0 UU | **30.0 UU** (keep) | Visible at camera distance |
| InputBufferWindow | 0.1s | **0.08s** (+ fix enforcement bug) | Buffer window not enforced in code; once fixed, 0.08s balances responsiveness vs. readability |
| TimePerLevel | 30.0s | **30.0s** (keep) | Tight by design; math confirms achievable |
| DifficultySpeedIncrement | 0.1 | **0.15** | 10% barely perceptible; 15% matches arcade Frogger pacing |
| WavesPerGapReduction | 2 | **2** (keep) | Creates correct difficulty wall at Wave 5 |

## Critical Bug Found

**InputBufferWindow is not enforced.** `FrogCharacter.cpp:121-145` accepts buffered input at ANY point during a hop, not just the last 0.1s. The `InputBufferWindow` property is defined but never referenced in `RequestHop()`. This should be added to Task #8 or created as its own P0 fix.

Ready to implement tuning changes once my blockers (#1, #2) clear.
</teammate-message>

## Assistant (16:29 UTC)

This is an outstanding tuning analysis — exactly the kind of deep, math-backed reasoning we need. Let me address each part.

The InputBufferWindow bug has already been captured as Task #14 (the team lead or another agent must have created it from your finding). It is assigned to the engine-architect, to be done after Phase 0 completes. Your recommended value change (0.1s -> 0.08s) is noted in the task description.

Let me review the DifficultySpeedIncrement recommendation more carefully before endorsing it, since changing it affects the seam test the QA Lead just wrote.

- **Used Read** on `Source/UnrealFrog/Tests/SeamTest.cpp` (offset=695, limit=80)

## Assistant (16:30 UTC)

I see the issue. The seam test hardcodes the current DifficultySpeedIncrement value (0.1) in its expected values — e.g., "Wave 3 speed multiplier is 1.2" assumes `1.0 + (3-1)*0.1 = 1.2`. If we change the increment to 0.15, those expected values break. The test also hardcodes "Row 3 wave 7 speed = 320" which assumes `200 * 1.6 = 320`.

However, the seam test reads `GM->DifficultySpeedIncrement` from the GameMode's default value. So if we change the default from 0.1 to 0.15, the GameMode's `GetSpeedMultiplier()` will produce different values and the hardcoded assertions will fail.

This means: the tuning change and the seam test update must happen together. The seam test should either (a) be updated to use the new expected values, or (b) be written to derive expectations from the GameMode's properties rather than hardcoding them. Option (b) is more resilient to future tuning changes.

- **Used SendMessage**

## Assistant (16:30 UTC)

Now let me also give the QA Lead a heads-up about the upcoming seam test update.

- **Used SendMessage**

## Assistant (16:30 UTC)

Good coordination. The sprint is well-organized:

- **Phase 0 nearly done:** Tasks #2, #3, #4 complete. Task #1 has one remaining fix (OnDyingComplete path).
- **Phase 1 ready to go:** Game Designer's tuning analysis is complete and thorough. QA Lead's seam tests are written. Both are standing by for Phase 0 to clear.
- **Task #14 (InputBufferWindow bug)** identified by Game Designer, assigned to Engine Architect for after Phase 0.
- **Cross-domain reviews happening:** Game Designer reviewed seam test for gameplay accuracy, DevOps standing by to review engine changes, I am actively navigating all streams.
