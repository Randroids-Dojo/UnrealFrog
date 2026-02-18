# Visual Comparison Skill

Compare before/after screenshots to prove visual changes are visible at gameplay distance.

## When to Use

- **Before committing any visual change** (models, materials, VFX, HUD, camera, lighting)
- After adjusting model geometry, colors, or scale
- When someone claims a visual improvement — proof required

## When NOT to Use

- Logic-only changes (tests, scoring, state machines)
- Tooling changes (Python scripts, build scripts)
- Audio changes

## Quick Reference

```bash
# Compare current commit vs previous (most common)
python3 Tools/PlayUnreal/visual_compare.py

# Compare specific refs
python3 Tools/PlayUnreal/visual_compare.py --before HEAD~3
python3 Tools/PlayUnreal/visual_compare.py --before abc123 --after def456

# Allow dirty working tree (auto-stashes)
python3 Tools/PlayUnreal/visual_compare.py --stash
```

## What It Does

1. Identifies changed Source/ files between two git refs
2. Checks out the "before" files, builds Editor, launches game
3. Takes screenshots at **4 zoom levels**: full field, road close-up, river close-up, frog close-up
4. Restores the "after" files, rebuilds, relaunches, takes the same 4 zoom screenshots
5. Prints paired `[BEFORE]` and `[AFTER]` paths at each zoom level

Output: `Saved/Screenshots/compare/<timestamp>/` with 8 PNGs:
- `before_full.png` / `after_full.png` — full gameplay view (Z=2200)
- `before_road.png` / `after_road.png` — road section close-up (Z=800)
- `before_river.png` / `after_river.png` — river section close-up (Z=800)
- `before_frog.png` / `after_frog.png` — frog start position tight (Z=400)

## Anti-Hallucination Rules (CRITICAL)

**Never trust commit messages as evidence.** A commit that says "bus windows, log caps, smaller frog" might have produced changes invisible at gameplay distance. The screenshots are the ONLY evidence.

**For each claimed change, you MUST:**
1. Identify which zoom level would show it (e.g., "bus windows" → road close-up)
2. Read the specific before/after pair at that zoom level
3. Describe ONLY what you can actually see differs between the two images
4. If you cannot see the difference, say so explicitly: "I cannot see this change in the screenshots"

**Prohibited behaviors:**
- Describing changes you "know" were made from reading the code or commit message
- Claiming visual differences you cannot point to in a specific screenshot pair
- Using hedging language like "appears to" or "seems like" to cover uncertainty — if you're not sure, say "I cannot confirm this"

**The zoom levels exist because the full-field view (Z=2200) hides details.** If a change is only visible in the road close-up but invisible in the full view, report that honestly: "Visible at road zoom, invisible at gameplay distance."

## Reporting Results

**Always include screenshot file paths in your response** so the user can confirm the comparison themselves. Format each zoom level:

```
**Full field (gameplay view):**
- Before: `/path/to/before_full.png`
- After: `/path/to/after_full.png`
- Observed difference: [what you ACTUALLY see, or "no visible difference"]

**Road close-up:**
- Before: `/path/to/before_road.png`
- After: `/path/to/after_road.png`
- Observed difference: [what you ACTUALLY see, or "no visible difference"]
```

The user cannot see what the agent sees — they need the paths to open the screenshots independently. Never summarize visual differences without linking to the evidence.

## Safety

- Files are always restored to the "after" state, even on error (try/finally + atexit)
- Editor processes are always killed on exit
- Refuses to run with dirty working tree unless `--stash` is passed
- Validates git refs before starting any work
- Camera is reset to default position after zoom shots
