# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `247322c9-2ef2-44f0-a3c1-77b896924763` |
| Model | claude-opus-4-6 |
| Tokens | ~547,023 input, ~37 output |
| Duration | 46s |

---

## User (17:53 UTC)

<teammate-message teammate_id="game-designer" color="purple" summary="Difficulty perception design: 5 multi-modal signals, priority ranking, PlayUnreal testing plan">
## Game Designer Sprint 8 Proposal: Difficulty Perception System

I have read `.team/agreements.md`, the full retrospective log through the post-Sprint 7 stakeholder review, the gameplay spec, and all relevant headers (GameMode, FrogCharacter, HazardBase, LaneManager, VFXManager, HUD, AudioManager). Here is my design.

---

### 1. Difficulty Feedback Design: Multi-Modal Wave Signals

**Core problem:** The current difficulty system works at the code level (speed multiplier applies correctly), but a 15% per-wave speed increase is below the human just-noticeable-difference (JND) threshold of ~20-25% for motion speed perception. Classic arcade games solve this with multi-modal feedback — not just "things move faster" but "things LOOK and SOUND different."

**Design principle (Iwatani):** Each wave should feel different in at least 2 sensory channels simultaneously. If you mute the TV, you should still perceive the change. If you close your eyes, you should still hear the change.

**The "5-second test":** Show a player 5 seconds of Wave 1 and 5 seconds of Wave 3 back-to-back. If they cannot immediately tell which is which, the feedback is insufficient. Every signal below is designed to pass this test.

