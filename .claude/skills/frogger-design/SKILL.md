---
name: frogger-design
description: Game design reference for 3D Frogger arcade gameplay, mechanics, and progression
context: fork
---

## Core Loop

**Cross lanes of traffic and rivers to reach home.**

One sentence. One mechanic (movement). One goal (reach the other side). Complexity comes from the environment, not the controls.

## Controls

- **Directional input**: Move in 4 directions (up, down, left, right)
- **Hop**: Grid-based movement with smooth visual interpolation
- **No attack, no items at launch** — pure movement mastery

## Game Structure

### Zones (Top to Bottom)
1. **Goal Zone** (top): 5 home slots the frog must fill
2. **River Zone**: Logs, turtles, lily pads moving laterally — frog must ride them
3. **Median**: Safe strip between zones — breathing room
4. **Road Zone**: Cars, trucks, buses moving laterally — frog must avoid them
5. **Start Zone** (bottom): Safe spawn area

### Lane System
- Each lane has ONE hazard type moving in ONE direction
- Adjacent lanes alternate direction (left, right, left, right)
- Speed varies per lane — outer lanes faster, inner lanes slower
- Gaps between hazards are ALWAYS passable (no impossible configurations)

## Mechanics

### Movement
- Grid-based: frog snaps to grid positions
- Visual interpolation: smooth hop animation between grid cells
- Hop arc: slight vertical arc for visual satisfaction
- Forward bias: moving forward is faster than retreating

### Scoring
- Points per forward hop (10 pts)
- Bonus for consecutive forward hops without retreating (multiplier)
- Time bonus for reaching home quickly
- Extra life at score thresholds (10,000 pts)
- Filling all 5 home slots completes the level

### Difficulty Progression
- **Wave 1-3**: Slow hazards, wide gaps, few lanes
- **Wave 4-6**: Medium speed, narrower gaps, more lane variety
- **Wave 7-9**: Fast hazards, tight gaps, all lane types active
- **Wave 10+**: Maximum speed, minimum gaps, hazard variety

### Difficulty Curve Pattern
```
Tension ████████░░ (high traffic density)
Release ██░░░░░░░░ (median safe zone)
Tension ██████████ (river with tricky timing)
Triumph █░░░░░░░░░ (reaching home)
```

## Enemy/Hazard Types

### Road Hazards
| Type | Speed | Width | Behavior |
|------|-------|-------|----------|
| Car | Medium | 1 lane | Steady speed, even spacing |
| Truck | Slow | 2 lanes | Slow but long, blocks vision |
| Bus | Fast | 2 lanes | Fast and long, scary |
| Motorcycle | Very Fast | 1 lane | Unpredictable spacing |

### River Objects
| Type | Speed | Width | Behavior |
|------|-------|-------|----------|
| Log (small) | Slow | 2 cells | Safe to ride |
| Log (large) | Medium | 4 cells | Safe to ride, more room |
| Turtle group | Slow | 3 cells | Submerges periodically! |
| Lily pad | Stationary | 1 cell | Fixed safe spot |

## Death Conditions
- Hit by any road vehicle → squish (visual + audio)
- Fall in water (miss all platforms) → splash (visual + audio)
- Carried off screen edge by river object → splash
- Time runs out → fade out

## Feel Parameters (Tunable via Blueprint)

```
HopDuration: 0.15s (time to complete one hop)
HopArcHeight: 30.0 units (vertical peak of hop arc)
InputBufferWindow: 0.1s (queue next input during hop)
InvincibilityFrames: 0.0s (none — arcade rules)
CameraFollowSpeed: 10.0 (smooth camera tracking)
RespawnDelay: 1.0s (pause after death before respawn)
TimePerLevel: 30.0s (countdown timer per attempt)
```
