# Team Working Agreements

*This is a living document. Updated during retrospectives by the XP Coach.*
*Last updated: Sprint 1 Retrospective*

## Day 0 Agreements

These are our starting agreements. They WILL evolve through retrospectives.

### 1. Mob Programming Rules

- **One driver per file at a time.** Multiple drivers are OK if they touch zero overlapping files. (Updated Sprint 1: strict single-driver was unnecessarily slow for independent systems.)
- **Driver rotation**: The XP Coach assigns the driver for each task based on domain expertise.
- **Navigators review in real-time.** Don't wait until the end — flag issues as you see them.
- **Any agent can call "Stop"** if they see a problem. The driver must pause and discuss.

### 2. TDD is Mandatory

- Write the failing test FIRST (Red)
- Implement the minimum code to pass (Green)
- Refactor while tests still pass (Refactor)
- No code merges without passing tests

### 3. Communication Protocol

- Before implementing, post a 3-sentence plan and wait for acknowledgment
- Use the shared task list for work coordination
- Use direct messages for design discussions
- Disagreements are resolved by the domain expert (Engine Architect for C++, Game Designer for mechanics, etc.)

### 4. Commit Standards

- Conventional Commits format: `feat:`, `fix:`, `refactor:`, `test:`, `docs:`, `chore:`
- Each commit is one logical change
- Commit message explains WHY, not WHAT (the diff shows what)
- Never commit broken builds

### 5. Feature Workflow

1. XP Coach breaks down the feature into tasks with dependency graph
2. Team discusses approach (all agents)
3. **Same agent writes test AND implementation** (prevents API mismatch — learned Sprint 1)
4. Domain expert drives implementation
5. All agents review
6. Tests pass → merge
7. XP Coach runs retrospective

### 6. Asset Pipeline

- All generated assets go through the validation pipeline before import
- Art Director approves visual assets
- Sound Engineer approves audio assets
- DevOps Engineer verifies import automation works

### 7. Retrospectives

- Mandatory after each completed feature
- XP Coach facilitates using the retrospective skill
- Produces concrete changes to THIS document and/or agent profiles
- Changes are committed as part of the retrospective

## Things We Will Figure Out Together

- ~~Optimal driver rotation timing~~ → Resolved: per-task, not time-based
- How to handle disagreements between agents
- When to bring in a specialist agent (Art Director, Sound Engineer, Level Designer not needed for Sprint 1)
- How to balance speed vs. quality

## Resolved Questions (Sprint 1)

- **8 agents right?** Only 3 were needed for Sprint 1 (XP Coach, Engine Architect, DevOps Engineer). Art/Sound/Level agents are for later sprints with content.
- **Opus for all?** Engine Architect benefits from opus. DevOps scaffolding could use sonnet for speed. Evaluate per-task.
- **Driver rotation timing?** Per-task rotation works well. Engine Architect drove most implementation since it's all C++ systems.
- **WIP limits?** One feature at a time is sufficient. Allow parallel non-overlapping systems within a feature.
