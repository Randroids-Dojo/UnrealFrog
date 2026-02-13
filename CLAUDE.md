# UnrealFrog - 3D Frogger Arcade Game

## Project
Unreal Engine 5.4 C++ project. 3D arcade Frogger with procedurally generated assets.
Low-poly stylized aesthetic. Mob programming workflow with agent team coordination.

## Architecture
- /Source/UnrealFrog/ — C++ gameplay source (actors, components, subsystems)
- /Content/ — Unreal assets (meshes, materials, maps, sounds)
- /Config/ — Project configuration (DefaultEngine.ini, DefaultGame.ini)
- /Tests/ — Automated test suites (Gauntlet, functional tests)
- /.claude/agents/ — Agent team profiles
- /.claude/skills/ — Shared knowledge skills
- /.team/ — Team agreements, roster, retrospective log

## Build
- `UnrealBuildTool UnrealFrog Development Win64` — build
- `UnrealEditor-Cmd UnrealFrog.uproject -ExecCmds="Automation RunTests UnrealFrog"` — test

## Standards
- C++ follows UE coding standards (UPROPERTY, UFUNCTION macros mandatory)
- Unreal naming: A- for Actors, U- for UObjects, F- for structs, E- for enums
- TDD: write test -> implement -> refactor. No code without a failing test.
- One agent writes at a time (mob programming). Others review.
- Assets generated via pipeline in `.claude/skills/asset-generation/`
- Conventional Commits format for all commit messages

## Team
Read `.team/agreements.md` before ANY work. Update it during retrospectives.

## Critical Rules
- NEVER modify engine source files
- NEVER commit broken builds
- NEVER bypass the asset validation pipeline
- Always run tests before marking tasks complete
- One driver at a time — mob programming is mandatory
- Retrospect after each feature completion

@.team/agreements.md
@.claude/skills/unreal-conventions/SKILL.md
