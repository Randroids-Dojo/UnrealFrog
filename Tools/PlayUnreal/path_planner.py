"""Predictive safe-path planner for Frogger-style games.

Incremental planning: queries hazards fresh before each hop, so predictions
are always based on current positions. Handles platform drift on river.

Road rows: swept-collision check ensures no hazard passes through the frog
during the entire 0.15s hop animation.

River rows: waits for a rideable platform to be at the frog's position,
then hops onto it. Re-queries after landing to track drift.

Usage:
    from path_planner import navigate_to_home_slot
    result = navigate_to_home_slot(pu, target_col=6)
"""

import time

# Game constants (defaults match UnrealFrog)
CELL_SIZE = 100.0
GRID_COLS = 13
HOP_DURATION = 0.15  # seconds
SAFE_ROWS = {0, 6, 13}
ROAD_ROWS = {1, 2, 3, 4, 5}
RIVER_ROWS = {7, 8, 9, 10, 11, 12}
HOME_ROW = 14

# Collision geometry (from FrogCharacter.cpp / HazardBase.cpp)
# Capsule overlap with box: |frog_x - hazard_x| < hazard_half_w + capsule_radius
FROG_CAPSULE_RADIUS = 34.0
# Extra buffer for API latency timing error (~80ms * ~200 units/s = 16 units)
TIMING_BUFFER = 30.0

# Execution timing
API_LATENCY = 0.10  # conservative RC API round-trip estimate
POST_HOP_WAIT = HOP_DURATION + 0.04  # wait after sending hop command


def predict_hazard_x(hazard, dt):
    """Predict a hazard's CENTER X position after dt seconds."""
    x0 = hazard["x"]
    speed = hazard["speed"]
    direction = 1.0 if hazard["movesRight"] else -1.0
    world_width = hazard["width"] * CELL_SIZE
    grid_world_width = GRID_COLS * CELL_SIZE

    wrap_min = -world_width
    wrap_max = grid_world_width + world_width
    wrap_range = wrap_max - wrap_min

    raw_x = x0 + speed * direction * dt
    return ((raw_x - wrap_min) % wrap_range) + wrap_min


def is_road_safe_swept(hazards_in_row, frog_x, hop_duration=HOP_DURATION):
    """Check if frog_x is clear of ALL road hazards for the full hop duration.

    Uses swept collision: checks if the hazard's extent passes through the
    frog's extent at ANY point during [0, hop_duration].

    Checks at 6 time samples across the hop to catch fast-moving hazards.
    """
    danger_radius = FROG_CAPSULE_RADIUS + TIMING_BUFFER  # 64 units each side

    # Sample at multiple times during the hop
    check_times = [i * hop_duration / 5 for i in range(6)]  # 0, 0.03, 0.06, ...0.15

    for h in hazards_in_row:
        if h.get("rideable", False):
            continue  # Skip river platforms that might be in road data
        half_w = h["width"] * CELL_SIZE * 0.5

        for t in check_times:
            hx = predict_hazard_x(h, t)
            # Overlap if frog extent intersects hazard extent
            if abs(frog_x - hx) < half_w + danger_radius:
                return False
    return True


def find_river_platform_wait(hazards_in_row, frog_x, max_wait=6.0):
    """Find when a rideable platform will be at frog_x.

    Uses the actual game collision check: |frog_x - platform_x| <= half_width.
    Scans forward in time to find the first moment a platform covers frog_x.

    Returns wait time in seconds, or None if no platform found.
    """
    rideables = [h for h in hazards_in_row if h.get("rideable", False)]
    if not rideables:
        return None

    # The frog needs to ARRIVE on the platform. So we check at (wait + HOP_DURATION).
    # Also check that the platform still covers frog_x for a small window after landing
    # (the frog spends ~0.05s settling before it starts riding).
    time_step = 0.02
    wait = 0.0

    while wait < max_wait:
        arrival_time = wait + HOP_DURATION
        for h in rideables:
            hx = predict_hazard_x(h, arrival_time)
            half_w = h["width"] * CELL_SIZE * 0.5
            # Match the game's FindPlatformAtCurrentPosition check:
            # |frog_x - hazard_x| <= half_width
            # Use a small inset margin for safety
            margin = 20.0  # land well inside the platform, not on the edge
            if abs(frog_x - hx) <= half_w - margin:
                return wait
        wait += time_step

    return None


