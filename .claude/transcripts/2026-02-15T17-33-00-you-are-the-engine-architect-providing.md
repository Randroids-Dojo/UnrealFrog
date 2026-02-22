# Conversation: 2026-02-15

| Field | Value |
|-------|-------|
| Date | 2026-02-15 |
| Session ID | `9cad10df-a2ee-4650-adec-4a8034993a77` |
| Model | claude-sonnet-4-5-20250929 |
| Tokens | ~417,807 input, ~109 output |
| Duration | 1m 12s |

---

## User (17:33 UTC)

<teammate-message teammate_id="team-lead" summary="Engine Architect retro feedback">
You are the Engine Architect providing retrospective feedback on the Post-Sprint 7 Stakeholder Review.

## Context
The stakeholder played the game after Sprint 7 and found 3 critical issues that 162 automated tests missed:

1. **"No visible differences when playing compared to previous sprints."** — Difficulty scaling works at the code level (hazards speed up 15% per wave) but the player can't perceive it.

2. **"Long-standing bug: explosion barely visible at bottom-left, +score text barely visible at top-left."** — Score pops hardcoded to screen position `FVector2D(20 + textLen*10, 10)` in FroggerHUD.cpp:116 instead of frog's projected position. Death VFX (scale 0.5→3.0) too small from Z=2200 camera. Home slot celebrations hardcoded to `Col * 100, 14 * 100, 50`.

3. **"PlayUnreal is not a real tool."** — PlayUnrealTest.cpp calls methods directly, never sends real inputs. Need HTTP server + Python client for scripted gameplay.

## Your Task
Provide your Engine Architect perspective. Focus on:
- **Technical root cause**: Why are VFX too small? What's the correct approach for camera-distance-relative scaling?
- **Score pop fix**: How should world-to-screen projection work in UE5 Canvas HUD? (Canvas->Project? GetPlayerController()->ProjectWorldLocationToScreen?)
- **PlayUnreal architecture**: HTTP server vs Remote Control API vs PythonScriptPlugin — which is most practical for UE 5.7?
- **Estimated complexity**: How many lines of code / hours for each fix?

Keep your response to 15-25 lines. Be technical and specific. Send your response to team-lead via message.
</teammate-message>

## Assistant (17:33 UTC)

🤖️

I need to read the current state of the key files to provide informed technical feedback on these issues.

- **Used Read** on `Source/UnrealFrog/Private/Core/FroggerHUD.cpp`

- **Used Read** on `Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp`

- **Used Read** on `Tests/PlayUnrealTest.cpp`

- **Used Glob** for `**/PlayUnreal*.cpp` in `.`

- **Used Read** on `Source/UnrealFrog/Public/Core/FroggerHUD.h`

- **Used Read** on `Source/UnrealFrog/Tests/PlayUnrealTest.cpp`

- **Used Read** on `Tools/PlayUnreal/run-tests.sh` (offset=0, limit=100)

## Assistant (17:33 UTC)

Now I have enough context. Let me send my technical assessment to the team lead.

- **Used SendMessage**

## Assistant (17:34 UTC)

I've sent my technical analysis to the team lead covering all three stakeholder issues:

## Summary of My Feedback

**Issue #2 (VFX/Score Pop Bugs) — Root Causes Identified:**

1. **Death VFX too small:** Fixed `EndScale = 3.0f` creates only ~12% screen height from camera Z=2200. Need camera-relative scaling formula: `DesiredScale = CamDistance * tan(FOV/2 * ScreenFraction) * 2`, which yields EndScale ~10.0f for 25% visibility.

2. **Score pops at wrong position:** `/Source/UnrealFrog/Private/Core/FroggerHUD.cpp:116` uses hardcoded `FVector2D(20 + textLen*10, 10)` instead of projecting the frog's world location. Need `Canvas->Project(FrogWorldLocation)` to get proper screen coordinates.

3. **Home slot sparkles hardcoded:** `/Source/UnrealFrog/Private/Core/FroggerVFXManager.cpp:159` uses literal `FVector(Col*100, 14*100, 50)` instead of referencing grid constants.

**Issue #1 (Difficulty Scaling Invisible):** The 15% speed increase per wave (100→115 cm/s) is mathematically correct but perceptually meaningless. Need audio/visual cues: rising-pitch SFX, screen-edge particle burst, and red-tinted wave announcements for waves >3.

**Issue #3 (PlayUnreal Not Real):** Current `/Source/UnrealFrog/Tests/PlayUnrealTest.cpp` calls methods directly — it's a unit test, not E2E. Recommended **Remote Control API** (UE 5.7 built-in WebSocket plugin) over HTTP server (experimental) or PythonScriptPlugin (crashes kill harness). This gives real input simulation + state inspection with minimal custom code.

**Estimated effort:** ~275 lines total, ~7.5 hours (VFX/scoring: 2h, difficulty cues: 1.5h, PlayUnreal: 4h).

All three issues are fixable in a single sprint with clear technical paths forward.
