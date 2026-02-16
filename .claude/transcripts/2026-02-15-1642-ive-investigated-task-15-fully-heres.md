# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `9cad10df-a2ee-4650-adec-4a8034993a77` |
| Model | claude-opus-4-6 |
| Tokens | ~1,451,145 input, ~55 output |
| Duration | 47s |

---

## User (16:42 UTC)

<teammate-message teammate_id="team-lead" summary="Task #15 design review — difficulty wiring approach">
I've investigated Task #15 fully. Here's the design for your cross-domain review:

**Root cause:** `OnRoundCompleteFinished()` increments `CurrentWave` and broadcasts `OnWaveCompleted` — but no listener applies the speed multiplier or gap reduction to hazards. LaneManager has no method to update hazard speeds after initial spawn.

**Proposed fix:**
1. Add `ApplyWaveDifficulty(float SpeedMultiplier, int32 GapReduction)` to `ALaneManager`
2. LaneManager stores base speeds per lane (from initial config) to avoid compounding multipliers on each wave
3. `ApplyWaveDifficulty` iterates `HazardPool`, sets each hazard's `Speed = BaseSpeed * SpeedMultiplier`
4. Gap reduction: for now, just reduce `MinGapCells` in the stored configs (affects respawn positioning if we ever respawn, but existing hazard spacing stays fixed since they're already placed)
5. In `OnRoundCompleteFinished()`, after `CurrentWave++` (line 524), get the LaneManager and call `ApplyWaveDifficulty(GetSpeedMultiplier(), GetGapReduction())`

**Gameplay impact:** Wave 2 hazards move 10% faster, Wave 3 20% faster, etc., up to 2x at Wave 11+. This creates the arcade difficulty ramp.

**Alternative considered:** Respawn all hazards each wave with new speeds/gaps. Rejected — respawning causes visual pop-in and is heavier than just updating the speed property on existing actors.

Does this feel right from a gameplay perspective? Any concerns about the speed-only approach (no gap reduction on existing hazards)?
</teammate-message>

## Assistant (16:42 UTC)

Good design. Let me think through this from a gameplay perspective, especially the gap reduction question.

- **Used Read** on `Source/UnrealFrog/Private/Core/HazardBase.cpp` (offset=1, limit=50)

- **Used Read** on `Source/UnrealFrog/Private/Core/HazardBase.cpp` (offset=50, limit=80)

- **Used Read** on `Source/UnrealFrog/Private/Core/HazardBase.cpp` (offset=125, limit=30)

## Assistant (16:42 UTC)

Good. I see that `TickMovement` at line 78 uses `Speed` directly (`Location.X += Speed * Direction * DeltaTime`). So changing the `Speed` property on existing hazards will immediately affect their movement next tick. The approach is clean.

Now let me also check that `CurrentWave` on turtles is important -- the submerge mechanic at line 88 checks `if (CurrentWave <= 1)`. The fix needs to also update turtle `CurrentWave` when waves advance.

- **Used SendMessage**

## Assistant (16:42 UTC)

Also sending directly to the team lead since they asked for the review.

- **Used SendMessage**

## Assistant (16:42 UTC)

Task #15 review complete. The turtle `CurrentWave` catch is important -- without it, we'd have the same pattern as the original bug (difficulty formulas computed correctly but never reaching the actors that need them). Standing by for play-test and Task #15 implementation.