def navigate_to_home_slot(pu, target_col=6, max_deaths=8):
    """Navigate frog from current position to a home slot.

    Incremental planning: queries hazards fresh before each dangerous hop.
    Re-queries state after each hop to track position (especially platform drift).

    Args:
        pu: PlayUnreal client instance
        target_col: home slot column to target (default 6 = center)
        max_deaths: give up after this many deaths

    Returns:
        dict with success, total_hops, deaths, elapsed, state
    """
    total_hops = 0
    deaths = 0
    initial_filled = None
    start = time.time()
    attempt = 0

    while attempt < max_deaths * 3:  # allow retries
        attempt += 1

        # Get current state
        state = pu.get_state()
        gs = state.get("gameState", "")

        if gs == "GameOver":
            return _result(False, total_hops, deaths, start, state)

        if gs in ("Dying", "Spawning"):
            deaths += 1
            time.sleep(1.5)
            continue

        if gs == "RoundComplete":
            return _result(True, total_hops, deaths, start, state)

        if gs != "Playing":
            time.sleep(0.5)
            continue

        # Track home slot fills
        filled = state.get("homeSlotsFilledCount", 0)
        if initial_filled is None:
            initial_filled = filled
        if filled > initial_filled:
            return _result(True, total_hops, deaths, start, state)

        pos = state.get("frogPos", [6, 0])
        frog_col = int(pos[0]) if isinstance(pos, list) and len(pos) > 0 else 6
        frog_row = int(pos[1]) if isinstance(pos, list) and len(pos) > 1 else 0

        if frog_row >= HOME_ROW:
            return _result(True, total_hops, deaths, start, state)

        # Decide next hop
        next_row = frog_row + 1

        # Lateral alignment — only on safe rows
        if frog_row in SAFE_ROWS and frog_col != target_col:
            direction = "right" if frog_col < target_col else "left"
            pu.hop(direction)
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # Safe rows and home row — hop immediately
        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # Dangerous row — query fresh hazards
        hazards = pu.get_hazards()
        row_hazards = [h for h in hazards if h.get("row") == next_row]

        frog_x = frog_col * CELL_SIZE

        if next_row in ROAD_ROWS:
            # Road: wait for a gap using swept-collision check
            waited = _wait_for_road_gap(row_hazards, frog_x, timeout=4.0)
            if waited < 0:
                # Timeout — hop anyway, accept risk
                pass
            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        if next_row in RIVER_ROWS:
            # River: wait for a platform to arrive at frog's X
            wait = find_river_platform_wait(row_hazards, frog_x, max_wait=6.0)
            if wait is not None and wait > 0.02:
                time.sleep(wait)
            elif wait is None:
                # No platform will pass — try waiting longer with real-time checks
                found = _wait_for_platform_realtime(pu, frog_x, next_row, timeout=8.0)
                if not found:
                    # Last resort: hop anyway (will die, retry loop handles it)
                    pass
            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # Unknown row type — hop
        pu.hop("up")
        total_hops += 1
        time.sleep(POST_HOP_WAIT)

    return _result(False, total_hops, deaths, start,
                   pu.get_state() if pu else {})


def _wait_for_road_gap(hazards_in_row, frog_x, timeout=4.0):
    """Spin-wait until the road is clear for a hop. Returns seconds waited."""
    start = time.time()
    while time.time() - start < timeout:
        if is_road_safe_swept(hazards_in_row, frog_x):
            return time.time() - start
        time.sleep(0.01)
    return -1  # timeout


