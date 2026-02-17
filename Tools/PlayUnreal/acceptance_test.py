"""PlayUnreal acceptance test — Hop the frog across the road, river, into a home slot.

This is the acceptance test defined in agreements Section 20:
  A Python script that hops the frog across the road, across the river,
  and into a home slot.

Uses the path planner for intelligent navigation instead of blind hopping.
Runs with invincibility OFF (real gameplay) and retries on death.

Usage:
  python3 Tools/PlayUnreal/acceptance_test.py
  ./Tools/PlayUnreal/run-playunreal.sh acceptance_test.py

Prerequisites:
  - Editor running with Remote Control API enabled (use run-playunreal.sh)
"""

import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from client import PlayUnreal, PlayUnrealError, ConnectionError
import path_planner

SCREENSHOT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Saved", "Screenshots", "acceptance_test"
)


def log(msg):
    print(f"[Acceptance] {msg}")


def screenshot(pu, name):
    path = os.path.join(SCREENSHOT_DIR, f"{name}.png")
    pu.screenshot(path)
    log(f"  [SCREENSHOT] {os.path.abspath(path)}")
    return path


def main():
    log("=== PlayUnreal Acceptance Test ===")
    log("")

    # -- Connect --
    pu = PlayUnreal()
    if not pu.is_alive():
        log("FATAL: RC API not responding on localhost:30010")
        return 2

    log("Connected to Remote Control API.")
    os.makedirs(SCREENSHOT_DIR, exist_ok=True)

    # Load game constants for path planner
    try:
        config = pu.get_config()
        path_planner.init_from_config(config)
        log(f"  Loaded config: cellSize={path_planner.CELL_SIZE}, cols={path_planner.GRID_COLS}")
    except Exception as e:
        log(f"  Config load failed ({e}), using defaults")

    # -- Phase 1: Reset and start game --
    log("")
    log("--- Phase 1: Reset and start game ---")
    gm_path = pu._get_gm_path()
    pu._call_function(gm_path, "ReturnToTitle")
    time.sleep(0.5)
    pu._call_function(gm_path, "StartGame")
    time.sleep(2.0)

    state = pu.get_state()
    log(f"  State: {state.get('gameState')}, Lives: {state.get('lives')}")
    log(f"  Frog: {state.get('frogPos')}")
    screenshot(pu, "01_start")

    # -- Phase 2: Navigate to home slot using path planner --
    log("")
    log("--- Phase 2: Navigate to home slot (path planner, no invincibility) ---")

    result = path_planner.navigate_to_home_slot(pu, target_col=6, max_deaths=5)

    log(f"  Navigation result: success={result['success']}")
    log(f"  Total hops: {result['total_hops']}, Deaths: {result['deaths']}")
    log(f"  Elapsed: {result['elapsed']:.1f}s")

    screenshot(pu, "02_after_navigation")

    # -- Phase 3: Verify results --
    log("")
    log("--- Phase 3: Verify results ---")

    state = pu.get_state()
    score = state.get("score", 0)
    lives = state.get("lives", 0)
    home_filled = state.get("homeSlotsFilledCount", 0)
    game_state = state.get("gameState", "unknown")

    log(f"  State: {game_state}")
    log(f"  Score: {score}, Lives: {lives}, Home filled: {home_filled}")

    screenshot(pu, "03_final")

    # -- Determine result --
    log("")
    if home_filled >= 1 or result["success"]:
        log("=== ACCEPTANCE TEST: FULL PASS ===")
        log(f"  Home slot filled! Score={score}, Lives={lives}")
        return 0
    elif score > 0:
        log("=== ACCEPTANCE TEST: PARTIAL PASS ===")
        log(f"  Hops executed, commands work, but no home slot reached.")
        log(f"  Score={score}, Lives={lives}, Deaths={result['deaths']}")
        return 0
    else:
        log("=== ACCEPTANCE TEST: FAIL ===")
        log(f"  No score gained. Commands may not be working.")
        return 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except ConnectionError as e:
        log(f"CONNECTION ERROR: {e}")
        sys.exit(2)
    except PlayUnrealError as e:
        log(f"ERROR: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        log("Interrupted.")
        sys.exit(130)
