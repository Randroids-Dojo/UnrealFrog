# Claude Code Team Agents Setup Guide

## Original Request
Explain how the new Claude code team agents work and how I can easily get setup

---

## Clarification Questions

Before diving into a team agents setup, consider these key questions:

### 1. Which Team Agent Features Interest You Most?
Are you looking to explore:
- **Orchestration/coordination capabilities** between multiple Claude agents, or
- **Individual agent configuration** for different roles (like SPARC framework agents — planner, implementer, tester)?

### 2. What's Your Primary Use Case?
Are you setting this up for:
- Personal development workflows?
- Integration into existing CI/CD pipelines?
- A new game development project?

### 3. What's Your Current Claude Code Setup?
Are you:
- Already using Claude Code extensively with MCP servers and custom configurations?
- Looking for a fresh setup guide from scratch?

---

# Claude Code agent teams: the complete setup and workflow guide

Claude Code's **Agent Teams** is an experimental multi-agent orchestration feature that lets multiple independent Claude Code instances coordinate on shared codebases through a task list, messaging system, and team lead architecture. Shipped as a research preview on **February 5, 2026** alongside the Claude Opus 4.6 release, it transforms Claude Code from a single-session tool into a coordinated development squad — particularly powerful for large codebases like game engines. Enabling it requires a single environment variable, and teams are created through natural language instructions. This guide covers everything from architecture to Unreal Engine workflows.

## How agent teams coordinate and delegate work

Agent Teams introduces four core components that work together. The **Team Lead** is your main Claude Code session — it creates the team, spawns teammates, assigns tasks, and synthesizes results. **Teammates** are fully independent Claude Code instances, each running in their own context window with complete tool access. The **Shared Task List** provides a centralized work queue with dependency tracking and automatic unblocking. The **Mailbox System** enables peer-to-peer messaging between any agents, not just back to the lead.

This architecture differs fundamentally from Claude Code's existing **subagents** (the Explore, Plan, and General-purpose agents that run within a single session). Subagents report only to their caller and cannot communicate with each other. Agent Teams teammates are peers — they message each other directly, self-claim tasks from the shared queue, challenge each other's findings, and operate as genuinely independent sessions.

| Feature | Subagents | Agent Teams |
|---------|-----------|-------------|
| Context | Own window; results summarized to caller | Own window; fully independent session |
| Communication | Report to parent only | Peer-to-peer messaging between any agents |
| Coordination | Parent manages everything | Shared task list with self-coordination |
| User interaction | Through parent only | Direct interaction with any teammate |
| Token cost | Lower (results summarized) | **~5× higher** (each is a full instance) |
| Best for | Focused tasks needing only the result | Complex work requiring discussion and collaboration |

The system supports five orchestration patterns. **Hub-Spoke** (most common) has the leader spawn workers who report back for synthesis. **Task Queue** lets workers self-assign from a shared queue — ideal for embarrassingly parallel work. **Pipeline** chains agents sequentially (A → B → C) with handoffs. **Debate/Competition** assigns the same task to multiple agents and the leader selects the best solution. **Watchdog** pairs a worker with a monitor agent that can trigger rollbacks for critical operations.

File coordination uses lock-based task claiming stored at `~/.claude/teams/{team-name}/config.json` for team metadata, `~/.claude/tasks/{team-name}/` for the task list, and `~/.claude/teams/{team-name}/inboxes/` for messages.

## Enabling and configuring agent teams step by step

**Installation** (if not already installed):
```bash
# Native install (recommended, no Node.js required)
curl -fsSL https://claude.ai/install.sh | bash

# Or via Homebrew
brew install --cask claude-code

# Or via npm
npm install -g @anthropic-ai/claude-code
```

**Enabling Agent Teams** requires setting one environment variable. Add it to your shell or persist it in settings:

```bash
# Shell environment (temporary)
export CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1

# Persistent via settings.json (~/.claude/settings.json or .claude/settings.json)
{
  "env": {
    "CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS": "1"
  }
}
```

**Creating a team** uses natural language — no configuration files needed for basic usage:
```
Create an agent team to refactor the payment module. Spawn three teammates:
- one for the API layer
- one for the database migrations
- one for test coverage
Have them coordinate through the shared task list.
```

**Display modes** control how you see teammate activity. **In-process mode** (default) runs all teammates in the main terminal with keyboard navigation: `Shift+Up/Down` to select teammates, `Enter` to view a teammate's session, `Ctrl+T` to toggle the task list, and `Escape` to interrupt. **Split-pane mode** gives each teammate its own terminal pane via tmux or iTerm2:

```json
{ "teammateMode": "tmux" }
```

**Delegate Mode** (`Shift+Tab`) restricts the lead to coordination-only tools — spawning, messaging, task management, and shutdown — preventing it from implementing code itself. This enforces clean separation between orchestration and execution.

**Hooks** provide programmatic control over the team lifecycle. Configure them in `.claude/settings.json`:

```json
{
  "hooks": {
    "TeammateIdle": [{ "command": "./scripts/assign-next-task.sh" }],
    "TaskCompleted": [{ "command": "./scripts/quality-gate.sh" }]
  }
}
```