def _wait_for_platform_realtime(pu, frog_x, target_row, timeout=8.0):
    """Re-query hazards in a loop until a platform is at frog_x.

    Used as fallback when single-snapshot prediction fails.
    """
    start = time.time()
    while time.time() - start < timeout:
        hazards = pu.get_hazards()
        row_hazards = [h for h in hazards if h.get("row") == target_row]
        rideables = [h for h in row_hazards if h.get("rideable", False)]

        for h in rideables:
            hx = h["x"]  # current position (just queried)
            half_w = h["width"] * CELL_SIZE * 0.5
            # Check if platform is approaching and will be at frog_x within HOP_DURATION
            wait = find_river_platform_wait([h], frog_x, max_wait=HOP_DURATION + 0.1)
            if wait is not None:
                if wait > 0.02:
                    time.sleep(wait)
                return True

        time.sleep(0.05)
    return False


def _result(success, total_hops, deaths, start, state):
    """Build a standard result dict."""
    return {
        "success": success,
        "total_hops": total_hops,
        "deaths": deaths,
        "elapsed": time.time() - start,
        "state": state,
    }


# Legacy API compatibility — plan_path and execute_path for verify_visuals.py
def plan_path(hazards, frog_col=6, frog_row=0, target_col=6,
              target_row=HOME_ROW):
    """Compute hop sequence (legacy API — used by verify_visuals.py).

    Returns list of {"wait": float, "direction": str} dicts.
    Note: This plans from a single snapshot. For better results,
    use navigate_to_home_slot() which re-queries per hop.
    """
    path = []
    current_col = frog_col
    current_row = frog_row
    elapsed = 0.0
    hop_exec_time = HOP_DURATION + API_LATENCY

    hazards_by_row = {}
    for h in hazards:
        r = h.get("row", -1)
        if r not in hazards_by_row:
            hazards_by_row[r] = []
        hazards_by_row[r].append(h)

    while current_row < target_row:
        if current_row in SAFE_ROWS and current_col != target_col:
            while current_col != target_col:
                d = "right" if current_col < target_col else "left"
                path.append({"wait": 0.0, "direction": d})
                current_col += (1 if d == "right" else -1)
                elapsed += hop_exec_time

        next_row = current_row + 1
        target_x = current_col * CELL_SIZE

        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += hop_exec_time
            continue

        row_hazards = hazards_by_row.get(next_row, [])
        if not row_hazards:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += hop_exec_time
            continue

        # Find safe wait
        wait = _find_safe_wait_snapshot(row_hazards, target_x, elapsed, next_row)
        path.append({"wait": max(wait, 0.0), "direction": "up"})
        current_row = next_row
        elapsed += max(wait, 0.0) + hop_exec_time

    return path


def _find_safe_wait_snapshot(hazards_in_row, target_x, base_time, row,
                              max_wait=6.0, time_step=0.02):
    """Find wait time from a snapshot (legacy). Returns float or 0.0."""
    is_river = row in RIVER_ROWS
    wait = 0.0
    while wait < max_wait:
        arrival = base_time + wait + HOP_DURATION
        if is_river:
            rideables = [h for h in hazards_in_row if h.get("rideable", False)]
            for h in rideables:
                hx = predict_hazard_x(h, arrival)
                half_w = h["width"] * CELL_SIZE * 0.5
                if abs(target_x - hx) <= half_w - 20.0:
                    return wait
        else:
            safe = True
            for h in hazards_in_row:
                if h.get("rideable", False):
                    continue
                for t_offset in [0, 0.05, 0.10, 0.15]:
                    hx = predict_hazard_x(h, base_time + wait + t_offset)
                    half_w = h["width"] * CELL_SIZE * 0.5
                    if abs(target_x - hx) < half_w + FROG_CAPSULE_RADIUS + TIMING_BUFFER:
                        safe = False
                        break
                if not safe:
                    break
            if safe:
                return wait
        wait += time_step
    return 0.0


def execute_path(pu, path):
    """Execute a pre-planned hop sequence (legacy API)."""
    total_hops = 0
    start_time = time.time()

    for step in path:
        if step["wait"] > 0.02:
            time.sleep(step["wait"])
        pu.hop(step["direction"])
        total_hops += 1
        time.sleep(POST_HOP_WAIT)

    return {
        "hops": total_hops,
        "elapsed": time.time() - start_time,
    }
