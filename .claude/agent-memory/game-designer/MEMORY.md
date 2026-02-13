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