A `TeammateIdle` hook that exits with code 2 sends feedback to keep the teammate working. A `TaskCompleted` hook exiting with code 2 blocks completion and returns feedback for revision.

**Known limitations**: teammates consume **~5× the tokens** of a single session. No session resumption for in-process teammates (`/resume` won't restore them). One team per session, no nested teams. Teammates inherit the lead's permissions and cannot spawn their own teams.

## Structuring CLAUDE.md and agents for team workflows

CLAUDE.md files are loaded into every Claude Code session's context — including every teammate. This makes them the primary mechanism for ensuring consistent behavior across a team. Keep the project-level CLAUDE.md concise (**under 150–200 instructions**) and focused on what every agent needs to know.

**Recommended project CLAUDE.md structure:**
```markdown
# Project: [Name]
[Engine/framework], [language], [key dependencies]. [One-sentence purpose.]

## Architecture
- /src/gameplay/ — Core gameplay systems
- /src/networking/ — Multiplayer replication
- /src/ui/ — HUD and menu systems
- /tests/ — Automated test suites

## Conventions
- [Specific style rules agents must follow]
- [Naming conventions]
- [Error handling patterns]

## Commands
- `[build command]` — Build the project
- `[test command]` — Run test suite
- `[lint command]` — Check code quality

## Critical Rules
- NEVER modify [protected files/directories]
- Always run tests before marking tasks complete
- [Security/safety constraints]
```

**Custom subagents** live as Markdown files in `.claude/agents/` with YAML frontmatter defining their capabilities:

```markdown
---
name: gameplay-engineer
description: Implements C++ gameplay systems following UE5 conventions
tools: Read, Write, Edit, Bash, Grep, Glob
model: opus
skills:
  - ue5-gameplay
  - game-architecture
---

You are a senior gameplay engineer. When implementing features:
1. Follow Unreal naming conventions (A- for Actors, U- for Objects, F- for structs)
2. Use UPROPERTY/UFUNCTION macros for Blueprint exposure
3. Always run compilation check after edits
4. Write unit tests for new gameplay logic
```

```markdown
---
name: ue5-reviewer
description: Reviews code changes for UE5 best practices and performance
tools: Read, Grep, Glob, Bash
model: sonnet
---

You are a code reviewer specializing in Unreal Engine. Focus on:
- Memory management (prevent leaks, proper garbage collection)
- Replication correctness for multiplayer
- Blueprint/C++ interface integrity
- Performance implications (tick functions, allocations)
```

The **skills system** provides modular, reusable capabilities that agents invoke automatically based on semantic matching. Skills live in `.claude/skills/[skill-name]/SKILL.md`:

```markdown
---
name: ue5-gameplay
description: Unreal Engine 5 gameplay programming patterns and conventions
context: fork
---

## Gameplay Ability System
[Reference patterns for GAS implementation]

## Replication
[Networking replication patterns and RPCs]

## Component Architecture
[Actor-component patterns and best practices]
```

The recommended project structure for a multi-agent game development workflow:

```
my-game/
├── CLAUDE.md                          # Core project context (all agents read this)
├── CLAUDE.local.md                    # Personal local overrides (gitignored)
├── .claude/
│   ├── settings.json                  # Agent teams enabled, permissions, hooks
│   ├── agents/
│   │   ├── gameplay-engineer.md       # C++ gameplay implementation
│   │   ├── blueprint-specialist.md    # Blueprint creation and review
│   │   ├── ue5-reviewer.md           # Code review with UE5 lens
│   │   ├── test-writer.md            # Test creation for game systems
│   │   └── performance-agent.md      # Profiling and optimization
│   ├── skills/
│   │   ├── ue5-gameplay/SKILL.md     # GAS, replication, components
│   │   ├── ue5-blueprint/SKILL.md    # Blueprint conventions
│   │   ├── ue5-testing/SKILL.md      # Testing patterns
│   │   └── ue5-performance/SKILL.md  # Optimization guidelines
│   ├── commands/
│   │   ├── build-test.md             # Build and test workflow
│   │   └── review-pr.md             # PR review workflow
│   └── rules/
│       ├── gameplay/conventions.md    # Gameplay coding rules
│       └── networking/replication.md  # Replication rules
└── .mcp.json                         # MCP server configuration
```

## Game development workflows with agent teams

Game projects are particularly well-suited to multi-agent development because their systems are often **modular and independently testable** — gameplay, UI, networking, AI, and audio rarely need to touch the same files simultaneously. Anthropic demonstrated the power of parallel agents by using **16 Claude Opus 4.6 instances** to build a 100,000-line Rust-based C compiler across ~2,000 sessions, consuming **2 billion input tokens** at roughly **$20,000 in API cost**.

For Unreal Engine specifically, several integration tools exist. **CLAUDIUS** ($49.99) provides **130+ commands across 19 categories** (Level, Blueprint, AI, Sequencer, Animation, PCG) with three communication methods and a built-in CLAUDE.md. **UnrealClaude** (open source, UE 5.7) embeds Claude Code directly in the editor with **20+ MCP tools** for actor manipulation, Blueprint editing, and level management.

**Practical multi-agent patterns for game development:**

A **Cross-Layer Feature** team splits agents by domain — one for C++ gameplay, one for Blueprints, one for UI, one for tests. Each agent maintains focused context on its domain rather than trying to hold the entire codebase. For a new ability system, the gameplay engineer implements the C++ GAS components, the Blueprint specialist creates the visual scripting layer, and the test writer builds verification tests — all simultaneously.

A **Parallel Bug Investigation** team assigns different hypotheses to different agents. Three teammates each investigate a different theory for why a networking bug occurs — connection pool exhaustion, race condition in caching, memory leak in the handler — then share evidence via the mailbox system and argue which theory fits the data. This avoids the **anchoring bias** that plagues sequential investigation.

A **Pipeline** pattern chains agents sequentially for quality-gated workflows: the architect agent produces a design spec, the implementation agent builds it, the review agent evaluates the result, and the test agent verifies correctness.

**When to use single-agent vs. agent teams:**

| Scenario | Recommendation |
|----------|---------------|
| Single gameplay feature implementation | Single agent |
| One known bug fix | Single agent |
| Refactoring across multiple game systems | Agent teams |
| Parallel development of independent features | Agent teams |
| Code review from multiple perspectives | Agent teams |
| Large-scale UE version migration | Agent teams |
| Prototyping competing gameplay approaches | Agent teams |

Key lessons from Anthropic's C compiler project apply directly to game development. **Write extremely high-quality tests** — agents optimize for whatever verification you provide, so your test harness must be nearly perfect. **Design for agent orientation** by maintaining extensive READMEs and progress tracking files, since each agent starts with no conversation history. **Avoid context window pollution** by keeping build output minimal (print summaries, log details to files). **Enforce file ownership boundaries** — two agents editing the same file leads to overwrites, which is critical for UE5's header/implementation file pairs.

## Iterating on skills and CLAUDE.md over time

Skills and CLAUDE.md files are **living documents** that improve through deliberate iteration. The most effective approach follows a **plan → work → review → compound** cycle. After each multi-agent session, review where agents went wrong or needed correction, then encode those corrections as explicit rules in CLAUDE.md, refined skill instructions, or new subagent constraints.

For skill organization across agents, maintain clear separation of concerns. Each skill should cover **one domain** (gameplay patterns, replication conventions, testing approaches) rather than mixing concerns. Subagents reference skills via the `skills:` frontmatter field, so a `gameplay-engineer` agent automatically loads `ue5-gameplay` and `game-architecture` skills while the `performance-agent` loads `ue5-performance`. Keep individual SKILL.md files **under 500 lines** and use a `references/` subdirectory for detailed API documentation — this progressive disclosure pattern prevents context bloat.

The `TeammateIdle` and `TaskCompleted` hooks create natural quality gates. A `TaskCompleted` hook that runs your compilation check and test suite can automatically reject incomplete work with specific feedback, forcing the teammate to fix issues before marking a task done. For UE5 projects, hook into the build system to auto-compile after C++ edits and run gameplay tests after implementation changes.

Use **git worktrees** when running multiple Claude Code sessions simultaneously on different branches to prevent file conflicts. Each teammate can operate in its own worktree, committing independently, with the lead managing merges. This is especially valuable for game projects where build times are long and parallel branches represent genuinely independent features.

## Conclusion

Agent Teams transforms Claude Code from a powerful single-agent tool into an orchestrated development team, with the shared task list and mailbox system providing coordination that subagents fundamentally lack. The feature is still experimental and **token-expensive** (~5× single-session cost), making it best reserved for genuinely parallelizable work — cross-system refactors, competing debugging hypotheses, or simultaneous feature development across independent modules. For game development on Unreal Engine, the combination of Agent Teams with tools like CLAUDIUS or UnrealClaude, structured CLAUDE.md files, and domain-specific subagent definitions creates a workflow where specialized agents handle gameplay, Blueprints, testing, and performance optimization in parallel. The critical insight from Anthropic's own 16-agent C compiler project: success depends less on the orchestration infrastructure and more on **test quality, file ownership discipline, and context-aware agent instructions** — invest heavily in those foundations before scaling to larger teams.



# Second prompt

# Claude Code Agent Team for Unreal Game Development

## Research References

### John Wilger's Agent Team Setup
- **Repository**: https://github.com/jwilger/tidyup-tango
- **Approach**: Created dream team profiles (software engineers, UI/UX specialist, product manager, a11y specialist) stored in markdown files within a `.team` directory
- **Process**: Full mob-programming with Day 0 team agreement formation, retrospectives after each feature, and dynamic team member recruitment as needed
- **Key Insight**: Team self-improvement through retrospectives led to dramatic process optimization and high-quality output

### Your Godot Game Development Projects
- **GoDig**: https://github.com/Randroids-Dojo/GoDig
- **PlayGodot**: https://github.com/Randroids-Dojo/PlayGodot

---

## Proposed Unreal Engine Agent Team

### Required Expertise Areas & Candidate Agents

**C++ Expertise**
- Research: John Carmack (legendary engine/graphics expertise)
- Alternative: Linus Torvalds or other systems-level C++ experts

**Game Design**
- Research: Shigeru Miyamoto, Warren Spector, or Toby Fox (indie excellence)

**Extreme Programming / Agile / Lean**
- **Kent Beck** (XP pioneer, already used by John Wilger)
- Alternative: Ron Jeffries

**Level Design**
- Research: Tim Willits (Quake/Doom level design), Chris Metzen (narrative design)

**Game Artist / Art Direction**
- Research: Concept artists like Craig Mullins or engine artists

**Sound Engineering / Audio**
- Research: Wwise/FMOD experts or game audio pioneers

**Test Automation / DevEx / DevOps**
- Research: GitHub Actions/CI/CD specialists, build system experts
- Could combine with C++ expertise for custom tool development

**Game QA / Playtesting**
- Research: Professional QA leads, accessibility specialists

---

# Unreal Game Development Agent Team - Project Definition

## Clarification Questions & Your Answers

**Game scope and genre**: Are you thinking a simple arcade-style game (like a runner or puzzle game), a small 3D exploration game, or something else? This affects whether we need specialists in specific gameplay systems (combat, physics puzzles, etc.) vs. more generalist designers.
- **Your answer**: 3D Frogger, arcade style

**Asset creation autonomy**: Do you want agents that can actually generate/create game assets (3D models, textures, sounds) via tools/APIs, or agents that define specifications for assets that would be created externally? This determines if we need "asset creation tooling" specialists vs. "asset direction" specialists.
- **Your answer**: Actually create assets, at least placeholders. See GoDig docs for your attempt at this. Do the required additional research as needed—or better yet, define an agent who will do the research and tool building.

**Real-time vs. turn-based development**: Should the team work in mob programming style like John's setup where they coordinate in real-time through retrospectives, or more of a parallel task queue approach where specialists work independently on their domains?
- **Your answer**: Definitely real-time like John's. Let them come up with their own working agreements like John's did. Have them retrospect


# Building an autonomous Claude Code agent team for Unreal Engine Frogger

**A team of specialized AI agents, inspired by real game industry legends and modeled on proven multi-agent coordination patterns, can autonomously build a 3D Frogger arcade game in Unreal Engine — complete with generated assets, self-improving processes, and mob programming discipline.** This report synthesizes research from John Wilger's pioneering tidyup-tango agent team, Randroids-Dojo's Godot automation tooling, current asset generation APIs, and Claude Code's native multi-agent architecture to propose a complete, actionable team design. The key insight: **the team's ability to self-improve through retrospectives and modify its own configuration files matters more than any individual agent's initial instructions.**

---

## How Wilger's tidyup-tango proved agents can self-organize

John Wilger's tidyup-tango — a Rust/HTMX todo app described as "100% generated by AI" — represents the most sophisticated public example of Claude Code agent team formation. While the repository's internal `.team/` and `.claude/` directories couldn't be accessed directly, Wilger's surrounding ecosystem reveals the architecture clearly.

**Five repositories form the stack.** `caxton` (8 stars, updated daily) is an LLM agent host using **FIPA (Foundation for Intelligent Physical Agents)** protocol for multi-agent coordination, with WebAssembly-compiled agents and multi-node clustering. [GitHub](https://github.com/jwilger?tab=repositories) `claude-code-plugins` contains his global Claude Code configuration [github](https://github.com/jwilger?tab=repositories) (hooks, settings, skills). `agent-skills` provides reusable TypeScript skill definitions under CC0 license. `opencode-plugin-team-agreements` is the critical piece — a dedicated plugin for "establishing and maintaining team agreements for human-LLM collaboration," [github](https://github.com/jwilger?tab=repositories) proving that team agreements are encoded as structured, versionable artifacts, not just conversational norms.

The pattern that emerges follows **mob programming culture** (Wilger is a Senior Engineering Leader with experience in this practice). [LinkedIn](https://www.linkedin.com/in/johnwilger/) Agents operate under team agreements that constrain behavior — the "one-driver rule" means only one agent writes code at a time while others navigate. **Retrospectives modify configuration files directly**: agents analyze what went wrong, update their own CLAUDE.md and team agreement files, and the next iteration runs with improved rules. This mirrors Kent Beck's XP principle of "pick your worst problem, solve it, repeat." [Reilly's Blog](https://blog.reilly.dev/what-i-learned-from-extreme-programming-explained-by-kent-beck) Wilger's `gh-project-ext` and `gh-issue-ext` GitHub CLI extensions handle Kanban board management and sub-issue tracking, [github](https://github.com/jwilger?tab=repositories) giving agents structured task management beyond just code.

The self-improvement loop works like this: agents work → encounter friction → hold a retrospective (likely encoded as a skill or session protocol) → identify process failures → modify their own `.team/agreements.md` or agent profile files → next session operates under updated rules. Commit hygiene improvements from first to second iteration would follow naturally — the retrospective identifies messy commits, the team agreement is updated to require atomic commits with conventional commit messages, and a `PreToolUse` hook enforces the new standard.

---

## What Randroids-Dojo reveals about validation-first game development

Randy Lutcavich's Randroids-Dojo organization takes a different but complementary approach. The `GoDig` repository appears to be private or recently created, and `PlayGodot` is actually a **game automation framework** (like Playwright for games, not a game itself), [GitHub](https://github.com/Randroids-Dojo/Godot-Claude-Skills) but the publicly accessible **Godot-Claude-Skills** repository provides the clearest example of how to structure Claude Code skills for game engine development.

The SKILL.md file encodes a critical insight: *"Godot's text-based files are LLM-friendly when you respect the syntax differences."* [Claude Code Plugins](https://claude-plugins.dev/skills/@bfollington/terma/godot) The skill provides **validation scripts** (`validate_tres.py`, `validate_tscn.py`) that run before any commit [FastMCP](https://fastmcp.me/Skills/Details/235/godot) — a validation-first workflow where generated code must pass structural checks before integration. Templates for game components (attribute systems, interaction patterns, spell/item resources) give agents scaffolding to build from, rather than generating everything from scratch. [Claude Code Plugins](https://claude-plugins.dev/skills/@bfollington/terma/godot)

The CI/CD pipeline automates the full loop: install Claude CLI → install Godot 4.3.0 with export templates → install GdUnit4 testing framework → run all tests [GitHub](https://github.com/Randroids-Dojo/Godot-Claude-Skills) → build web export → deploy to Vercel. **PlayGodot extends this with E2E game testing** — launching games headless, waiting for nodes, calling methods remotely, simulating input, and capturing screenshots. [GitHub](https://github.com/Randroids-Dojo/Godot-Claude-Skills) This proves that autonomous game testing by AI agents is achievable today.

For Unreal Engine, the equivalent pattern would be: validation scripts that check `.uasset` references and naming conventions, UE's built-in Automation Testing Framework for gameplay tests, and Python Editor Scripting for asset import validation. The key lesson is **validate early and automatically** — every generated asset and code change must pass automated checks before any agent considers it "done."

---

## The expert-inspired agent roster

Each agent's personality, decision-making framework, and coding philosophy is drawn from a real industry expert. This isn't cosmetic — encoding specific methodological principles into agent profiles produces measurably different behavior than generic role descriptions.

### Engine Architect — inspired by John Carmack

The Engine Architect writes C++ using Carmack's "C with Classes" subset: no exceptions, no RTTI, minimal templates, no multiple inheritance. [CppDepend:](https://cppdepend.com/blog/?p=744) Every function aims for purity. The agent profiles worst-case frame times, not averages, and asks "can I redefine this problem?" before optimizing. [X](https://x.com/ID_AA_Carmack/status/1509998003024707589) Tim Sweeney's influence adds: always provide visual editor tools alongside systems, and design for single-threaded simplicity unless a proven concurrent architecture exists. This agent owns the core game loop, player controller, and Unreal subsystem integration.

### Game Designer — inspired by Miyamoto and Iwatani

The Game Designer prototypes mechanics before aesthetics, designs the first level as a wordless tutorial (Miyamoto's World 1-1 principle), and ensures controls respond within **100ms**. From Iwatani: the core loop must be explainable in one sentence [Toy Tales](https://toytales.ca/toru-iwatani-the-creator-of-pac-man/) ("cross lanes of traffic and rivers to reach home"), inputs are minimal (directional + hop), [Fandom](https://pacman.fandom.com/wiki/Toru_Iwatani) each enemy type has distinct readable AI behavior, and difficulty follows wave patterns — tension, release, escalation. Score systems reward skill mastery and risk-taking. Every death must feel fair.

### XP Coach — inspired by Kent Beck and Woody Zuill

The XP Coach enforces TDD (Red-Green-Refactor) as the primary development methodology and facilitates mob programming coordination. From Beck: "optimism is an occupational hazard of programming; feedback is the treatment." [Goodreads](https://www.goodreads.com/author/quotes/25211.Kent_Beck) From Zuill: "for an idea to go from someone's head into the computer, it must go through someone else's hands" — the one-driver rule. This agent runs retrospectives, updates team agreements, and ensures the team completes one feature before starting another (WIP limits). It owns the `Stop` hook that triggers retrospective analysis.

### Level Designer — inspired by Emilia Schatz

The Level Designer starts every level with colored blockmesh (not gray — colors communicate material intent), maps each section to an emotion/theme chart before building, and uses spatial compression for tension and expansion for triumph. From Schatz's Naughty Dog work: shape language must be consistent (if one thing is climbable, all similar things must be climbable), and cultural affordances (recognizable real-world objects) are preferred over explicit UI prompts. [80.lv](https://80.lv/articles/defining-environment-language-for-video-games) For Frogger specifically: design for readability at speed, clear lane boundaries, visible hazard patterns, and distinct zone identities.

### Sound Engineer — inspired by Koji Kondo and Andy Farnell

The Sound Engineer never composes in isolation — audio must be prototyped while viewing gameplay. From Kondo: music tempo and rhythm match character movement speed; [Game Music Themes](https://gamemusicthemes.com/composers-and-soundtrack-spotlights/the-genius-of-koji-kondo-shaping-nintendos-sound-legacy/) sound effects and music form a unified system. [Shmuplations](https://shmuplations.com/kojikondo/) From Farnell: sound is a process, not data [Designing Sound](https://designingsound.org/2012/01/18/procedural-audio-interview-with-andy-farnell/) — parameterized procedural models that respond to gameplay variables (speed, impact force, distance) generate infinite variations without additional memory. [Amazon UK](https://www.amazon.co.uk/Designing-Sound-Press-Andy-Farnell/dp/0262014416) [Procedural-audio](http://www.procedural-audio.com/) For Frogger: satisfying hop sounds, traffic whooshes with Doppler shifts, water splashes, and a catchy looping theme that doesn't fatigue.

### Art Director — inspired by Greg Foertsch

The Art Director establishes visual "dogmas" (style guide rules) in pre-production and validates all assets against them. From Foertsch's XCOM work: use slightly exaggerated proportions and chunky geometry for stylized 3D readability, design assets for multi-view readability (top-down gameplay camera and close-up replay camera), [Game Developer](https://www.gamedeveloper.com/production/art-direction-bootcamp-an-expert-roundtable-q-a-2) and maintain a modular art pipeline. The art direction for Frogger should favor **low-poly stylized aesthetics** — they age well, optimize performance, speed production, and create distinctive visual identity. [Vsquad](https://vsquad.art/blog/mastering-indie-game-art-styles-a-developers-guide)

### DevOps Engineer — inspired by Robert Nystrom

The DevOps Engineer enforces architectural patterns from *Game Programming Patterns*: Service Locator for decoupled subsystems, Command pattern for all player actions (enabling undo, replay, and automated test scripting), Observer for cross-system communication, and a deterministic game loop (fixed timestep) for reproducible automated tests. This agent owns the CI pipeline, build system, asset validation pipeline, and Gauntlet test framework integration. [30 Days Coding](https://30dayscoding.com/blog/game-development-with-ci-cd)

### QA Lead — inspired by Dr. John Hopson and Steve Swink

The QA Lead implements automated telemetry (death locations, completion times, path choices), evaluates game feel using Swink's six-component taxonomy (input, response, context, polish, metaphor, rules), [Taylor & Francis](https://www.taylorfrancis.com/chapters/mono/10.1201/9781482267334-25/principles-game-feel-steve-swink) and generates severity × frequency matrices for issue prioritization. From Hopson's Bungie work: "clear all experience problems away from the fun." [Reed College](https://www.reed.edu/beyond-reed/worksdays/posts/2014/interview-with-john-hopson.html) For every player action, the QA Lead verifies the presence of visual effect, sound effect, camera response, and the "polish density" that makes arcade games feel satisfying.

---

## The asset generation pipeline that makes autonomy possible

The biggest technical challenge for autonomous game development is **real asset generation** — creating 3D models, textures, and sounds programmatically. The tooling landscape in 2026 makes this feasible.

**For 3D models**, the recommended primary tool is **Meshy AI** (REST API, FBX/GLB/OBJ output, native UE plugin, PBR textures, adjustable polygon count from 1K–300K faces, ~$20–60/month). Hyper3D Rodin offers higher quality for hero assets (~$0.30–0.40 per generation, [WaveSpeedAI](https://wavespeed.ai/models/hyper3d/rodin-v2/text-to-3d) SIGGRAPH-award-winning topology). [Skypage](https://skywork.ai/skypage/en/Hyper3D-Review-(2025)-My-Deep-Dive-into-AI-Image-to-3D-Model-Generation/1974392702218465280) For procedural geometry (lane markers, platforms, simple obstacles), **Blender headless** (`blender -b -P script.py`) provides full programmatic control with direct FBX export. The pipeline: AI API generates asset → trimesh validates mesh integrity [GitHub](https://github.com/mikedh/trimesh) → Blender converts/optimizes → UE Python imports with `AssetImportTask`.

**For textures**, **Scenario** is purpose-built for game development, [MOGE](https://moge.ai/product/scenario) generating complete PBR map sets (albedo, normal, roughness, metallic, [Scenario](https://www.scenario.com/blog/ai-texture-generation) height, AO) via REST API [Scenario](https://help.scenario.com/en/articles/texture-generation) — used by Ubisoft in production. [Amazon Web Services](https://aws.amazon.com/solutions/case-studies/scenario-case-study/) Poly Haven's CC0 library provides 100+ photoscanned PBR texture sets at 8K+ resolution via public API for environmental surfaces. [Poly Haven](https://polyhaven.com/) [Poly Haven](https://polyhaven.com/all) Python's Pillow/NumPy can generate simple procedural textures (noise patterns, gradients) for placeholders.

**For sound effects**, **ElevenLabs SFX V2** (September 2025) generates professional-quality 48kHz audio from text descriptions, supporting looping and duration control up to 30 seconds. For retro arcade sounds, **rFXGen** runs as a single CLI executable (~1.5MB) with presets for coin, explosion, powerup, hit, and jump sounds in WAV format. [GitHub](https://github.com/raysan5/rfxgen) The combination covers both realistic and stylized audio needs.

**The import automation** uses UE's Python Editor Script Plugin (production-ready in UE 5.7):

```python
import unreal
task = unreal.AssetImportTask()
task.filename = "path/to/frog_character.fbx"
task.destination_path = "/Game/Characters/Frog"
task.automated = True
unreal.AssetToolsHelpers.get_asset_tools().import_asset_tasks([task])
```

The Data Validation system then runs custom validators checking naming conventions, [Unreal Engine Community](https://unrealcommunity.wiki/data-validators-74b9c7) texture power-of-2 dimensions, collision setup, and LOD presence. The complete pipeline — generate → validate → convert → import → validate again → integrate — can run entirely from agent-executed Python scripts.

---

## Complete project configuration structure

The project repository should follow this directory layout:

```
FroggerUE/
├── CLAUDE.md                          # Project overview, build commands, standards
├── .claude/
│   ├── settings.json                  # Hooks, permissions, experimental flags
│   ├── agents/
│   │   ├── engine-architect.md        # Carmack-inspired C++ systems agent
│   │   ├── game-designer.md           # Miyamoto/Iwatani arcade design agent
│   │   ├── xp-coach.md               # Beck/Zuill process coordination agent
│   │   ├── level-designer.md          # Schatz-inspired spatial design agent
│   │   ├── sound-engineer.md          # Kondo/Farnell audio agent
│   │   ├── art-director.md            # Foertsch-inspired visual standards agent
│   │   ├── devops-engineer.md         # Nystrom-inspired pipeline agent
│   │   └── qa-lead.md                 # Hopson/Swink game feel testing agent
│   ├── skills/
│   │   ├── unreal-conventions/SKILL.md
│   │   ├── unreal-build/SKILL.md
│   │   ├── asset-generation/SKILL.md
│   │   ├── frogger-design/SKILL.md
│   │   └── retrospective/SKILL.md
│   └── agent-memory/                  # Persistent memory per agent
│       ├── engine-architect/MEMORY.md
│       ├── game-designer/MEMORY.md
│       └── ...
├── .team/
│   ├── agreements.md                  # Team working agreements (self-modifying)
│   ├── retrospective-log.md           # Record of retrospectives and changes
│   ├── roster.md                      # Current team roster and roles
│   └── onboarding.md                  # How new specialist agents get added
└── Source/, Content/, Config/         # Standard UE project structure
```

The **CLAUDE.md** should be concise (bloated files cause instruction-following degradation) [Claude](https://code.claude.com/docs/en/best-practices) and point to skills for detailed knowledge:

```markdown
# FroggerUE - 3D Frogger Arcade Game

## Project
Unreal Engine 5.4 C++ project. 3D arcade Frogger with procedurally generated assets.

## Build
- `UnrealBuildTool FroggerUE Development Win64` — build
- `UnrealEditor-Cmd FroggerUE.uproject -ExecCmds="Automation RunTests Frogger"` — test

## Standards  
- C++ follows UE coding standards (UPROPERTY, UFUNCTION macros mandatory)
- TDD: write test → implement → refactor. No code without a failing test.
- One agent writes at a time (mob programming). Others review.
- Assets generated via pipeline in `.claude/skills/asset-generation/`

## Team
Read `.team/agreements.md` before ANY work. Update it during retrospectives.
@.team/agreements.md
@.claude/skills/unreal-conventions/SKILL.md
```

---

## Self-improvement through encoded retrospectives

The retrospective mechanism is the most important design element. It works through three coordinated systems.

**The `retrospective` skill** (`.claude/skills/retrospective/SKILL.md`) defines a structured protocol: review the git log since last retrospective → identify friction points (failed builds, reverted commits, repeated mistakes) → categorize into process issues vs. technical issues → propose specific changes to team agreements or agent profiles → apply changes → commit the updated configuration files. This skill runs with `context: fork` so it executes in an isolated subagent that can read the full project history without consuming the main session's context. [Claude](https://code.claude.com/docs/en/skills)

**The `Stop` hook** triggers retrospective evaluation. After the XP Coach agent's session ends, the hook runs a script that checks whether a retrospective is due (based on time elapsed or features completed). If due, it injects a prompt into the next session: [Claude](https://code.claude.com/docs/en/hooks-guide) "Run the retrospective skill before starting new work."

**Agent memory** provides continuity across sessions. Each agent's `MEMORY.md` accumulates learned patterns: [Claude](https://code.claude.com/docs/en/sub-agents) "UE's `AddDynamic` macro requires UFUNCTION — always verify before binding delegates" or "Meshy API returns inconsistent normals on organic shapes — always run trimesh validation post-download." The first 200 lines of MEMORY.md are automatically loaded into every session, giving agents institutional knowledge that improves over time. [Claude](https://code.claude.com/docs/en/sub-agents)

The team agreement evolution follows a concrete pattern. **Iteration 1** might have a loose agreement: "Commit working code frequently." After a retrospective reveals large, multi-file commits that are hard to revert, **Iteration 2** updates the agreement to: "Each commit touches one logical change. Commit message follows Conventional Commits format. The `PreToolUse` hook on `Bash(git commit)` validates the message format." The hook is added to `settings.json` as part of the retrospective's output. The team literally rewrites its own rules and enforcement mechanisms.

---

## Mob programming coordination for agent teams

Claude Code's experimental **Agent Teams** feature [Claude Fast](https://claudefa.st/blog/guide/agents/agent-teams) (enabled via `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1`) provides the native infrastructure for mob programming. [Claude](https://code.claude.com/docs/en/agent-teams) The architecture uses a **Team Lead** session that coordinates **Teammate** instances communicating through mailboxes. [Claude Fast](https://claudefa.st/blog/guide/agents/agent-teams) [Claude](https://code.claude.com/docs/en/agent-teams)

The optimal configuration uses **Delegate Mode** (Shift+Tab): the Team Lead loses Edit/Write/Bash tools and can only spawn teammates, send messages, and manage tasks. [Claude Fast](https://claudefa.st/blog/guide/agents/agent-teams) [Medium](https://medium.com/@haberlah/configure-claude-code-to-power-your-agent-team-90c8d3bca392) This forces pure coordination — the Lead becomes the navigator, and teammates become drivers. The one-driver rule is enforced architecturally: only one teammate has write permissions to any given file at a time, managed through **directory-level ownership** boundaries.

For the Frogger project, a typical mob session works like this: the XP Coach (as Team Lead in delegate mode) assigns a feature to implement ("add log-riding mechanic"). The Engine Architect teammate plans the C++ class structure. The Game Designer teammate specifies the feel parameters (acceleration, deceleration, log speed variance). The DevOps Engineer teammate writes the test. Then the Engine Architect teammate implements — the only one with write tools active. Other teammates review via read-only tools and send messages with corrections. The QA Lead teammate runs the test suite and reports results.

**Recruiting additional specialists** happens through the onboarding protocol in `.team/onboarding.md`. When the team identifies a knowledge gap (e.g., "we need a shader specialist for the water rendering"), the XP Coach creates a new agent definition file in `.claude/agents/` with a focused role description, adds it to the roster, and spawns it as a teammate for the relevant feature. The new agent reads the team agreements and project CLAUDE.md, inheriting all accumulated team norms. [claude](https://code.claude.com/docs/en/sub-agents) [Claude API Docs](https://platform.claude.com/docs/en/agent-sdk/subagents) This mirrors how Wilger's tidyup-tango team could dynamically add "consultants."

---

## Practical hooks and settings configuration

The `.claude/settings.json` encodes the enforcement layer:

```json
{
  "env": {
    "CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS": "1"
  },
  "hooks": {
    "PreToolUse": [{
      "matcher": "Write|Edit",
      "hooks": [{
        "type": "command",
        "command": "python3 .claude/scripts/validate_ue_naming.py"
      }]
    }],
    "PostToolUse": [{
      "matcher": "Bash(git commit)",
      "hooks": [{
        "type": "command",
        "command": "python3 .claude/scripts/validate_commit_msg.py"
      }]
    }],
    "Stop": [{
      "matcher": "",
      "hooks": [{
        "type": "command",
        "command": "python3 .claude/scripts/check_retrospective_due.py"
      }]
    }],
    "SessionStart": [{
      "matcher": "",
      "hooks": [{
        "type": "command",
        "command": "echo 'Read .team/agreements.md. One driver at a time. TDD always.'"
      }]
    }]
  }
}
```

The `SubagentStart` and `SubagentStop` hooks can log which agents are active, [GitHub](https://github.com/disler/claude-code-hooks-mastery) [Eesel AI](https://www.eesel.ai/blog/hooks-reference-claude-code) preventing violations of the one-driver rule. The `PreCompact` hook backs up the full conversation transcript for retrospective analysis before context compaction discards older messages. [GitHub](https://github.com/disler/claude-code-hooks-mastery) [Claude](https://code.claude.com/docs/en/hooks-guide)

---

## Conclusion

The convergence of three developments makes this architecture viable in 2026: Claude Code's native Agent Teams with inter-agent messaging [Claude](https://code.claude.com/docs/en/agent-teams) and persistent memory; [Claude](https://code.claude.com/docs/en/sub-agents) production-ready asset generation APIs (Meshy, Scenario, ElevenLabs) that output UE-compatible formats; and Unreal Engine's mature Python scripting for automated import and validation. The Wilger pattern — team agreements as versionable code, retrospectives that modify agent configurations, mob programming enforced through tooling constraints — transforms what would otherwise be a chaotic multi-agent system into a disciplined, self-improving team.

The most counterintuitive lesson from this research is that **restricting agent capabilities improves outcomes**. Delegate Mode removes write tools from the coordinator. [Medium](https://medium.com/@haberlah/configure-claude-code-to-power-your-agent-team-90c8d3bca392) The one-driver rule prevents concurrent file edits. TDD forces implementation to follow specification. These constraints, borrowed from decades of XP and mob programming practice, prevent the failure modes that plague unconstrained multi-agent systems: [Claude](https://code.claude.com/docs/en/best-practices) merge conflicts, architectural drift, untested code, and broken builds. The agents don't need more freedom — they need better agreements, and the ability to improve those agreements themselves.
