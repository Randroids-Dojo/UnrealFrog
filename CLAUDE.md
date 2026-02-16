# UnrealFrog - 3D Frogger Arcade Game

## Mission

**Prove that AI agents can build real games in real engines as effectively as they build web apps.** The game (Frogger) is the proving ground. The tools (PlayUnreal, auto-screenshot, spatial assertions, feedback loop infrastructure) are the product.

The benchmark: [WebFrogger](/Users/randroid/Documents/Dev/WebFrogger/) — a complete 3D Frogger built with Three.js in a single prompt. One file, 1,311 lines, instant result. Road with vehicles, river with logs, home slots, HUD, title screen, touch controls. All working, all visible, all playable. That is what "good" looks like for agentic game development.

UnrealFrog exists to close the gap between that experience and Unreal Engine 5. Every process improvement and tooling investment should be evaluated against one question: **does this close the gap between agentic web development and agentic Unreal development?**

## Project
Unreal Engine 5.7 C++ project. 3D arcade Frogger with procedurally generated assets.
Low-poly stylized aesthetic. Mob programming workflow with agent team coordination.

## Architecture
- `/Source/UnrealFrog/` — C++ gameplay source (actors, components, subsystems)
- `/Content/` — Unreal assets (meshes, materials, maps, sounds)
- `/Config/` — Project configuration (DefaultEngine.ini, DefaultGame.ini)
- `/Source/UnrealFrog/Tests/` — Automated test suites (170+ tests)
- `/Tools/PlayUnreal/` — Live game testing: screenshots, video, state queries, hop commands
- `/Tools/AudioGen/` — Procedural WAV generation (SFX + music)
- `/Docs/` — Design specs, sprint plans, QA checklists, testing docs
- `/.claude/agents/` — Agent team profiles
- `/.claude/skills/` — Shared knowledge skills
- `/.team/` — Team agreements, roster, retrospective log

## Build (macOS, UE 5.7)
- Game target: `"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" UnrealFrog Mac Development "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject"`
- Editor target: `"/Users/Shared/Epic Games/UE_5.7/Engine/Build/BatchFiles/Mac/Build.sh" UnrealFrogEditor Mac Development "/Users/randroid/Documents/Dev/Unreal/UnrealFrog/UnrealFrog.uproject"`
- **Both targets must succeed before any commit.**

## Standards
- C++ follows UE coding standards (UPROPERTY, UFUNCTION macros mandatory)
- Unreal naming: A- for Actors, U- for UObjects, F- for structs, E- for enums
- TDD: write test -> implement -> refactor. No code without a failing test.
- One agent writes at a time. All agents contribute opinions — no solo decisions.
- Assets generated via pipeline in `.claude/skills/asset-generation/`
- Conventional Commits format for all commit messages

## Agent Team Workflow

### Always Use Agent Teams
Agent teams are the default for ALL tasks — even small ones. Every task benefits from differing opinions. A single agent working alone produces blind spots; multiple perspectives catch errors, surface better designs, and prevent anchoring bias. The token cost is worth it.

### Starting a Team Session
1. Agent Teams is enabled via `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1` in `.claude/settings.json`
2. Use **Delegate Mode** (`Shift+Tab`) — the Team Lead coordinates only, teammates implement
3. Spawn at minimum: xp-coach + domain expert + one challenger (an agent from a different domain who will question assumptions)
4. Team Lead assigns driver per task based on domain expertise

### Agent Roster (`.team/roster.md`)
| Agent | When to Spawn |
|-------|--------------|
| xp-coach | Always — Team Lead, facilitates process and retrospectives |
| engine-architect | C++ systems, core game loop, subsystems, physics |
| game-designer | Mechanics tuning, feel parameters, progression design |
| devops-engineer | Build system, test infra, CI/CD, validation scripts |
| qa-lead | Test coverage, play-testing, game feel verification |
| level-designer | Map layout, zone design, spatial readability |
| art-director | Visual assets, materials, style guide enforcement |
| sound-engineer | Audio assets, SFX, music, procedural audio |

### Multi-Perspective on Every Task
Every task goes through three phases where differing opinions are required:

1. **Design debate** — Before any code is written, at least 2 agents propose approaches via mailbox. The Team Lead synthesizes or picks the best. Disagreement is encouraged.
2. **Active navigation** — While the driver implements, at least 1 other agent actively reviews in real-time via messages (not after the fact). Navigators question design choices, flag edge cases, and suggest alternatives.
3. **Cross-domain review** — Before committing, an agent from a *different* domain than the driver reviews the change. Engine Architect reviews Game Designer's tuning values; QA Lead reviews Engine Architect's systems code; etc.

