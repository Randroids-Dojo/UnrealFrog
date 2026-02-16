"""Predictive safe-path planner for Frogger-style games.

Strategy: instead of waiting at one column for a gap to pass, FIND the
nearest column that's already safe and move there. Like a human player
who looks at the whole lane and picks the gap.

Incremental: queries fresh hazard data before each dangerous hop.
Measured API latency: ~9ms. Hop response: ~33ms.

Usage:
    from path_planner import navigate_to_home_slot
    result = navigate_to_home_slot(pu, target_col=6)
"""

import time

# Game constants
CELL_SIZE = 100.0
GRID_COLS = 13
HOP_DURATION = 0.15
SAFE_ROWS = {0, 6, 13}
ROAD_ROWS = {1, 2, 3, 4, 5}
RIVER_ROWS = {7, 8, 9, 10, 11, 12}
HOME_ROW = 14
HOME_COLS = {1, 4, 6, 8, 11}

# Collision geometry
FROG_CAPSULE_RADIUS = 34.0
# Safety margin: capsule (34) + generous buffer (80) = 114 total.
# At 250 u/s (fastest hazard), 114 units = 0.46s of margin.
SAFETY_MARGIN = 80.0

# Measured timing (from debug_navigation.py)
API_LATENCY = 0.01  # 9ms measured
HOP_RESPONSE = 0.035  # 33ms until position changes
POST_HOP_WAIT = HOP_DURATION + 0.04  # total wait after hop command


def predict_hazard_x(hazard, dt):
    """Predict hazard CENTER X after dt seconds."""
    x0 = hazard["x"]
    speed = hazard["speed"]
    direction = 1.0 if hazard["movesRight"] else -1.0
    world_width = hazard["width"] * CELL_SIZE
    wrap_min = -world_width
    wrap_max = GRID_COLS * CELL_SIZE + world_width
    wrap_range = wrap_max - wrap_min
    raw_x = x0 + speed * direction * dt
    return ((raw_x - wrap_min) % wrap_range) + wrap_min


def is_column_safe_for_hop(hazards_in_row, col, duration=HOP_DURATION):
    """Check if column `col` is safe for the frog for the full hop duration.

    Checks at 8 time samples across [0, duration] to catch fast movers.
    A column is safe if no hazard's extent + margin overlaps the frog.
    """
    frog_x = col * CELL_SIZE
    danger = FROG_CAPSULE_RADIUS + SAFETY_MARGIN  # 114 units

    check_times = [i * duration / 7 for i in range(8)]

    for h in hazards_in_row:
        if h.get("rideable", False):
            continue
        half_w = h["width"] * CELL_SIZE * 0.5
        for t in check_times:
            hx = predict_hazard_x(h, t)
            if abs(frog_x - hx) < half_w + danger:
                return False
    return True


def find_safe_road_column(hazards_in_row, current_col, preferred_col):
    """Find the nearest column that's safe to hop through RIGHT NOW.

    Searches outward from current_col, preferring the direction of preferred_col.
    Returns (safe_col, wait) where wait is always 0.0 (gap exists now).
    """
    # First check current column
    if is_column_safe_for_hop(hazards_in_row, current_col):
        return current_col, 0.0

    # Search outward, biased toward preferred direction
    for dist in range(1, GRID_COLS):
        # Try direction toward preferred first
        if preferred_col >= current_col:
            candidates = [current_col + dist, current_col - dist]
        else:
            candidates = [current_col - dist, current_col + dist]

        for col in candidates:
            if 0 <= col < GRID_COLS:
                if is_column_safe_for_hop(hazards_in_row, col):
                    return col, 0.0

    # No column is safe right now — wait briefly and return current
    # (the retry loop will re-query hazards next iteration)
    return current_col, 0.3


def find_platform_column(hazards_in_row, current_col, max_wait=4.0):
    """Find the best column to hop to for landing on a river platform.

    Searches for the column + wait combination that gets the frog onto
    a platform soonest, with minimal lateral movement.

    Returns (target_col, wait_time) or (None, None) if impossible.
    """
    rideables = [h for h in hazards_in_row if h.get("rideable", False)]
    if not rideables:
        return None, None

    best_col = None
    best_wait = max_wait
    best_score = float("inf")  # lower is better: wait + lateral_penalty

    # Check all columns within reasonable lateral distance
    for col in range(max(0, current_col - 4), min(GRID_COLS, current_col + 5)):
        frog_x = col * CELL_SIZE
        lateral_cost = abs(col - current_col) * POST_HOP_WAIT  # time to move there

        # Scan forward in time for a platform at this column
        for wait in _frange(0.0, max_wait, 0.02):
            arrival = wait + HOP_DURATION
            on_platform = False
            for h in rideables:
                hx = predict_hazard_x(h, arrival)
                half_w = h["width"] * CELL_SIZE * 0.5
                # Match game's FindPlatformAtCurrentPosition: |frog - hazard| <= half_w
                # Use small inset so we land well inside, not on the edge
                if abs(frog_x - hx) <= half_w - 20.0:
                    on_platform = True
                    break

            if on_platform:
                score = wait + lateral_cost
                if score < best_score:
                    best_score = score
                    best_col = col
                    best_wait = wait
                break  # Found earliest wait for this column

    return best_col, best_wait


def _frange(start, stop, step):
    """Float range generator."""
    val = start
    while val < stop:
        yield val
        val += step


