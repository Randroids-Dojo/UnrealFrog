# DevOps Engineer Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Build System
- UE 5.7 at /Users/Shared/Epic Games/UE_5.7/
- Editor binary: /Users/Shared/Epic Games/UE_5.7/Engine/Binaries/Mac/UnrealEditor
- UnrealEditor-AutomationDriver.dylib exists (can use for PlayUnreal)
- UnrealEditor-AutomationController.dylib exists
- FunctionalTesting module available in engine (Developer/FunctionalTesting)
- Run tests via: UnrealEditor <project> -ExecCmds="Automation RunTests UnrealFrog" -unattended -nopause -nullrhi
- Build times: Editor ~12s, Game ~26s

## Architecture Notes
- AFrogCharacter extends APawn (not ACharacter)
- AUnrealFrogGameMode extends AGameModeBase
- UScoreSubsystem extends UGameInstanceSubsystem
- UFroggerAudioManager extends UGameInstanceSubsystem
- UFroggerVFXManager extends UGameInstanceSubsystem
- ALaneManager extends AActor
- AHazardBase extends AActor
- All classes expose both dynamic (BP) and native (C++) delegates
- Build.cs public deps: Core, CoreUObject, Engine, InputCore, EnhancedInput, UMG
- Build.cs private deps: Slate, SlateCore

## Sprint 8 State (Current)
- 162 tests passing, 0 failures, 17 categories (as of Sprint 7 end)
- run-tests.sh: --all, --seam, --audio, --e2e, --integration, --wiring, --vfx, --list, --report, --functional
- run-tests.sh now has mkdir-based lock file (/tmp/.unrealFrog_test_lock) to prevent concurrent runs (Section 19)
- RemoteControl plugin enabled in .uproject (Sprint 8 Task 2)
- Binary assets in git: Content/Audio/Music/*.wav (~4.4 MB), Content/Audio/SFX/*.wav
- Seam matrix at Docs/Testing/seam-matrix.md

## Remote Control API (Sprint 8 Spike Findings)
- Plugin: Engine/Plugins/VirtualProduction/RemoteControl/
- WebRemoteControl module: Type=Runtime, PlatformAllowList: Mac, Win64, Linux
- Default HTTP port: 30010 (URemoteControlSettings::RemoteControlHttpServerPort)
- In -game mode: REQUIRES `-RCWebControlEnable` flag (disabled by default outside editor)
- bAutoStartWebServer defaults true; with -RCWebControlEnable the server auto-starts
- Alternative startup: CVar `WebControl.EnableServerOnStartup=1` or console cmd `WebControl.StartServer`
- SearchActor/SearchObject routes: 501 Not Implemented in UE 5.7
- Object resolution: StaticFindObject() by exact UE object path
- Functions: must be UFUNCTION(BlueprintCallable) for /remote/object/call
- CDO path: /Script/UnrealFrog.Default__UnrealFrogGameMode
- Live instance path: /Game/Maps/<MapName>.<MapName>:PersistentLevel.<ClassName>_0
- NullRHI blocks RC API entirely (FApp::CanEverRender() check)
- Launch cmd: UnrealEditor.app ... -game -windowed -RCWebControlEnable -ExecCmds="WebControl.EnableServerOnStartup 1"

## Sprint 6 Retro Decisions
- M_FlatColor.uasset: DROPPED (Section 17 applied). Runtime fallback sufficient. Re-create if packaging becomes a goal.
- Functional tests in CI: DROPPED (Section 17 applied). NullRHI headless tests sufficient.
- Per-subsystem commits enforced and working (Section 4).
- Solo driver with review gates formally adopted (Section 1 updated).
- NEW Section 18: Post-implementation review — domain expert reviews diff before commit.

## Sprint 7 Plan (Agreed)
- Tight scope: play-test + tuning + quick fixes only
- P0: Full gameplay play-test (QA Lead owns, 2 sprints overdue)
- P0: Pre-flight cleanup in run-tests.sh (stale process kill before test launch)
- P1: Stale process detection in run-tests.sh
- Tuning + multiplier HUD prioritized over GameCoordinator refactor (defer to Sprint 8)
- PlayUnreal E2E rename to "Scenario" still carried forward
- P2: Screenshot capture in play-game.sh
- P2: Basic CI pipeline (GitHub Actions, compile + test on push)

## Known Issues Carrying Forward
- Music looping: bLooping=true on USoundWaveProcedural + single QueueAudio() call. Untested in live playback. QueueAudio buffer may drain without re-fill callback. Sprint 7 play-test should verify.
- No CI pipeline exists. Manual build/test only.
- PlayUnreal E2E tests are scenario tests (direct method calls), NOT true E2E (binary launch + input injection). Cannot replace manual play-testing.
- Stale process kill dance (pkill UnrealTraceServer, pkill UnrealEditor) is manual — needs to be automated in run-tests.sh.

## Retro Lessons (Stable Patterns)
- Two sprints of unverified gameplay is a quality debt that compounds.
- PlayUnreal E2E tests verify state machine logic but NOT rendering, animation, or audio playback.
- Section 17 (deferred item deadline) works — it forced honest resolution of 5-sprint-old items.
- Per-subsystem commits are enforceable and valuable for rollback granularity.
- Domain-expert reviewer assignment is better than "any agent" for post-implementation review.
