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
- ALaneManager extends AActor
- AHazardBase extends AActor
- All classes expose both dynamic (BP) and native (C++) delegates
