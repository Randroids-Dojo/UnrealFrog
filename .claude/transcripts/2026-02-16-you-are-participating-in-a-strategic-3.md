# Conversation: 2026-02-16

| Field | Value |
|-------|-------|
| Date | 2026-02-16 |
| Session ID | `623ed273-2316-4fd8-a700-c10fd07213ff` |
| Model | claude-opus-4-6 |
| Tokens | ~974,463 input, ~152 output |
| Duration | 2m 1s |

---

## User (03:30 UTC)

<teammate-message teammate_id="team-lead" summary="Game Designer: One-prompt advantage analysis">
You are participating in a strategic retrospective for the UnrealFrog project. Your task is Task #4: Analyze the "one prompt" advantage of web development.

## Context

The stakeholder created WebFrogger — a complete 3D Frogger game using Three.js — in a SINGLE PROMPT. Here's what it produced:

**WebFrogger (1 prompt, 1 file, 1,311 lines):**
- Full 3D scene with perspective camera
- Road section with 5 lanes of colored vehicles (buses, trucks, cars) moving at different speeds
- River section with logs and turtles (some submerging)
- 5 home slots at the top
- Frog character with hop animation
- Score, level, lives HUD
- Timer bar
- Title screen, game over screen, level complete screen
- Touch controls for mobile
- Multiple levels with increasing difficulty
- All working. All visible. All playable.

**UnrealFrog (80 commits, 11,393 lines, 8 sprints):**
- Equivalent gameplay systems exist in C++
- 170+ unit tests pass
- But: VFX spawned at world origin for 7 sprints. Score pops were in wrong position. Visual changes still partially unverified.

## Your Analysis Task

1. **What makes the web platform enable single-prompt games?** Think about:
   - Immediate visual feedback (save → refresh → see)
   - Single-file architecture (no header/implementation split, no build system)
   - No binary asset pipeline (colors are CSS values, meshes are Three.js geometry)
   - The browser as universal preview tool
   - Declarative styling (CSS) vs procedural (C++ material system)
   - Three.js: 3D graphics with web-dev ergonomics

2. **What lessons can UnrealFrog learn from WebFrogger?** Even within UE5 constraints:
   - Could we use a more "declarative" approach to UE development?
   - What if game parameters were in config files instead of C++ constants?
   - What if the agent could describe what it wants and a tool translates to UE5?
   - How do we reduce the "surface area" of UE5 knowledge an agent needs?

3. **What would "one-prompt Unreal" look like?** Is it even possible? What would need to change — in the engine, in the tooling, in the agent workflow — to get close?

Read the WebFrogger source to understand what was built:
- `/Users/randroid/Documents/Dev/WebFrogger/index.html` (the entire game)

Write your analysis and send it to xp-coach via SendMessage. Then mark Task #4 as completed.

IMPORTANT: This is a RESEARCH and ANALYSIS task. Do NOT write any code. Read files, think deeply, and produce a written analysis.
</teammate-message>

## Assistant (03:30 UTC)

I'll start by reading the WebFrogger source and my agent memory, then analyze the one-prompt advantage.

- **Used TaskList**

- **Used TaskUpdate** on task 4

- **Used Read** on `/Users/randroid/Documents/Dev/WebFrogger/index.html`

## Assistant (03:30 UTC)

Now let me read the current game design spec and some key source files to ground my comparison.

- **Used Read** on `Docs/Design/sprint1-gameplay-spec.md`

- **Used Read** on `.claude/agent-memory/game-designer/MEMORY.md`

## Assistant (03:31 UTC)

Now let me look at the actual scale of our codebase to quantify the gap more precisely.

- **Used Bash**: Count total lines in UnrealFrog source

- **Used Bash**: Count total source files

- **Used Bash**: Count test lines

- **Used Bash**: Count WebFrogger lines

- **Used Bash**: Count total commits

## Assistant (03:31 UTC)

Now I have everything I need. Let me compose the analysis from the Game Designer perspective.

- **Used SendMessage**

- **Used TaskUpdate** on task 4

## Assistant (03:32 UTC)

Analysis sent to xp-coach and Task #4 marked complete.

**Summary of the analysis:**

The core finding is that the web platform's advantage is not about language or framework -- it is about **the distance between intent and outcome**. WebFrogger has zero distance: `color: 0x22cc22` means the thing IS green. UE5 has enormous distance: making a mesh green requires 4 concepts, 2 files, and a known landmine (the "BaseColor" vs "Color" parameter name we burned a sprint on).

Five concrete lessons for UnrealFrog:
1. **Data tables as single source of truth** -- config files that ARE the design doc, eliminating spec/code divergence
2. **Recipe pattern for visuals** -- describe actors as lists of primitives, not UE5 API call sequences  
3. **Collapse the wiring problem** -- fewer delegates, fewer layers where "defined but not bound" can hide
4. **Sound as code** -- Web Audio-style synthesis beats our file pipeline for arcade sound
5. **Near-zero verification tax** -- if it takes 60+ seconds to verify, it will be skipped (proven over 7 sprints)

The honest assessment: true "one-prompt Unreal" is not possible. But "one-prompt UE5 prototype" is achievable with a declarative scene format + runtime loader + instant PlayUnreal verification.
