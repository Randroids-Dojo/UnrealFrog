"""PlayUnreal visual verification — Minimal script to verify the game is visually working.

Connects to the running game, resets it, hops the frog a few times, checks
that game state changes accordingly, and takes screenshots at each step.

This is the "minimum viable play-test" that should run before any commit
that touches visual systems (VFX, HUD, materials, camera, lighting).

Usage:
    ./Tools/PlayUnreal/run-playunreal.sh verify_visuals.py

Or with the game already running:
    python3 Tools/PlayUnreal/verify_visuals.py

Exit codes:
    0 = all checks passed
    1 = one or more checks failed
    2 = cannot connect to Remote Control API
"""

import json
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from client import PlayUnreal, PlayUnrealError, RCConnectionError

SCREENSHOT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Saved", "Screenshots", "verify_visuals"
)

FAILURES = 0


def log(msg):
    print(f"[VerifyVisuals] {msg}")


def burst_screenshots(pu, prefix, count=3, interval=0.15):
    """Take a rapid burst of screenshots."""
    for i in range(count):
        path = os.path.join(SCREENSHOT_DIR, f"{prefix}_{i+1}.png")
        pu.screenshot(path)
        if i < count - 1:
            time.sleep(interval)


def check(desc, condition, detail=""):
    global FAILURES
    status = "PASS" if condition else "FAIL"
    print(f"  [{status}] {desc}")
    if detail:
        print(f"         {detail}")
    if not condition:
        FAILURES += 1
    return condition