The **one-writer-per-file rule** still applies for file safety — but "one writer" does not mean "one opinion." The driver writes; the team thinks.

### Orchestration Patterns
- **Debate-first** (default): Before implementation, 2+ agents propose competing approaches. Lead synthesizes the best. Use this for every non-trivial task.
- **Hub-Spoke**: Lead spawns workers for parallel subtasks, they report back for synthesis
- **Task Queue**: Workers self-assign from shared task list — for embarrassingly parallel work
- **Pipeline**: Sequential handoff (architect → implementer → reviewer → tester) — for quality-gated features

### File Ownership
Each agent owns specific directories (defined in `.team/roster.md`). Two agents must NEVER edit the same file. This prevents merge conflicts and overwrites — critical for UE5 header/implementation pairs.

### The Retrospective Cycle
After every feature/sprint: review git log → identify friction → categorize (process vs technical) → propose concrete changes to `.team/agreements.md` or agent profiles → commit updated config. The team literally rewrites its own rules. This is the most important mechanism — see `.claude/skills/retrospective/SKILL.md`.

### Key Constraints
- Teammates consume **~5x tokens** of a single session — spawn judiciously
- No session resumption for teammates (`/resume` won't restore them)
- One team per session, no nested teams
- Teammates inherit the Lead's permissions
- Keep build output minimal — print summaries, log details to files (prevents context pollution)

### Essential References

**Team process** (read before any work):
- `.team/agreements.md` — Living team norms (21 sections, battle-tested over 8 sprints)
- `.team/roster.md` — Agent roles and file ownership boundaries
- `.team/onboarding.md` — How to add/retire specialist agents
- `.team/retrospective-log.md` — Full history of process improvements
- `.claude/agents/*.md` — Individual agent profiles with expert-inspired principles

**PlayUnreal — live game testing** (read before any visual/gameplay work):
- `Tools/PlayUnreal/README.md` — Full usage guide: client API, test scripts, video capture, troubleshooting
- `Tools/PlayUnreal/client.py` — Python client: `hop()`, `get_state()`, `screenshot()`, `record_video()`
- `Tools/PlayUnreal/verify_visuals.py` — Visual smoke test (run after ANY visual system change)
- `Tools/PlayUnreal/diagnose.py` — Deep RC API diagnostics (run when things are broken)
- `Tools/PlayUnreal/qa_checklist.py` — Sprint QA gate (required before "QA: verified")
- `Tools/PlayUnreal/run-playunreal.sh` — Launcher: handles editor lifecycle + RC API setup

**Game design and specs**:
- `Docs/Design/sprint1-gameplay-spec.md` — Grid system, lane layout, movement, scoring rules
- `.claude/skills/frogger-design/SKILL.md` — Core loop, controls, progression, difficulty philosophy

**Testing**:
- `Docs/Testing/seam-matrix.md` — Seam test coverage matrix (all system interaction pairs)
- `Tools/PlayUnreal/run-tests.sh` — Unit test runner (170+ tests, category filters: `--seam`, `--audio`, `--e2e`, `--all`)

**Build and UE5 conventions**:
- `.claude/skills/unreal-conventions/SKILL.md` — Naming, UPROPERTY/UFUNCTION patterns, lifecycle
- `.claude/skills/unreal-build/SKILL.md` — Build.cs, test framework, automation patterns

**Asset pipelines**:
- `.claude/skills/asset-generation/SKILL.md` — 3D model, texture, and audio asset validation
- `Tools/AudioGen/generate_sfx.py` — Procedural 8-bit SFX generation (9 WAVs)
- `Tools/AudioGen/generate_music.py` — Procedural music generation (Title + Gameplay tracks)

**Sprint planning and history**:
- `Docs/Planning/sprint*-plan.md` — Sprint plans (2, 4, 7, 8)
- `Docs/QA/sprint2-playtest-checklist.md` — QA playtest checklist template
- `Docs/Research/editor-automation-spike.md` — PythonScriptPlugin + editor automation research

## Team
Read `.team/agreements.md` before ANY work. Update it during retrospectives.

## Critical Rules
- NEVER modify engine source files
- NEVER commit broken builds — verify build (Game + Editor) before every commit
- NEVER bypass the asset validation pipeline
- Always run tests before marking tasks complete
- Run `verify_visuals.py` after ANY change to visual systems (VFX, HUD, materials, camera, lighting)
- One writer per file — but every task gets multiple opinions before, during, and after
- Retrospect after each feature completion
- Every sprint must leave the project in a playable, working state

@.team/agreements.md
@.claude/skills/unreal-conventions/SKILL.md
