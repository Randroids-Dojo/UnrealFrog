# Team Working Agreements

*This is a living document. Updated during retrospectives by the XP Coach.*
*Last updated: Sprint 2 Retrospective*

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
- **Build verification is mandatory before every commit.** Run the UBT build for BOTH Game AND Editor targets and confirm `Result: Succeeded` before `git commit`. Never build just one target — the `-game` flag uses Editor binaries. (Updated Sprint 2: building only the Game target caused debugging confusion.)

### 5. Feature Workflow

1. XP Coach breaks down the feature into tasks with dependency graph
2. Team discusses approach (all agents)
3. **Same agent writes test AND implementation** (prevents API mismatch — learned Sprint 1)
4. Domain expert drives implementation
5. All agents review
6. **Build verification** — UBT build must succeed (Game + Editor targets)
7. Tests pass → merge
8. **QA Lead play-tests** using PlayUnreal automation harness (Added: Stakeholder Review)
9. XP Coach runs retrospective

### 5a. Definition of Done (Added: Stakeholder Review)

A sprint is NOT done until:
- The project builds successfully (Game + Editor targets)
- All automated tests pass
- **Visual smoke test passes** after foundation phase: game launches, ground visible, player visible, camera correct, HUD renders (Added Sprint 2: three visual bugs were invisible to unit tests)
- The game is playable — you can press Play and interact with the game
- QA Lead has verified gameplay via PlayUnreal or manual play-testing
- The project is left in a working state for the next sprint

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

### 8. PlayUnreal Test Harness (Added: Stakeholder Review)

- Agents must be able to launch the game and interact with it programmatically
- Build a PlayUnreal automation tool (similar to PlayGodot) that can:
  - Launch the editor with a test map
  - Send input commands (hop, pause, etc.) via Remote Control API or Automation Driver
  - Capture game state (frog position, score, lives, game state)
  - Take screenshots for visual verification
- QA Lead and Game Designer use PlayUnreal to validate gameplay feel
- DevOps Engineer owns the PlayUnreal infrastructure

### 9. Visual Smoke Test After Foundation (Added: Sprint 2 Retrospective)

After foundation systems are in place (actors, meshes, camera, ground), **launch the game before writing integration code** and verify:
- [ ] Can you see the ground plane?
- [ ] Can you see the player actor?
- [ ] Does the camera show the full play area?
- [ ] Does the HUD render text?
- [ ] Is there lighting in the scene?

If any fail, fix them immediately. Do NOT proceed to integration tasks with an invisible game.

**Why:** Sprint 2 built 12 tasks before launching. Three critical visual bugs (no lighting, unwired HUD, wrong camera angle) were invisible to unit tests and required a full extra debugging session.

### 10. Scene Requirements for New Maps (Added: Sprint 2 Retrospective)

When creating a new map or empty scene via code, always ensure:
- A directional light exists (sun)
- Ambient lighting exists (sky light or fill light)
- Camera view target is explicitly set
- HUD is wired to game state (polling or delegates)
- Ground/floor geometry is present and lit

**Why:** An empty .umap has NO lights, NO sky, NO atmosphere. C++ auto-spawned meshes are invisible without lighting.

## Things We Will Figure Out Together

- ~~Optimal driver rotation timing~~ → Resolved: per-task, not time-based
- How to handle disagreements between agents
- When to bring in a specialist agent (Art Director, Sound Engineer, Level Designer not needed for Sprint 1)
- ~~How to balance speed vs. quality~~ → Resolved: quality gate is "playable and building" (Stakeholder Review)

## Resolved Questions (Sprint 1)

- **8 agents right?** Only 3 were needed for Sprint 1 (XP Coach, Engine Architect, DevOps Engineer). Art/Sound/Level agents are for later sprints with content.
- **Opus for all?** Engine Architect benefits from opus. DevOps scaffolding could use sonnet for speed. Evaluate per-task.
- **Driver rotation timing?** Per-task rotation works well. Engine Architect drove most implementation since it's all C++ systems.
- **WIP limits?** One feature at a time is sufficient. Allow parallel non-overlapping systems within a feature.
