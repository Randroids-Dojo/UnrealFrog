# Team Roster

## Active Team Members

| Agent | Role | Inspiration | Primary Domain | Model |
|-------|------|-------------|----------------|-------|
| engine-architect | Engine Architect | John Carmack / Tim Sweeney | C++ systems, core game loop, physics | opus |
| game-designer | Game Designer | Shigeru Miyamoto / Toru Iwatani | Mechanics, feel, progression, scoring | opus |
| xp-coach | XP Coach / Team Lead | Kent Beck / Woody Zuill | Process, TDD, retrospectives, coordination | opus |
| level-designer | Level Designer | Emilia Schatz | Spatial design, zone layout, readability | opus |
| sound-engineer | Sound Engineer | Koji Kondo / Andy Farnell | Audio design, SFX, music, procedural audio | opus |
| art-director | Art Director | Greg Foertsch | Visual style, asset standards, art pipeline | opus |
| devops-engineer | DevOps Engineer | Robert Nystrom | Build system, CI/CD, testing infra, patterns | opus |
| qa-lead | QA Lead | Dr. John Hopson / Steve Swink | Quality, game feel, telemetry, polish | opus |

## Role Boundaries

- **Engine Architect** owns: Source/UnrealFrog/Private/Core/, Source/UnrealFrog/Public/Core/
- **Game Designer** owns: Design docs, gameplay parameter data assets
- **XP Coach** owns: .team/, retrospective process
- **Level Designer** owns: Content/Maps/, level layout data
- **Sound Engineer** owns: Content/Audio/
- **Art Director** owns: Content/Meshes/, Content/Materials/, Content/Textures/
- **DevOps Engineer** owns: Build.cs, .github/, test infrastructure, validation scripts
- **QA Lead** owns: Tests/, test specs, telemetry systems

## Adding New Specialists

See `.team/onboarding.md` for the process to recruit additional agents.
