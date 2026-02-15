# QA Lead Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 5 State (current)
- 24 test files, 148 total tests (0 failures) across 17 categories
- New test files this sprint: VFXTest.cpp (7), HUDTest.cpp (7+4 title-specific), AudioTest.cpp (6 new music tests), SeamTest.cpp (5 new seams: 8-12)
- Sprint goal: "Make UnrealFrog feel like an arcade cabinet -- music loops, particle feedback, styled HUD, title screen"
- **PLAY-TEST HAS NOT HAPPENED** -- BLOCKER per agreements section 5a

## Sprint 5 Critical Findings

### Confirmed Dead Code: TickVFX()
- `UFroggerVFXManager::TickVFX()` is defined (FroggerVFXManager.cpp:164) but NEVER called from GameMode::Tick or anywhere else
- VFX actors spawn at StartScale and never animate (no scale lerp, no rise)
- Mitigated by `Actor->SetLifeSpan(5.0f)` -- effects auto-destroy, just don't animate
- Grep results: only references are the definition and the header declaration

### Music Looping Risk
- `USoundWaveProcedural` with `bLooping = true` and one-shot `QueueAudio()` is unverified
- QueueAudio fills an internal buffer; GeneratePCMData drains it. When the buffer empties, there is no re-queue callback
- Risk: music plays once and stops, or has silence at loop point
- Music tracks exist: Content/Audio/Music/Music_Title.wav, Music_Gameplay.wav

### HUD: Score Pop Hardcoded Position
- `FScorePop.Position = FVector2D(160.0f, 10.0f)` -- always top-left regardless of screen size
- Should be relative to where "SCORE:" is drawn (20.0f, 10.0f)

### HUD Tests Don't Test DrawHUD
- All 11 HUD tests use `NewObject<AFroggerHUD>()` -- no Canvas, no GetWorld()
- Score pop detection, wave announcement, and timer pulse all duplicate DrawHUD logic inline in the tests
- Title screen (`DrawTitleScreen()`) cannot be tested without Canvas

### VFX Tests Only Verify Null Safety
- 7 VFX tests: 1 init, 4 null-world safety, 1 disabled flag, 1 delegate compat
- Zero tests verify actual VFX spawning (because GetWorld() returns nullptr)
- Zero tests for TickVFX animation logic

### Seam Matrix Current State
- 17 entries: 12 COVERED, 5 DEFERRED
- Sprint 5 added seams 8-12 (audio+GM, VFX+frog, HUD+score, audio+mute)
- Missing seam: VFX TickVFX integration (nobody calls TickVFX so the seam doesn't exist yet)

## Defect Escape Rate Trend
- Sprint 2: 5% (4 critical bugs from 81 tests)
- Sprint 3: 2% (2 bugs from 101 tests)
- Sprint 4: 1.6% (2 bugs from 123 tests)
- Sprint 5: UNKNOWN (play-test not done -- expect at least TickVFX and music looping bugs)

## Key File Locations (updated Sprint 5)
- VFX Manager: /Source/UnrealFrog/Public/Core/FroggerVFXManager.h, Private/Core/FroggerVFXManager.cpp
- Audio Manager: /Source/UnrealFrog/Public/Core/FroggerAudioManager.h, Private/Core/FroggerAudioManager.cpp
- HUD: /Source/UnrealFrog/Public/Core/FroggerHUD.h, Private/Core/FroggerHUD.cpp
- GameMode wiring: /Source/UnrealFrog/Private/Core/UnrealFrogGameMode.cpp (lines 98-117 = VFX wiring)
- Seam matrix: /Docs/Testing/seam-matrix.md
- Tests: /Source/UnrealFrog/Tests/ (24 files)
- Agreements: /.team/agreements.md
- Retro log: /.team/retrospective-log.md

## Spec-vs-Implementation Mismatches Found (Sprint 2 IPM)
10 mismatches identified between sprint1-gameplay-spec.md and current code/tests:
1. Game state enum: spec has 6 states, code has 4 (missing Spawning, Dying, RoundComplete; has Paused which spec defers)
2. Multiplier: spec says +1 per hop (1x,2x,3x,4x,5x cap), code uses +0.5f
3. MaxLives: spec says 9, code says 5
4. Time bonus: spec says RemainingSeconds*10, code uses (Remaining/Max)*1000
5. Round-complete 1000 bonus: spec requires it, no code/test exists
6. WavesPerGapReduction: spec says every 2 waves, code says every 3
7. Speed cap at 2.0x: spec requires it, no code enforces it
8. GridToWorld offset: spec uses (Col-6)*100 offset, code does not apply offset
9. BackwardHopMultiplier: spec says all directions equal at 0.15s, code adds 1.5x backward penalty
10. Lane direction: spec says Row1=LEFT, formula in test gives Row1=RIGHT (odd=right)

## Integration Test Gaps (historic, most now addressed)
- No Frog+Hazard collision in live world -- PARTIALLY ADDRESSED by seam tests
- No Frog+ScoreSubsystem cross-system test -- ADDRESSED in IntegrationTest.cpp
- No GameMode+ScoreSubsystem communication test -- ADDRESSED in IntegrationTest.cpp
- No LaneManager actual spawning test (only config queries) -- STILL MISSING
- No input-to-movement pipeline test -- STILL MISSING
- No timer-triggers-death end-to-end test -- ADDRESSED in DelegateWiringTest.cpp