def main():
    global FAILURES

    log("=== PlayUnreal Visual Verification ===")
    log(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    log("")

    pu = PlayUnreal()

    # -- Step 0: Connection check --------------------------------------------
    log("--- Step 0: Connection ---")
    if not pu.is_alive():
        log("FATAL: Remote Control API is not responding on localhost:30010")
        log("       Launch with: ./Tools/PlayUnreal/run-playunreal.sh verify_visuals.py")
        return 2

    check("RC API connection", True)

    # Verify we have live instances (not just CDO)
    gm_path = pu._get_gm_path()
    frog_path = pu._get_frog_path()
    check("GameMode is live instance", "Default__" not in gm_path, gm_path)
    check("FrogCharacter is live instance", "Default__" not in frog_path, frog_path)

    if "Default__" in gm_path or "Default__" in frog_path:
        log("FATAL: Cannot verify visuals without live game instances.")
        log("       The game may not have loaded FroggerMain map.")
        log("       Run diagnose.py first for detailed debugging.")
        return 1

    os.makedirs(SCREENSHOT_DIR, exist_ok=True)

    # -- Step 1: Reset game and verify title state ---------------------------
    log("")
    log("--- Step 1: Reset game ---")
    try:
        pu._call_function(gm_path, "ReturnToTitle")
        time.sleep(0.5)
        state = pu.get_state()
        check("Game in Title state after reset",
              state.get("gameState") == "Title",
              f"gameState={state.get('gameState')}")
    except PlayUnrealError as e:
        check("Reset to title", False, str(e))

    burst_screenshots(pu, "01_title", count=1)

    # -- Step 2: Start game and verify transition to Playing -----------------
    log("")
    log("--- Step 2: Start game ---")
    try:
        pu._call_function(gm_path, "StartGame")
        # StartGame -> Spawning (1s) -> Playing
        time.sleep(2.0)
        state = pu.get_state()
        check("Game in Playing state after start",
              state.get("gameState") == "Playing",
              f"gameState={state.get('gameState')}")
        check("Score is 0 at game start",
              state.get("score", -1) == 0,
              f"score={state.get('score')}")
        check("Lives > 0 at game start",
              state.get("lives", 0) > 0,
              f"lives={state.get('lives')}")
        check("Frog at start position",
              state.get("frogPos") == [6, 0] or state.get("frogPos") == [6.0, 0.0],
              f"frogPos={state.get('frogPos')}")
    except PlayUnrealError as e:
        check("Start game", False, str(e))

    burst_screenshots(pu, "02_playing", count=1)

    # -- Step 3: Hop left/right on safe row 0 to verify movement -------------
    log("")
    log("--- Step 3: Hop left and right on safe row 0 ---")
    prev_pos = pu.get_state().get("frogPos", [6, 0])
    positions_changed = 0

    # Stay on row 0 (safe) — row 1+ has traffic that kills the frog
    safe_hops = ["right", "right", "left"]
    for i, direction in enumerate(safe_hops):
        try:
            pu.hop(direction)
            time.sleep(0.4)  # HopDuration=0.15 + margin
            state = pu.get_state()
            new_pos = state.get("frogPos", prev_pos)
            if new_pos != prev_pos:
                positions_changed += 1
            log(f"  Hop {i+1} ({direction}): frogPos={new_pos}, score={state.get('score')}, "
                f"gameState={state.get('gameState')}")
            prev_pos = new_pos
        except PlayUnrealError as e:
            log(f"  Hop {i+1} ({direction}): ERROR - {e}")

    check("Frog position changed after hops",
          positions_changed >= 2,
          f"{positions_changed}/3 hops resulted in position changes")

    state = pu.get_state()
    check("Game still in Playing state",
          state.get("gameState") == "Playing",
          f"gameState={state.get('gameState')}")

    burst_screenshots(pu, "03_after_hops", count=1)

    # -- Step 4: Verify timer is counting down --------------------------------
    log("")
    log("--- Step 4: Timer check ---")
    time1 = state.get("timeRemaining", 30.0)
    time.sleep(1.0)
    state2 = pu.get_state()
    time2 = state2.get("timeRemaining", 30.0)
    check("Timer is counting down",
          time2 < time1,
          f"time before={time1:.1f}, after={time2:.1f}")

    # -- Step 5: Hop forward into traffic and verify survival/death ----------
    log("")
    log("--- Step 5: Hop forward (into traffic) ---")
    try:
        # Start 3s video, then hop immediately — no delay.
        # Recording starts first so the full 3 seconds captures the death.
        video_path = os.path.join(SCREENSHOT_DIR, "04_death_video.mov")
        video_proc = pu.record_video(video_path)
        pu.hop("up")
        log("  Video started + hop sent (no delay)")

        # Wait for the 3s recording to finish
        video_proc.wait(timeout=8)
        log(f"  Video saved: {video_path}")

        # Take a still screenshot of current state too
        burst_screenshots(pu, "05_after_forward", count=1)

        state5 = pu.get_state()
        gs = state5.get("gameState", "")
        frog_pos = state5.get("frogPos", [0, 0])

        if gs == "Playing":
            # Frog either survived or already respawned
            lives_now = state5.get("lives", 3)
            if lives_now < 3:
                check("Frog died and respawned (fast respawn)", True,
                      f"frogPos={frog_pos}, lives={lives_now}")
            else:
                check("Frog survived hop into row 1", True,
                      f"frogPos={frog_pos}")
                check("Score increased from forward hop",
                      state5.get("score", 0) > 0,
                      f"score={state5.get('score')}")
        elif gs in ("Dying", "Spawning"):
            check("Frog died from traffic (expected behavior)", True,
                  f"gameState={gs}, frogPos={frog_pos}")
            log("  Waiting for respawn...")
            time.sleep(2.0)
            state5 = pu.get_state()
            check("Frog respawned after death",
                  state5.get("gameState") == "Playing",
                  f"gameState={state5.get('gameState')}, lives={state5.get('lives')}")
        elif gs == "GameOver":
            # Timer ran out — game is working correctly, just took too long
            check("Game reached GameOver (timer expired)", True,
                  f"gameState={gs}, lives={state5.get('lives')}, score={state5.get('score')}")
        else:
            check("Forward hop produced expected state", False,
                  f"gameState={gs}")
    except PlayUnrealError as e:
        check("Forward hop", False, str(e))

    # -- Step 6: Final state check -------------------------------------------
    log("")
    log("--- Step 6: Final state ---")
    final_state = pu.get_state()
    log(f"  Full state: {json.dumps(final_state, indent=2)}")

    check("Game still responsive", pu.is_alive())
    check("Game not in error state",
          final_state.get("gameState") in ["Playing", "Spawning", "Title", "Dying",
                                            "RoundComplete", "GameOver", "Paused"],
          f"gameState={final_state.get('gameState')}")

    burst_screenshots(pu, "06_final", count=1)

    # -- Summary -------------------------------------------------------------
    log("")
    log("=" * 50)

    if FAILURES == 0:
        log("RESULT: PASS (all checks passed)")
        log(f"Screenshots saved to: {SCREENSHOT_DIR}")
        log("")
        log("IMPORTANT: Automated checks verify state changes only.")
        log("Review screenshots manually to confirm:")
        log("  - Ground plane is visible")
        log("  - Frog actor is visible and colored (not gray)")
        log("  - Camera shows full play area")
        log("  - HUD text renders (score, lives, timer)")
        log("  - Lighting is present (not pitch black)")
        return 0
    else:
        log(f"RESULT: FAIL ({FAILURES} check(s) failed)")
        log("")
        log("Review output above for details. Common issues:")
        log("  - Live object paths not found: run diagnose.py first")
        log("  - Frog doesn't move: RequestHop parameter format may be wrong")
        log("  - Score doesn't change: delegate wiring may be broken")
        log("  - Wrong game state: StartGame/ReturnToTitle may not work via RC API")
        return 1


if __name__ == "__main__":
    try:
        code = main()
        sys.exit(code)
    except RCConnectionError as e:
        log(f"CONNECTION ERROR: {e}")
        sys.exit(2)
    except PlayUnrealError as e:
        log(f"ERROR: {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        log("Interrupted.")
        sys.exit(130)