def navigate_to_home_slot(pu, target_col=6, max_deaths=8):
    """Navigate frog from current position to a home slot.

    Strategy:
    - Road rows: find nearest safe column (where a gap exists NOW), move
      there, hop forward. No waiting — go where the gap IS.
    - River rows: find column + timing where a platform will arrive,
      move there on the safe row, hop onto the platform.
    - After each hop: re-query state for actual position.

    Args:
        pu: PlayUnreal client instance
        target_col: home slot column to aim for (default 6)
        max_deaths: give up after this many deaths

    Returns:
        dict with success, total_hops, deaths, elapsed, state
    """
    total_hops = 0
    deaths = 0
    initial_filled = None
    start = time.time()
    max_iterations = 100  # safety limit

    for _ in range(max_iterations):
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
            time.sleep(0.3)
            continue

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

        next_row = frog_row + 1

        # --- Safe row: align toward target, then hop forward ---
        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            # On safe rows, align laterally toward target
            if frog_row in SAFE_ROWS and frog_col != target_col:
                d = "right" if frog_col < target_col else "left"
                pu.hop(d)
                total_hops += 1
                time.sleep(POST_HOP_WAIT)
                continue

            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # --- Road row: find safe column, move there, hop forward ---
        if next_row in ROAD_ROWS:
            hazards = pu.get_hazards()
            row_hazards = [h for h in hazards
                           if h.get("row") == next_row and not h.get("rideable", False)]

            if not row_hazards:
                pu.hop("up")
                total_hops += 1
                time.sleep(POST_HOP_WAIT)
                continue

            safe_col, wait = find_safe_road_column(row_hazards, frog_col, target_col)

            # Move laterally to safe column
            while frog_col != safe_col:
                d = "right" if frog_col < safe_col else "left"
                pu.hop(d)
                total_hops += 1
                time.sleep(POST_HOP_WAIT)
                frog_col += 1 if d == "right" else -1

            if wait > 0.02:
                time.sleep(wait)

            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # --- River row: find platform column + timing ---
        if next_row in RIVER_ROWS:
            hazards = pu.get_hazards()
            row_hazards = [h for h in hazards if h.get("row") == next_row]

            plat_col, plat_wait = find_platform_column(
                row_hazards, frog_col, max_wait=6.0)

            if plat_col is None:
                # No platform found — try hopping anyway (will die, retry)
                pu.hop("up")
                total_hops += 1
                time.sleep(POST_HOP_WAIT)
                continue

            # Move laterally to platform column (only possible on safe rows)
            if frog_row in SAFE_ROWS:
                while frog_col != plat_col:
                    d = "right" if frog_col < plat_col else "left"
                    pu.hop(d)
                    total_hops += 1
                    time.sleep(POST_HOP_WAIT)
                    frog_col += 1 if d == "right" else -1

                # Re-query hazards after lateral movement (time has passed)
                if frog_col != int(pos[0]):
                    hazards = pu.get_hazards()
                    row_hazards = [h for h in hazards if h.get("row") == next_row]
                    _, plat_wait = find_platform_column(
                        row_hazards, frog_col, max_wait=6.0)
                    if plat_wait is None:
                        plat_wait = 0.0

            if plat_wait is not None and plat_wait > 0.02:
                time.sleep(plat_wait)

            pu.hop("up")
            total_hops += 1
            time.sleep(POST_HOP_WAIT)
            continue

        # Unknown row — hop forward
        pu.hop("up")
        total_hops += 1
        time.sleep(POST_HOP_WAIT)

    return _result(False, total_hops, deaths, start,
                   pu.get_state() if pu else {})


def _result(success, total_hops, deaths, start, state):
    return {
        "success": success,
        "total_hops": total_hops,
        "deaths": deaths,
        "elapsed": time.time() - start,
        "state": state,
    }


# Legacy API compatibility for verify_visuals.py
def plan_path(hazards, frog_col=6, frog_row=0, target_col=6,
              target_row=HOME_ROW):
    """Legacy API — returns list of {"wait", "direction"} dicts."""
    path = []
    current_col = frog_col
    current_row = frog_row

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

        next_row = current_row + 1

        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            continue

        row_hazards = hazards_by_row.get(next_row, [])
        if not row_hazards:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            continue

        if next_row in ROAD_ROWS:
            safe_col, wait = find_safe_road_column(row_hazards, current_col, target_col)
            while current_col != safe_col:
                d = "right" if current_col < safe_col else "left"
                path.append({"wait": 0.0, "direction": d})
                current_col += (1 if d == "right" else -1)
            path.append({"wait": wait, "direction": "up"})
        else:
            plat_col, plat_wait = find_platform_column(row_hazards, current_col)
            if plat_col is not None:
                while current_col != plat_col:
                    d = "right" if current_col < plat_col else "left"
                    path.append({"wait": 0.0, "direction": d})
                    current_col += (1 if d == "right" else -1)
                path.append({"wait": max(plat_wait or 0, 0.0), "direction": "up"})
            else:
                path.append({"wait": 0.5, "direction": "up"})

        current_row = next_row

    return path


def execute_path(pu, path):
    """Legacy API — execute a pre-planned path."""
    total_hops = 0
    start_time = time.time()
    for step in path:
        if step["wait"] > 0.02:
            time.sleep(step["wait"])
        pu.hop(step["direction"])
        total_hops += 1
        time.sleep(POST_HOP_WAIT)
    return {"hops": total_hops, "elapsed": time.time() - start_time}
