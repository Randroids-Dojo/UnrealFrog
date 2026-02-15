# Game Designer Memory

*Persistent knowledge accumulated across sessions. First 200 lines loaded automatically.*

## Sprint 1 Design Spec
- Written at `/Docs/Design/sprint1-gameplay-spec.md`
- Grid: 13 columns x 15 rows, 100 UU cell size, origin at (Col-6)*100, Row*100, 0
- Frog spawns at grid (6, 0) = world (0, 0, 0)
- Zones bottom-to-top: Start(0), Road(1-5), Median(6), River(7-12), Goal(13-14)
- Home slots at columns 1, 4, 6, 8, 11 on row 14
- HopDuration 0.15s, HopArcHeight 30 UU, InputBuffer 0.1s
- Forward hop multiplier: 1x-5x, resets on backward hop or death
- Timer: 30s per round, resets only on round completion
- Wave progression: speed *= 1.0 + (wave-1)*0.1, gap shrinks every 2 waves, min gap 1

## Key Design Decisions
- Road hitboxes smaller than visual (forgiving near-misses, deaths feel fair)
- River platform bounds larger than visual (generous landing)
- Turtle submerge is OUT of scope for Sprint 1
- No lily pads, no sound, no HUD beyond debug text in Sprint 1
- All tunable values must be UPROPERTY(EditAnywhere) and data-driven via FLaneConfig

## Sprint 1 -> Sprint 2 Spec Divergences Found
- GameMode has 4 states (Menu, Playing, Paused, GameOver) but spec has 6 (Title, Spawning, Playing, Dying, RoundComplete, GameOver). Need Spawning+Dying for Sprint 2.
- BackwardHopMultiplier = 1.5f in code but spec says all hops equal duration. Should be 1.0.
- MultiplierIncrement = 0.5f in code but spec says integer +1 per hop. Should be 1.0.
- MaxLives = 5 in code but spec says 9.
- GridToWorld has no column-6 offset (grid starts at X=0, not centered). Keep as-is, update spec.
- TryFillHomeSlot does not award 200 points. Need AddBonusPoints method on ScoreSubsystem.
- Input buffering lacks timing checks (MOVE-07, MOVE-09) — buffers at any point during hop.

## Sprint 2 Camera Design
- Fixed isometric top-down, pitch -72 degrees, FOV 60
- Shows full 13x15 grid, no follow, no movement
- Camera Distance ~1800 UU above grid center
- NOT a SpringArmCamera — use ACameraActor or APlayerCameraManager override

## Sprint 2 Input Design
- Enhanced Input: IMC_Frog with IA_HopForward/Back/Left/Right + IA_Start
- Down trigger only (no repeat, no hold)
- Arrow keys + WASD for hops, Enter/P for start/pause

## Sprint 2 HUD Design
- Top bar: Score (left), HiScore (center-left), Timer bar (right, green->yellow->red)
- Bottom bar: Lives as frog icons (left), Wave number (right)
- Center overlay: "GAME OVER" during GameOver state
- Home slots are world-space (on the grid), not HUD elements
- UUserWidget bound to existing delegates

## Sprint 2 Minimum Playable Checklist
1. Map with visible grid (colored zones)
2. Fixed camera showing full grid
3. Input bindings (arrow+WASD)
4. Frog visible and hopping
5. Hazards visible and moving
6. Collision causing death + respawn
7. River riding working
8. Score/Lives/Timer visible
9. Game Over on 0 lives
10. Home slots fillable + round completion

## Sprint 5 Game Feel Assessment
### What was designed this sprint
- Title screen: animated "UNREAL FROG" (green<->yellow pulse 1.5Hz), blinking "PRESS START" (2Hz), high score display (conditional on >0), credits line
- Score pop animation: +N text, rises 60px over 1.5s with alpha fade, yellow for <=100pts, white for >100pts
- Wave announcement timing: 0.3s fade in, 1.0s hold, 0.7s fade out = 2.0s total
- Timer pulse: activates at <16.7% remaining, bar oscillates 8-14px height at ~1Hz, red flash
- VFX types: hop dust (3 brown cubes, 0.2s), death puff (expanding sphere, 0.5s, color-coded), home sparkle (4 gold spheres rising, 1.0s), round celebration (sparkle at all 5 slots, 2.0s)

### Music integration
- Two tracks: Music_Title and Music_Gameplay
- HandleGameStateChanged switches automatically: Title->title music, Playing->gameplay music, GameOver->stop
- Music adds significant atmosphere and pacing

### Feel gaps identified for Sprint 6
1. No transition animations between states (hard cuts Title->Spawning->Playing)
2. Death puff needs supplemental juice: screen shake (0.2s) + freeze frame (0.1s pause)
3. Round complete celebration at top of screen may be missed — needs center-screen acknowledgment
4. No high score persistence (resets every session)
5. No difficulty communication to player ("SPEED UP!" text on wave increase)
6. Timer warning is visual-only — SFX_TimerWarning exists but is not wired to timer pulse
