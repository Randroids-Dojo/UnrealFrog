# Conversation: 2026-02-13

| Field | Value |
|-------|-------|
| Date | 2026-02-13 |
| Session ID | `8403401c-929b-45e5-9b1a-de3092b0cf7a` |
| Model | claude-opus-4-6 |
| Tokens | ~385,075 input, ~88 output |
| Duration | 2m 59s |

---

## User (04:02 UTC)

<teammate-message teammate_id="team-lead" summary="Game design spec for Sprint 1">
You are the Game Designer on the UnrealFrog team. Read `.team/agreements.md` first.

You are a NAVIGATOR for Sprint 1. You will NOT write C++ code — you will create the design specification that guides implementation.

## Task: Write the Sprint 1 Game Design Spec

Create a design document at `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Design/sprint1-gameplay-spec.md` that specifies EXACTLY what the Engine Architect and QA Lead need to implement and test.

Reference the frogger-design skill at `.claude/skills/frogger-design/SKILL.md` for the core design.

The spec should cover:

### 1. Grid System
- Cell size (100 UU recommended for clean math)  
- Grid dimensions: how many columns (13) and rows (15) for a classic Frogger layout
- World-space mapping: where is (0,0)? How do grid coords map to world coords?

### 2. Zone Layout (Bottom to Top)
Define each row's purpose:
- Rows 0: Start zone (safe)
- Rows 1-5: Road zone (5 traffic lanes)
- Row 6: Median (safe)
- Rows 7-12: River zone (6 river lanes)
- Rows 13-14: Goal zone (5 home slots at specific columns)

### 3. Movement Parameters
All values that AFrogCharacter needs:
- HopDuration, HopArcHeight, GridCellSize, InputBufferWindow
- Forward hop speed vs backward hop speed
- Movement cooldown

### 4. Hazard Parameters  
For each hazard type (Car, Truck, Bus, Motorcycle, Small Log, Large Log, Turtle Group):
- Speed (units/sec)
- Width (in grid cells)
- Spawn interval / gap size
- Direction per lane

### 5. Scoring Rules
- Points per action (exact values)
- Multiplier rules (when it increases, when it resets)
- Time bonus calculation
- Extra life thresholds

### 6. Game State Transitions
- State diagram with all transitions and their triggers
- Timer rules
- Wave progression formula

### 7. Acceptance Criteria
For each system, write testable acceptance criteria that the QA Lead can turn into automated tests.

Write the spec NOW. Be precise with numbers — the implementation team needs exact values, not ranges.
</teammate-message>

## Assistant (04:02 UTC)

Let me start by reading the team agreements and the frogger design skill document.

- **Used Read** on `.team/agreements.md`

- **Used Read** on `.claude/skills/frogger-design/SKILL.md`

- **Used Glob** for `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/.claude/agent-memory/game-designer/**/*` in `.`

- **Used Glob** for `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/**/*` in `.`

- **Used Read** on `.claude/agent-memory/game-designer/MEMORY.md`

## Assistant (04:03 UTC)

Good. I have all the context I need. Let me create the Docs/Design directory and write the Sprint 1 gameplay specification.

- **Used Bash**: List project root directory

- **Used Bash**: Create Docs/Design directory

## Assistant (04:03 UTC)

Now let me write the comprehensive design specification.

- **Used Write** on `Docs/Design/sprint1-gameplay-spec.md`

## Assistant (04:05 UTC)

Now let me update my agent memory with what I have learned in this session.

- **Used Write** on `.claude/agent-memory/game-designer/MEMORY.md`

## Assistant (04:05 UTC)

Now let me mark my task as complete and notify the team lead.

- **Used TaskUpdate** on task 10

- **Used SendMessage**

## Assistant (04:05 UTC)

The Sprint 1 Game Design Specification is complete. Here is a summary of what was delivered:

**File**: `/Users/randroid/Documents/Dev/Unreal/UnrealFrog/Docs/Design/sprint1-gameplay-spec.md`

**7 major sections**:

1. **Grid System** -- 13x15 grid, 100 UU cells, with exact coordinate mapping formulas and a C++ reference implementation for `GridToWorld` / `WorldToGrid`.

2. **Zone Layout** -- All 15 rows defined with zone types and hazard directions. Home slots at columns 1, 4, 6, 8, 11 on row 14.

3. **Movement Parameters** -- HopDuration (0.15s), HopArcHeight (30 UU), InputBufferWindow (0.1s), with the parabolic arc formula and 10 movement rules.

4. **Hazard Parameters** -- Complete Wave 1 lane configurations for all 5 road lanes and 6 river lanes, with exact speeds, widths, gaps, spawn counts, and collision bound dimensions. Road hitboxes are intentionally undersized (deaths feel fair). River platforms are intentionally oversized (landings feel generous).

5. **Scoring Rules** -- 10 points per forward hop with a 1x-5x consecutive multiplier. 200 per home, 1000 round bonus, time bonus formula. Extra life at every 10,000 points.

6. **Game State Transitions** -- 6 states with a complete transition table and ASCII state diagram. Timer = 30s, resets only on round completion.

7. **Acceptance Criteria** -- 42 testable criteria across 8 categories (GRID, MOVE, ROAD, RIVER, GOAL, SCORE, STATE, WAVE), written so the QA Lead can translate them directly into automated tests.

**3 appendices**: Data-driven FLaneConfig struct + enums, Sprint 1 scope boundary, and feel notes for implementers.
