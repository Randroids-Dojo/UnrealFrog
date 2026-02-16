"""Predictive safe-path planner for Frogger-style games.

Given a snapshot of hazard positions and velocities, computes a sequence
of timed hops that safely navigates the frog from start to a home slot.
No polling required during execution — query once, plan once, execute.

Optimized for speed: short waits, tight timing, fast retries on death.
A full crossing (14 rows) executes in ~4-6 seconds.

Usage:
    from path_planner import plan_path, execute_path

    hazards = pu.get_hazards()
    path = plan_path(hazards, frog_col=6, frog_row=0, target_col=6)
    execute_path(pu, path)
"""

# Game constants (defaults match UnrealFrog)
CELL_SIZE = 100.0
GRID_COLS = 13
HOP_DURATION = 0.15  # seconds
SAFE_ROWS = {0, 6, 13}
ROAD_ROWS = {1, 2, 3, 4, 5}
RIVER_ROWS = {7, 8, 9, 10, 11, 12}
HOME_ROW = 14

# Execution timing — tuned for speed
# RC API call takes ~50-100ms, so real hop-to-hop time is ~0.20-0.25s.
# We account for this in predictions by using API_LATENCY.
API_LATENCY = 0.08  # estimated RC API round-trip
HOP_EXEC_TIME = HOP_DURATION + API_LATENCY  # ~0.23s real time per hop


def predict_hazard_x(hazard, dt, cell_size=CELL_SIZE, grid_cols=GRID_COLS):
    """Predict a hazard's X position after dt seconds.

    Linear extrapolation with wrapping. Matches HazardBase exactly.
    """
    x0 = hazard["x"]
    speed = hazard["speed"]
    direction = 1.0 if hazard["movesRight"] else -1.0
    world_width = hazard["width"] * cell_size
    grid_world_width = grid_cols * cell_size

    wrap_min = -world_width
    wrap_max = grid_world_width + world_width
    wrap_range = wrap_max - wrap_min

    raw_x = x0 + speed * direction * dt
    return ((raw_x - wrap_min) % wrap_range) + wrap_min


def is_road_safe(hazards_in_row, target_x, arrival_time,
                 cell_size=CELL_SIZE, grid_cols=GRID_COLS):
    """Check if target_x is clear of road hazards at arrival_time."""
    frog_half = cell_size * 0.4
    frog_left = target_x - frog_half
    frog_right = target_x + frog_half

    for h in hazards_in_row:
        hx = predict_hazard_x(h, arrival_time, cell_size, grid_cols)
        hw = h["width"] * cell_size
        if frog_right > hx and frog_left < hx + hw:
            return False
    return True


def is_river_safe(hazards_in_row, target_x, arrival_time,
                  cell_size=CELL_SIZE, grid_cols=GRID_COLS):
    """Check if a rideable platform covers target_x at arrival_time."""
    for h in hazards_in_row:
        if not h.get("rideable", False):
            continue
        hx = predict_hazard_x(h, arrival_time, cell_size, grid_cols)
        hw = h["width"] * cell_size
        margin = cell_size * 0.3
        if hx - margin <= target_x <= hx + hw + margin:
            return True
    return False


def _find_safe_wait(hazards_in_row, target_x, base_time, row,
                    cell_size=CELL_SIZE, grid_cols=GRID_COLS,
                    max_wait=1.5, time_step=0.02):
    """Find minimum wait before hopping is safe. Max 1.5s then go anyway."""
    is_river = row in RIVER_ROWS

    wait = 0.0
    while wait < max_wait:
        arrival = base_time + wait + HOP_DURATION
        if is_river:
            if is_river_safe(hazards_in_row, target_x, arrival,
                             cell_size, grid_cols):
                return wait
        else:
            if is_road_safe(hazards_in_row, target_x, arrival,
                            cell_size, grid_cols):
                return wait
        wait += time_step

    return 0.0  # No safe window found — hop anyway, accept the risk


