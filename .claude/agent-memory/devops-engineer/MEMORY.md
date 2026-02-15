# DevOps Engineer Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 1 State (Completed)
- 5 test files, ~50 tests total: FrogCharacter, GameState, Score, LaneSystem, Collision
- All tests use IMPLEMENT_SIMPLE_AUTOMATION_TEST (not latent/functional)
- Tests use NewObject<> directly -- no world/level needed for unit tests
- Build.cs public deps: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG
- Build.cs private deps: Slate, SlateCore (added Sprint 2 for HUD widgets)
- No Content/Maps exist yet (no .umap files found)
- DefaultEngine.ini references /Game/Maps/MainMenu but map doesn't exist
- Config uses EnhancedInput for input system

## Build System
- UE 5.7 at /Users/Shared/Epic Games/UE_5.7/
- Editor binary: /Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor
- UnrealEditor-AutomationDriver.dylib exists (can use for PlayUnreal)
- UnrealEditor-AutomationController.dylib exists
- FunctionalTesting module available in engine (Developer/FunctionalTesting)
- Run tests via: UnrealEditor <project> -ExecCmds="Automation RunTests UnrealFrog" -unattended -nopause -nullrhi

## Architecture Notes
- AFrogCharacter extends APawn (not ACharacter)
- AUnrealFrogGameMode extends AGameModeBase
- UScoreSubsystem extends UGameInstanceSubsystem
- UFroggerAudioManager extends UGameInstanceSubsystem
- UFroggerVFXManager extends UGameInstanceSubsystem
- ALaneManager extends AActor
- AHazardBase extends AActor
- All classes expose both dynamic (BP) and native (C++) delegates

## Sprint 5 State
- 148 tests passing, 0 failures, 17 categories
- Build times: Editor ~12s, Game ~26s
- run-tests.sh supports: --all, --seam, --audio, --e2e, --integration, --wiring, --vfx, --list, --report, --functional
- Binary assets in git: Content/Audio/Music/*.wav (~4.4 MB), Content/Audio/SFX/*.wav
- Seam matrix at Docs/Testing/seam-matrix.md: 12 COVERED, 5 DEFERRED

## Known Issues Carrying Forward
- TickVFX() is defined in UFroggerVFXManager but NEVER called from Tick(). VFX spawn but don't animate (scale/rise). SetLifeSpan(5.0f) prevents leaks, but effects sit static then pop away.
- Music looping: bLooping=true on USoundWaveProcedural + single QueueAudio() call. Untested in live playback. QueueAudio buffer may drain without re-fill callback.
- M_FlatColor.uasset: 5th sprint without generating it. Runtime fallback works but WITH_EDITORONLY_DATA will fail in packaged builds.
- Functional tests (AFunctionalTest actors) still cannot run in CI (need rendering context).
- No packaging step in CI.

## Stale P1 Items (Tracking)
- M_FlatColor.uasset generation: deferred Sprints 3, 4, 5 (now 3 sprints)
- Functional tests in CI: deferred Sprints 3, 4, 5
- Packaging step in CI: deferred Sprints 4, 5
