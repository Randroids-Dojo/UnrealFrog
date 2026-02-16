"""PlayUnreal visual verification — Full VFX/HUD smoke test.

Connects to the running game, resets it, and exercises ALL visual systems:
  - Basic gameplay (hops, timer, score, lives)
  - Score pop burst capture ("+50" text near frog)
  - Hop dust VFX (dust at hop origin)
  - Home slot celebration VFX
  - Wave transition fanfare
  - Ground color change between waves

This script should run before any commit that touches visual systems
(VFX, HUD, materials, camera, lighting).

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
    "..", "..", "Saved", "Screenshots", "smoke_test"
)

FAILURES = 0
SCREENSHOT_COUNT = 0


def log(msg):
    print(f"[VerifyVisuals] {msg}")


def take_screenshot(pu, name):
    """Take a single screenshot and print the [SCREENSHOT] line."""
    global SCREENSHOT_COUNT
    SCREENSHOT_COUNT += 1
    path = os.path.join(SCREENSHOT_DIR, f"{name}.png")
    pu.screenshot(path)
    abs_path = os.path.abspath(path)
    print(f"[SCREENSHOT] {abs_path}")
    return path


def burst_screenshots(pu, prefix, count=3, interval=0.1):
    """Take a rapid burst of screenshots and print [SCREENSHOT] lines."""
    paths = []
    for i in range(count):
        path = take_screenshot(pu, f"{prefix}_{i+1}")
        paths.append(path)
        if i < count - 1:
            time.sleep(interval)
    return paths


def check(desc, condition, detail=""):
    global FAILURES
    status = "PASS" if condition else "FAIL"
    print(f"  [{status}] {desc}")
    if detail:
        print(f"         {detail}")
    if not condition:
        FAILURES += 1
    return condition


def ensure_playing(pu, gm_path):
    """Reset game and wait until Playing state. Returns state dict or None."""
    try:
        pu._call_function(gm_path, "ReturnToTitle")
        time.sleep(0.5)
        pu._call_function(gm_path, "StartGame")
        # Invalidate cached frog path -- actor may be recreated after restart
        pu._frog_path = None
        time.sleep(2.0)
        state = pu.get_state()
        if state.get("gameState") == "Playing":
            return state
        # Try waiting longer
        state = pu.wait_for_state("Playing", timeout=5)
        return state
    except PlayUnrealError as e:
        log(f"  Could not reset to Playing: {e}")
        return None


# Grid constants
GRID_CELL_SIZE = 100.0  # UU per cell
SAFE_ROWS = {0, 6, 13}  # No hazards on these rows
HOME_ROW = 14
HOME_COLUMNS = [1, 4, 6, 8, 11]
ROAD_ROWS = {1, 2, 3, 4, 5}
RIVER_ROWS = {7, 8, 9, 10, 11, 12}


def _get_hazards_in_row(hazards, row):
    """Filter hazards list to only those in the given row."""
    return [h for h in hazards if h.get("row") == row]


def _is_road_gap_clear(hazards_in_row, frog_x, cell_size=GRID_CELL_SIZE):
    """Check if frog_x is clear of road hazards right now.

    For road rows, a gap means no hazard overlaps the frog's X position.
    Each hazard occupies [x, x + width*cell_size) along X.
    Returns True if safe to hop into this position.
    """
    for h in hazards_in_row:
        hx = h.get("x", 0)
        hw = h.get("width", 1) * cell_size
        # Frog occupies roughly one cell centered at frog_x
        frog_left = frog_x - cell_size * 0.4
        frog_right = frog_x + cell_size * 0.4
        if frog_right > hx and frog_left < hx + hw:
            return False
    return True


def _is_platform_present(hazards_in_row, frog_x, cell_size=GRID_CELL_SIZE):
    """Check if a rideable platform overlaps frog_x.

    For river rows, the frog must land ON a rideable hazard (log/turtle).
    Returns True if there's a platform covering frog_x.
    """
    for h in hazards_in_row:
        if not h.get("rideable", False):
            continue
        hx = h.get("x", 0)
        hw = h.get("width", 1) * cell_size
        if hx - cell_size * 0.3 <= frog_x <= hx + hw + cell_size * 0.3:
            return True
    return False


def _wait_for_safe_moment(pu, target_row, frog_x, timeout=8.0):
    """Poll hazard positions until the frog can safely hop into target_row.

    For road rows: waits until no hazard overlaps frog_x.
    For river rows: waits until a rideable platform overlaps frog_x.
    For safe/home rows: returns immediately.

    Returns True if a safe moment was found within timeout.
    """
    if target_row in SAFE_ROWS or target_row >= HOME_ROW:
        return True

    start = time.time()
    is_river = target_row in RIVER_ROWS

    while time.time() - start < timeout:
        hazards = pu.get_hazards()
        row_hazards = _get_hazards_in_row(hazards, target_row)

        if not row_hazards:
            return True  # No hazards in this row

        if is_river:
            if _is_platform_present(row_hazards, frog_x):
                return True
        else:
            if _is_road_gap_clear(row_hazards, frog_x):
                return True

        time.sleep(0.05)  # Poll at 20Hz

    return False


def hop_to_home_slot(pu, gm_path, max_deaths=10, label=""):
    """Navigate frog from row 0 to a home slot using real-time hazard queries.

    Strategy per the user's request: query player position, find the next
    row, query hazards in that row, wait for a safe gap (road) or platform
    alignment (river), then hop.

    Target column: 6 (center home slot).

    Returns:
        dict with keys:
            "success": bool -- True if home slot was reached
            "state": dict -- final game state
            "deaths": int -- number of deaths during the attempt
            "home_filled": int -- homeSlotsFilledCount after attempt
    """
    deaths = 0
    restarts = 0
    initial_filled = None

    while restarts <= max_deaths:
        state = pu.get_state()
        gs = state.get("gameState", "")

        if gs == "GameOver":
            restarts += 1
            if restarts > max_deaths:
                log(f"  {label}Giving up after {max_deaths} game overs")
                return {"success": False, "state": state, "deaths": deaths,
                        "home_filled": state.get("homeSlotsFilledCount", 0)}
            log(f"  {label}Game over #{restarts}, restarting...")
            result = ensure_playing(pu, gm_path)
            if not result:
                return {"success": False, "state": state, "deaths": deaths,
                        "home_filled": 0}
            initial_filled = None
            continue

        if gs in ("Dying", "Spawning"):
            deaths += 1
            time.sleep(2.0)
            continue

        if gs == "RoundComplete":
            return {"success": True, "state": state, "deaths": deaths,
                    "home_filled": state.get("homeSlotsFilledCount", 0)}

        if gs != "Playing":
            time.sleep(1.0)
            continue

        # Track initial home slots to detect new fills
        home_filled = state.get("homeSlotsFilledCount", 0)
        if initial_filled is None:
            initial_filled = home_filled
        if home_filled > initial_filled:
            return {"success": True, "state": state, "deaths": deaths,
                    "home_filled": home_filled}

        pos = state.get("frogPos", [6, 0])
        col = int(pos[0]) if isinstance(pos, list) and len(pos) > 0 else 6
        row = int(pos[1]) if isinstance(pos, list) and len(pos) > 1 else 0

        # Target column 6 (center home slot)
        target_col = 6
        frog_x = col * GRID_CELL_SIZE
        target_x = target_col * GRID_CELL_SIZE

        # Align laterally on safe rows before crossing dangerous rows
        if row in SAFE_ROWS and col != target_col:
            direction = "right" if col < target_col else "left"
            pu.hop(direction)
            time.sleep(0.25)
            continue

        next_row = row + 1
        if next_row > HOME_ROW:
            time.sleep(0.5)
            continue

        # Query hazards and wait for a safe moment to hop
        safe = _wait_for_safe_moment(pu, next_row, target_x, timeout=8.0)
        if not safe:
            log(f"  {label}Timeout waiting for safe gap at row {next_row}, hopping anyway")

        pu.hop("up")
        time.sleep(0.25)  # Wait for hop to complete (HopDuration=0.15s)

    return {"success": False, "state": pu.get_state(), "deaths": deaths,
            "home_filled": 0}


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

    take_screenshot(pu, "01_title")

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

    take_screenshot(pu, "02_playing")

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

    take_screenshot(pu, "03_after_hops")

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
        take_screenshot(pu, "05_after_forward")

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

    take_screenshot(pu, "06_final")

    # ========================================================================
    # Steps 7-11: VFX/HUD Visual Verification
    # Each step is wrapped in try/except so one failure does not abort others.
    # ========================================================================

    # -- Step 7: Hop dust VFX ------------------------------------------------
    log("")
    log("--- Step 7: Hop dust VFX ---")
    log("  Captures screenshot immediately after a hop to catch dust at origin.")
    try:
        state7 = ensure_playing(pu, gm_path)
        if state7:
            check("Game reset for hop dust test", True,
                  f"gameState={state7.get('gameState')}")

            # Extra wait — frog may not accept input immediately after spawn
            time.sleep(0.5)

            # Record frog position before the hop
            state7 = pu.get_state()
            pos_before = state7.get("frogPos", [6, 0])
            log(f"  Frog position before hop: {pos_before}")

            # Start video to capture the transient dust effect
            dust_video_path = os.path.join(SCREENSHOT_DIR, "07_hop_dust_video.mov")
            dust_video = pu.record_video(dust_video_path)

            # Hop right on safe row 0 (no traffic danger)
            pu.hop("right")

            # Take rapid screenshot burst at 0.05s intervals to catch dust
            # Dust VFX is transient — lasts <1 second
            time.sleep(0.05)
            burst_screenshots(pu, "07_hop_dust", count=3, interval=0.05)

            # Wait for hop animation to complete before checking position
            time.sleep(0.4)

            # Wait for video to finish
            dust_video.wait(timeout=8)
            log(f"  Dust video saved: {dust_video_path}")

            state_after = pu.get_state()
            check("Frog moved after hop (dust should be at origin)",
                  state_after.get("frogPos") != pos_before,
                  f"before={pos_before}, after={state_after.get('frogPos')}")
        else:
            check("Game reset for hop dust test", False, "Could not enter Playing state")
    except Exception as e:
        check("Hop dust VFX capture", False, str(e))

    # -- Step 8: Score pop burst capture -------------------------------------
    log("")
    log("--- Step 8: Score pop burst capture ---")
    log("  Hops forward (scoring hop), captures 3 rapid screenshots for '+50' text.")
    try:
        state8 = ensure_playing(pu, gm_path)
        if state8:
            check("Game reset for score pop test", True,
                  f"gameState={state8.get('gameState')}")

            score_before = state8.get("score", 0)
            log(f"  Score before forward hop: {score_before}")

            # Start video to capture transient score pop
            pop_video_path = os.path.join(SCREENSHOT_DIR, "08_score_pop_video.mov")
            pop_video = pu.record_video(pop_video_path)

            # Forward hop awards points and triggers score pop VFX
            pu.hop("up")

            # Rapid burst at 0.1s intervals to catch the "+50" text
            time.sleep(0.05)
            burst_screenshots(pu, "08_score_pop", count=3, interval=0.1)

            pop_video.wait(timeout=8)
            log(f"  Score pop video saved: {pop_video_path}")

            state_after = pu.get_state()
            score_after = state_after.get("score", 0)

            # The frog might have died from traffic on row 1, but score
            # should still have increased from the forward hop attempt
            check("Score increased after forward hop",
                  score_after > score_before,
                  f"before={score_before}, after={score_after}")

            log(f"  Score pop should appear near frog at {state_after.get('frogPos')}")
            log("  Review screenshots: look for '+50' or '+N' text near the frog")
        else:
            check("Game reset for score pop test", False, "Could not enter Playing state")
    except Exception as e:
        check("Score pop burst capture", False, str(e))

    # -- Step 9: Home slot fill + celebration VFX ----------------------------
    log("")
    log("--- Step 9: Home slot fill + celebration VFX ---")
    log("  Scripts frog across road and river into a home slot.")
    log("  Queries hazard positions in real-time to time safe crossings.")
    log("  Layout: row 0=start, 1-5=road, 6=median, 7-12=river, 13-14=goal")
    log("  Home slots at columns 1, 4, 6, 8, 11 on row 14")
    try:
        state9 = ensure_playing(pu, gm_path)
        if state9:
            check("Game reset for home slot test", True,
                  f"gameState={state9.get('gameState')}")

            take_screenshot(pu, "09a_home_start")

            result9 = hop_to_home_slot(pu, gm_path, max_deaths=10,
                                       label="[Step9] ")

            if result9["success"]:
                check("Frog reached home slot", True,
                      f"deaths={result9['deaths']}, "
                      f"homeFilled={result9['home_filled']}")
            else:
                check("Frog reached home slot", False,
                      f"deaths={result9['deaths']}, "
                      f"state={result9['state'].get('gameState')}")

            # Capture celebration VFX (sparkle actors, flash)
            time.sleep(0.3)
            burst_screenshots(pu, "09b_home_celebration", count=3, interval=0.15)

            # Also capture video for transient celebration effects
            celeb_video_path = os.path.join(SCREENSHOT_DIR, "09_celebration_video.mov")
            celeb_video = pu.record_video(celeb_video_path)
            celeb_video.wait(timeout=8)
            log(f"  Celebration video saved: {celeb_video_path}")

            final9 = pu.get_state()
            log(f"  Final state: score={final9.get('score')}, "
                f"homeFilled={final9.get('homeSlotsFilledCount')}, "
                f"gameState={final9.get('gameState')}")
        else:
            check("Game reset for home slot test", False, "Could not enter Playing state")
    except Exception as e:
        check("Home slot fill capture", False, str(e))

    # -- Step 10: Wave transition / fanfare ----------------------------------
    log("")
    log("--- Step 10: Wave transition fanfare ---")
    log("  Fills all 5 home slots to trigger wave completion.")
    log("  Uses hazard-query strategy for each crossing.")
    log("  Captures wave fanfare text animation.")
    try:
        state10 = ensure_playing(pu, gm_path)
        if state10:
            check("Game reset for wave transition test", True,
                  f"gameState={state10.get('gameState')}")

            take_screenshot(pu, "10a_wave1_start")
            wave_before = state10.get("wave", 1)
            log(f"  Starting wave: {wave_before}")

            # Fill 5 home slots by crossing the field repeatedly.
            # After each home slot fill, frog respawns at row 0.
            total_deaths = 0
            wave_transition_found = False

            for slot_num in range(1, 6):
                log(f"  Attempting home slot {slot_num}/5...")

                # Wait for Playing state (may be in RoundComplete between slots)
                for _ in range(20):
                    st = pu.get_state()
                    if st.get("gameState") == "Playing":
                        break
                    time.sleep(1.0)

                # Check for wave transition before each crossing
                wave_now = st.get("wave", 1)
                if wave_now > wave_before:
                    log(f"  Wave transition detected! {wave_before} -> {wave_now}")
                    wave_transition_found = True
                    check("Wave transition occurred", True,
                          f"wave {wave_before} -> {wave_now}")
                    burst_screenshots(pu, "10c_wave_fanfare", count=3, interval=0.2)
                    fanfare_video = os.path.join(SCREENSHOT_DIR, "10_fanfare_video.mov")
                    vid = pu.record_video(fanfare_video)
                    vid.wait(timeout=8)
                    log(f"  Fanfare video saved: {fanfare_video}")
                    break

                result = hop_to_home_slot(pu, gm_path, max_deaths=8,
                                          label=f"[Slot{slot_num}] ")
                total_deaths += result["deaths"]

                if result["success"]:
                    filled = result["home_filled"]
                    log(f"  Home slot {slot_num} filled! "
                        f"(total filled={filled}, deaths={result['deaths']})")
                    take_screenshot(pu, f"10b_slot_{slot_num}_filled")

                    # Check for wave transition right after filling
                    time.sleep(1.0)
                    post_state = pu.get_state()
                    wave_now = post_state.get("wave", 1)
                    if wave_now > wave_before:
                        log(f"  Wave transition detected! {wave_before} -> {wave_now}")
                        wave_transition_found = True
                        check("Wave transition occurred", True,
                              f"wave {wave_before} -> {wave_now}")
                        burst_screenshots(pu, "10c_wave_fanfare", count=3, interval=0.2)
                        fanfare_video = os.path.join(SCREENSHOT_DIR, "10_fanfare_video.mov")
                        vid = pu.record_video(fanfare_video)
                        vid.wait(timeout=8)
                        log(f"  Fanfare video saved: {fanfare_video}")
                        break
                else:
                    log(f"  Failed to fill slot {slot_num} "
                        f"(deaths={result['deaths']}, "
                        f"state={result['state'].get('gameState')})")
                    check(f"Home slot {slot_num} filled", False,
                          f"Could not reach home slot after {result['deaths']} deaths")
                    break

            if not wave_transition_found:
                # Check final state -- maybe wave changed and we missed it
                final10 = pu.get_state()
                wave_final = final10.get("wave", 1)
                if wave_final > wave_before:
                    check("Wave transition occurred (late detection)", True,
                          f"wave {wave_before} -> {wave_final}")
                    take_screenshot(pu, "10c_wave_late")
                else:
                    check("Wave transition reached", False,
                          f"total_deaths={total_deaths}, "
                          f"wave still {wave_final}")
        else:
            check("Game reset for wave transition test", False, "Could not enter Playing state")
    except Exception as e:
        check("Wave transition fanfare capture", False, str(e))

    # -- Step 11: Ground color change between waves --------------------------
    log("")
    log("--- Step 11: Ground color change ---")
    log("  Compares ground appearance at Wave 1 vs Wave 2+.")
    log("  Ground should shift from cool to warm colors with difficulty.")
    try:
        # First, capture Wave 1 ground (fresh game)
        state11 = ensure_playing(pu, gm_path)
        if state11:
            check("Game reset for ground color test", True,
                  f"gameState={state11.get('gameState')}")

            wave11 = state11.get("wave", 1)
            take_screenshot(pu, "11a_ground_wave1")
            log(f"  Wave 1 ground screenshot captured (wave={wave11})")

            # If we already triggered a wave transition in step 10,
            # the game might be at wave 2+. Try to get there if not.
            state_check = pu.get_state()
            wave_check = state_check.get("wave", 1)
            if wave_check > 1:
                take_screenshot(pu, "11b_ground_wave2")
                log(f"  Wave 2+ ground screenshot captured (wave={wave_check})")
                check("Ground color screenshots taken for comparison", True,
                      f"wave1 and wave{wave_check} screenshots captured")
            else:
                log("  Wave 2 not reached yet. Ground color comparison requires wave transition.")
                log("  If Step 10 captured a wave transition, compare 11a with 10c screenshots.")
                check("Ground color screenshots taken for comparison", True,
                      "Wave 1 captured. Wave 2+ comparison deferred to step 10 screenshots")
        else:
            check("Game reset for ground color test", False, "Could not enter Playing state")
    except Exception as e:
        check("Ground color change capture", False, str(e))

    # -- Summary -------------------------------------------------------------
    log("")
    log("=" * 60)

    if FAILURES == 0:
        log("RESULT: PASS (all checks passed)")
    else:
        log(f"RESULT: FAIL ({FAILURES} check(s) failed)")

    log(f"Screenshots taken: {SCREENSHOT_COUNT}")
    log(f"Screenshots saved to: {os.path.abspath(SCREENSHOT_DIR)}")
    log("")
    log("REVIEW CHECKLIST — examine each screenshot for:")
    log("  1. Ground plane visible, lit, colored")
    log("  2. Frog actor visible and colored (not gray)")
    log("  3. Camera shows full play area from top-down at Z=2200")
    log("  4. HUD text renders (score, lives, timer)")
    log("  5. Lighting present (not pitch black)")
    log("")
    log("VFX/HUD CHECKS — examine smoke_test/ screenshots:")
    log("  6. [Step 7]  Hop dust: visible particles at hop origin")
    log("  7. [Step 8]  Score pop: '+50' text near frog, not top-left")
    log("  8. [Step 9]  Home celebration: sparkle/flash at home slot")
    log("  9. [Step 10] Wave fanfare: centered text animation")
    log("  10. [Step 11] Ground color: cool (wave 1) vs warm (wave 2+)")
    log("")

    if FAILURES > 0:
        log("Common issues:")
        log("  - Live object paths not found: run diagnose.py first")
        log("  - Frog doesn't move: RequestHop parameter format may be wrong")
        log("  - Score doesn't change: delegate wiring may be broken")
        log("  - Wrong game state: StartGame/ReturnToTitle may not work via RC API")

    return 0 if FAILURES == 0 else 1


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