def plan_path(hazards, frog_col=6, frog_row=0, target_col=6,
              target_row=HOME_ROW, cell_size=CELL_SIZE,
              grid_cols=GRID_COLS):
    """Compute a fast safe hop sequence from start to home slot.

    Aggressive timing: max 1.5s wait per row, hops immediately if no
    safe window found. Total path executes in ~4-6 seconds.

    Returns:
        list of dicts: [{"wait": float, "direction": str}, ...]
    """
    path = []
    current_col = frog_col
    current_row = frog_row
    elapsed = 0.0

    # Group hazards by row
    hazards_by_row = {}
    for h in hazards:
        r = h.get("row", -1)
        if r not in hazards_by_row:
            hazards_by_row[r] = []
        hazards_by_row[r].append(h)

    while current_row < target_row:
        # Lateral alignment on safe rows
        if current_row in SAFE_ROWS and current_col != target_col:
            while current_col != target_col:
                d = "right" if current_col < target_col else "left"
                path.append({"wait": 0.0, "direction": d})
                current_col += (1 if d == "right" else -1)
                elapsed += HOP_EXEC_TIME

        next_row = current_row + 1
        target_x = target_col * cell_size

        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += HOP_EXEC_TIME
            continue

        row_hazards = hazards_by_row.get(next_row, [])
        if not row_hazards:
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += HOP_EXEC_TIME
            continue

        wait = _find_safe_wait(row_hazards, target_x, elapsed, next_row,
                               cell_size, grid_cols)
        path.append({"wait": wait, "direction": "up"})
        current_row = next_row
        elapsed += wait + HOP_EXEC_TIME

    return path


def execute_path(pu, path):
    """Execute a hop sequence as fast as possible.

    No state checks during execution — just blast through.
    One API call per hop, minimal sleep between hops.
    """
    import time

    total_hops = 0
    start_time = time.time()

    for step in path:
        if step["wait"] > 0.02:
            time.sleep(step["wait"])

        pu.hop(step["direction"])
        total_hops += 1
        # Minimal wait — just enough for hop animation to complete.
        # The UE input buffer opens at 0.08s into a 0.15s hop,
        # so we only need to wait ~0.15s before the next hop is accepted.
        time.sleep(HOP_DURATION + 0.02)

    elapsed = time.time() - start_time
    return {
        "hops": total_hops,
        "elapsed": elapsed,
    }


def navigate_to_home_slot(pu, target_col=6, max_attempts=8):
    """Plan + execute with fast retries on death.

    Each attempt: query hazards (1 call), plan (0 calls), execute (N calls).
    On death: wait for respawn (1.5s), retry from new position.
    """
    import time

    total_hops = 0
    deaths = 0
    initial_filled = None
    start = time.time()

    for attempt in range(max_attempts):
        state = pu.get_state()
        gs = state.get("gameState", "")

        if gs == "GameOver":
            return {"success": False, "total_hops": total_hops,
                    "deaths": deaths, "elapsed": time.time() - start,
                    "state": state}

        if gs in ("Dying", "Spawning"):
            time.sleep(1.5)  # DyingDuration=0.5 + SpawningDuration=1.0
            continue

        if gs == "RoundComplete":
            return {"success": True, "total_hops": total_hops,
                    "deaths": deaths, "elapsed": time.time() - start,
                    "state": state}

        if gs != "Playing":
            try:
                state = pu.wait_for_state("Playing", timeout=5)
            except Exception:
                continue

        # Track home slot fills
        filled = state.get("homeSlotsFilledCount", 0)
        if initial_filled is None:
            initial_filled = filled
        if filled > initial_filled:
            return {"success": True, "total_hops": total_hops,
                    "deaths": deaths, "elapsed": time.time() - start,
                    "state": state}

        pos = state.get("frogPos", [6, 0])
        frog_col = int(pos[0]) if isinstance(pos, list) else 6
        frog_row = int(pos[1]) if isinstance(pos, list) and len(pos) > 1 else 0

        # Query hazards ONCE
        hazards = pu.get_hazards()

        # Plan (pure math, instant)
        path = plan_path(hazards, frog_col=frog_col, frog_row=frog_row,
                         target_col=target_col)

        # Execute (fast — one API call per hop, ~0.17s per hop)
        result = execute_path(pu, path)
        total_hops += result["hops"]

        # Check outcome
        time.sleep(0.3)
        final = pu.get_state()
        final_gs = final.get("gameState", "")
        final_filled = final.get("homeSlotsFilledCount", 0)

        if final_filled > (initial_filled or 0):
            return {"success": True, "total_hops": total_hops,
                    "deaths": deaths, "elapsed": time.time() - start,
                    "state": final}
        if final_gs == "RoundComplete":
            return {"success": True, "total_hops": total_hops,
                    "deaths": deaths, "elapsed": time.time() - start,
                    "state": final}

        # Died or still playing — retry
        if final_gs in ("Dying", "Spawning", "GameOver"):
            deaths += 1
            time.sleep(1.5)
        else:
            # Still playing but didn't fill — maybe we're off-target, retry
            deaths += 1

    return {"success": False, "total_hops": total_hops,
            "deaths": deaths, "elapsed": time.time() - start,
            "state": pu.get_state() if pu else {}}