#### Signal 1: Wave Color Temperature (Visual — Ambient)
- **What changes:** The ground/safe zone color shifts from cool to warm as waves increase.
- **Wave 1:** Green/blue (safe, calm). Wave 3: Yellow. Wave 5: Orange. Wave 7+: Red.
- **Implementation:** Modify `GroundBuilder` to accept a wave tint. GameMode calls `UpdateWaveColor(WaveNumber)` from `OnRoundCompleteFinished`. The ground mesh's dynamic material gets a color lerp.
- **Why it works:** Pac-Man uses this — maze color changes between levels. It is ambient (player doesn't need to focus on it), constant (visible for entire wave), and creates an unconscious "danger rising" feeling.
- **LOC estimate:** ~40 lines (GroundBuilder color method + GameMode wiring + material param set)

#### Signal 2: Audio Pitch/Tempo Shift (Audio — Ambient)
- **What changes:** Gameplay music pitch and/or playback rate increases slightly per wave.
- **Wave 1:** Normal pitch/tempo. Each wave: +3% playback speed (compound).
- **Wave 3:** ~6% faster. Wave 5: ~12% faster. Wave 8: ~21% faster.
- **Implementation:** `AudioManager->SetMusicPitchMultiplier(1.0 + (Wave-1) * 0.03)` — UE's `UAudioComponent::SetPitchMultiplier()` handles this natively with no new assets.
- **Why it works:** Space Invaders does exactly this. The accelerating tempo creates subconscious urgency. Players may not consciously notice the pitch change but they FEEL more stressed.
- **LOC estimate:** ~10 lines (new method on AudioManager + call from GameMode)

#### Signal 3: Wave Fanfare + "WAVE N" Ceremony (Audio + Visual — Event)
- **What changes:** Wave transitions get a 1.5s ceremony instead of the current brief text flash.
- **Sequence:** (a) Freeze all hazards for 0.5s. (b) Play ascending jingle (3-note arpeggio, generated procedurally). (c) "WAVE N" text animates from 200% scale to 100% scale with a screen flash. (d) Hazards resume at new speed.
- **Implementation:** New state in RoundComplete flow: the existing 2.0s `RoundCompleteDuration` already covers this. Add a `DrawWaveFanfare()` method to HUD with scale animation. Add a wave fanfare SFX to AudioManager. Hazard freeze is already implicit (they only move during `Playing` state).
- **Why it works:** The wave number is the most important piece of information for perception. Currently it's a brief text overlay. Making it dramatic makes the player AWARE they are entering a harder wave.
- **LOC estimate:** ~50 lines (HUD fanfare drawing + audio jingle generation + wiring)

#### Signal 4: Speed Lines / Screen Edge Vignette (Visual — Active)
- **What changes:** From Wave 3+, faint directional motion lines appear at screen edges, getting denser and more opaque per wave. Like a racing game's speed effect.
- **Wave 1-2:** None (baseline). Wave 3-4: Subtle (alpha 0.1). Wave 5-6: Medium (alpha 0.25). Wave 7+: Strong (alpha 0.4).
- **Implementation:** Canvas-based overlay in HUD. Draw 8-12 white lines from screen center toward edges, with alpha modulated by wave number. Cheap, no meshes needed.
- **LOC estimate:** ~30 lines (HUD drawing method + wave-based alpha calc)

#### Signal 5: Hazard Color Intensification (Visual — Object-Level)
- **What changes:** Road hazards get progressively more saturated/brighter colors. Wave 1 cars are muted blue; Wave 5 cars are vivid red.
- **Implementation:** `AHazardBase::ApplyWaveVisuals(int32 Wave)` called from `LaneManager::ApplyWaveDifficulty()`. Lerps hazard material color from base to intense.
- **LOC estimate:** ~25 lines (HazardBase color update + LaneManager wiring)

#### Priority Ranking (What to Build First)

| Priority | Signal | Perception Channel | LOC | Passes 5-sec Test? |
|----------|--------|-------------------|-----|---------------------|
| 1 | Audio pitch/tempo shift | Audio (ambient) | ~10 | Yes — immediately audible |
| 2 | Wave fanfare ceremony | Audio + Visual (event) | ~50 | Yes — dramatic moment |
| 3 | Ground color temperature | Visual (ambient) | ~40 | Yes — entire screen changes |
| 4 | Speed lines | Visual (active) | ~30 | Marginal — subtle overlay |
| 5 | Hazard color intensification | Visual (object) | ~25 | Marginal — need to focus |

**Minimum viable set for Sprint 8:** Signals 1-3 (~100 LOC). These hit 3 perception channels (ambient audio, event audio+visual, ambient visual) and all pass the 5-second test independently.

---

### 2. Priority Assessment: Should Difficulty Perception Be P0 or P1?

**My recommendation: P1, not P0.** Here is why:

PlayUnreal automation is the correct P0. The stakeholder explicitly said the game shows "no visible differences" — but we cannot verify whether our fixes work without a tool to play the game. If we build difficulty perception first, we are in the same trap as Sprint 5: "does the code work? yes. Can a human perceive it? Unknown."

The correct sequence is:
1. **P0: PlayUnreal** — build the automation tool
2. **P0: VFX/HUD position fixes** — fix score pops and death VFX (these are bugs, not features)
3. **P1: Difficulty perception signals** — implement signals 1-3, then validate with PlayUnreal

However, I want to flag that difficulty perception is the **single most important game feel issue**. The stakeholder's #1 complaint was "no visible differences." PlayUnreal enables us to verify, but perception is what the player experiences. If Sprint 8 can only fit PlayUnreal + VFX fixes, difficulty perception becomes Sprint 9 P0.

---

### 3. PlayUnreal and Difficulty Work: Parallel or Sequential?

**They can run in parallel for implementation, but verification is sequential.**

- DevOps/Engine Architect build PlayUnreal (P0) — no dependency on my work.
- I can write the difficulty perception code and tests simultaneously.
- BUT: I cannot mark difficulty perception as "done" until PlayUnreal exists and I use it to verify the signals are perceivable. Per agreement Section 5 step 8 + Section 9, visual features require in-game verification.

So: **parallel implementation, sequential verification.** This is the most efficient use of sprint time.

---

### 4. What I Want to Test First with PlayUnreal

Once PlayUnreal has `hop(direction)` + `get_state()` + `screenshot()`:

**A/B Tuning Script (the 20-variations-in-5-minutes claim):**

```python
# pseudocode for tuning_sweep.py
SPEED_INCREMENTS = [0.10, 0.15, 0.20, 0.25, 0.30]
PITCH_SHIFTS = [0.02, 0.03, 0.04, 0.05]

for speed_inc in SPEED_INCREMENTS:
    for pitch_shift in PITCH_SHIFTS:
        reset_game()
        set_tuning("DifficultySpeedIncrement", speed_inc)
        set_tuning("MusicPitchPerWave", pitch_shift)
        
        # Play through 3 waves
        for wave in range(3):
            fill_all_home_slots()  # scripted hop sequence
            screenshot(f"wave{wave}_speed{speed_inc}_pitch{pitch_shift}.png")
        
        state = get_state()
        log(f"speed={speed_inc}, pitch={pitch_shift}, final_wave={state['wave']}")
```

This produces a grid of 20 screenshot sets (5 speed x 4 pitch) that I can compare visually. The "5 minutes" claim is for the script execution — human review of the screenshots adds time, but that is still 100x faster than manually playing through 20 configurations.

**Specific tests I would write:**
1. **Wave progression screenshot comparison:** Screenshot same camera view at Wave 1, 3, 5, 7. Verify ground color change, speed line overlay, hazard saturation differ visibly.
2. **Audio tempo measurement:** Record gameplay audio at Wave 1 and Wave 5. Compare waveform period to verify pitch shift.
3. **Fanfare timing:** Trigger wave completion, take screenshots at 0s, 0.25s, 0.5s, 1.0s, 1.5s. Verify text animation progression.
4. **Death VFX visibility:** Trigger death, screenshot at 0.1s, 0.3s. Verify VFX subtends >5% of viewport (stakeholder requirement).

---

### 5. Stretch Items

**Multiplier visibility on HUD ("x3" display):** ~15 LOC. The multiplier value already exists on `UScoreSubsystem`. Just needs a `DrawMultiplier()` method in HUD that reads it and draws "x3" next to the score in a larger, yellow font. Trivial. Could be done in the same commit as signal 3 (wave fanfare) since both touch HUD.

**Death freeze frame + screen shake:** ~25 LOC across GameMode + HUD.
- Freeze: Set `DeltaTime = 0` for 0.05s after entering Dying state. Add `DeathFreezeRemaining` float to GameMode.
- Shake: Random camera offset (+/-5, +/-5, 0) for 0.2s, decaying amplitude. Add `ShakeRemaining` and `ShakeAmplitude` to the camera actor or HUD overlay offset.
- These are "juice" — they make death feel impactful rather than clinical. Medium priority but high perception-per-LOC ratio.

---

### Summary for Sprint 8

| Item | Priority | LOC | Depends On |
|------|----------|-----|------------|
| PlayUnreal automation | P0 | ~150 (DevOps) | Nothing |
| Score pop positioning fix | P0 | ~15 | Nothing |
| VFX visibility fix | P0 | ~30 | Nothing |
| Audio pitch/tempo per wave | P1 | ~10 | Nothing (can parallel) |
| Wave fanfare ceremony | P1 | ~50 | Nothing (can parallel) |
| Ground color temperature | P1 | ~40 | Nothing (can parallel) |
| Multiplier HUD display | Stretch | ~15 | Nothing |
| Death freeze + shake | Stretch | ~25 | Nothing |
| Speed lines overlay | Stretch | ~30 | Fanfare first |
| Hazard color intensification | Stretch | ~25 | Ground color first |
| PlayUnreal A/B tuning script | P1 | ~60 (Python) | PlayUnreal P0 |

**Total new game-feel LOC (P1 + stretch):** ~195. Well within the 200-line review gate per subsystem (signals 1-3 are separate subsystems: audio, HUD, ground).

I am ready to drive the difficulty perception implementation once PlayUnreal and VFX fixes are underway. I recommend Engine Architect as my navigator for the C++ work, and QA Lead as cross-domain reviewer.

</teammate-message>

## Assistant (17:53 UTC)

Excellent input from Game Designer. Let me analyze this against our agreements and the other proposals.

- **Used Read** on `Source/UnrealFrog/Public/Core/FroggerAudioManager.h`

- **Used Read** on `Source/UnrealFrog/Public/Core/UnrealFrogGameMode.h`

## Assistant (17:53 UTC)

Good. The Game Designer's proposal is thorough and well-structured. I note that AudioManager has no `SetPitchMultiplier` method yet but has a `MusicComponent` that would support it. The GameMode already has the wiring points (`OnRoundCompleteFinished`, `GetSpeedMultiplier`). Let me synthesize the Game Designer's input with my existing plan and send the response.

- **Used SendMessage**

- **Used SendMessage**

## Assistant (17:53 UTC)

I've processed the Game Designer's input and sent feedback. Now I'm waiting for either:
- Team-lead's go-ahead to write the plan
- Engine-architect's response on PlayUnreal technical questions

The IPM is progressing well. We have:
- XP Coach: full plan proposal with phasing, scope, assignments
- Game Designer: detailed difficulty perception design with priority ranking and acceptance criteria
- Pending: Engine Architect technical input on PlayUnreal implementation
