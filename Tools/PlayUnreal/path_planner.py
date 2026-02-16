"""Predictive safe-path planner for Frogger-style games.

Given a snapshot of hazard positions and velocities, computes a sequence
of timed hops that safely navigates the frog from start to a home slot.
No polling required during execution — query once, plan once, execute.

The algorithm:
1. Query all hazard positions + velocities (single RC API call)
2. For each row the frog must cross, predict hazard positions at future
   timestamps using linear extrapolation with wrapping
3. Find the earliest safe moment to hop into each row
4. Return a hop sequence: [{wait, direction}]

Usage:
    from path_planner import plan_path, execute_path

    hazards = pu.get_hazards()
    state = pu.get_state()
    path = plan_path(hazards, frog_col=6, frog_row=0, target_col=6)
    execute_path(pu, path)
"""

import math

# Game constants (defaults match UnrealFrog)
CELL_SIZE = 100.0
GRID_COLS = 13
HOP_DURATION = 0.15  # seconds
SAFE_ROWS = {0, 6, 13}
ROAD_ROWS = {1, 2, 3, 4, 5}
RIVER_ROWS = {7, 8, 9, 10, 11, 12}
HOME_ROW = 14


def predict_hazard_x(hazard, dt, cell_size=CELL_SIZE, grid_cols=GRID_COLS):
    """Predict a hazard's X position after dt seconds.

    Uses linear extrapolation with wrapping at grid boundaries.
    Matches HazardBase::TickMovement + WrapPosition exactly.

    Args:
        hazard: dict with keys x, speed, width, movesRight
        dt: time delta in seconds (can be negative for past)
        cell_size: grid cell size in UU
        grid_cols: number of grid columns

    Returns:
        Predicted X position in world units
    """
    x0 = hazard["x"]
    speed = hazard["speed"]
    direction = 1.0 if hazard["movesRight"] else -1.0
    world_width = hazard["width"] * cell_size
    grid_world_width = grid_cols * cell_size

    # Wrap boundaries (matches HazardBase::WrapPosition)
    wrap_min = -world_width
    wrap_max = grid_world_width + world_width
    wrap_range = wrap_max - wrap_min

    # Linear extrapolation with modular wrapping
    raw_x = x0 + speed * direction * dt
    wrapped_x = ((raw_x - wrap_min) % wrap_range) + wrap_min
    return wrapped_x


def is_road_safe(hazards_in_row, target_x, arrival_time,
                 cell_size=CELL_SIZE, grid_cols=GRID_COLS):
    """Check if target_x is clear of road hazards at arrival_time.

    A road position is safe when no hazard's extent overlaps the frog.
    The frog occupies approximately one cell centered at target_x.

    Args:
        hazards_in_row: list of hazard dicts for this row
        target_x: world X position where frog will land
        arrival_time: time from now when frog arrives (seconds)
        cell_size: grid cell size
        grid_cols: number of columns

    Returns:
        True if no hazard overlaps frog at target_x at arrival_time
    """
    frog_half = cell_size * 0.4  # slight margin inside a cell
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
    """Check if a rideable platform covers target_x at arrival_time.

    A river position is safe when at least one rideable (non-submerged)
    platform overlaps the frog's landing position.

    Args:
        hazards_in_row: list of hazard dicts for this row
        target_x: world X position where frog will land
        arrival_time: time from now when frog arrives
        cell_size: grid cell size
        grid_cols: number of columns

    Returns:
        True if a rideable platform covers target_x at arrival_time
    """
    for h in hazards_in_row:
        if not h.get("rideable", False):
            continue
        hx = predict_hazard_x(h, arrival_time, cell_size, grid_cols)
        hw = h["width"] * cell_size
        # Platform detection: frog center within platform bounds (with margin)
        margin = cell_size * 0.3
        if hx - margin <= target_x <= hx + hw + margin:
            return True
    return False


def _find_safe_wait(hazards_in_row, target_x, base_time, row,
                    cell_size=CELL_SIZE, grid_cols=GRID_COLS,
                    max_wait=10.0, time_step=0.01):
    """Find the minimum wait time before hopping into a row is safe.

    Scans forward in time_step increments from base_time, checking
    whether the frog can safely land at target_x after HOP_DURATION.

    Args:
        hazards_in_row: hazards in the target row
        target_x: landing X position
        base_time: current elapsed time in the plan
        row: target row number
        max_wait: maximum seconds to wait before giving up
        time_step: simulation resolution (seconds)

    Returns:
        Wait time in seconds (0.0 if immediately safe, or max_wait if
        no safe window found)
    """
    is_river = row in RIVER_ROWS

    wait = 0.0
    while wait < max_wait:
        arrival = base_time + wait + HOP_DURATION
        if is_river:
            safe = is_river_safe(hazards_in_row, target_x, arrival,
                                 cell_size, grid_cols)
        else:
            safe = is_road_safe(hazards_in_row, target_x, arrival,
                                cell_size, grid_cols)
        if safe:
            return wait
        wait += time_step

    return max_wait  # No safe window found


def plan_path(hazards, frog_col=6, frog_row=0, target_col=6,
              target_row=HOME_ROW, cell_size=CELL_SIZE,
              grid_cols=GRID_COLS, max_wait_per_row=10.0):
    """Compute a safe hop sequence from start to home slot.

    Given a single snapshot of all hazard positions and velocities,
    predicts their future positions and finds safe crossing windows
    for every row between frog_row and target_row.

    Args:
        hazards: list of hazard dicts from get_hazards()
        frog_col: starting grid column
        frog_row: starting grid row
        target_col: target home slot column (must be valid: 1,4,6,8,11)
        target_row: target row (default 14)
        cell_size: grid cell size in UU
        grid_cols: number of columns
        max_wait_per_row: max seconds to wait for a safe gap per row

    Returns:
        list of dicts: [{"wait": float, "direction": str}, ...]
        wait = seconds to sleep before this hop
        direction = "up", "down", "left", or "right"
    """
    path = []
    current_col = frog_col
    current_row = frog_row
    elapsed = 0.0

    # Group hazards by row for fast lookup
    hazards_by_row = {}
    for h in hazards:
        row = h.get("row", -1)
        if row not in hazards_by_row:
            hazards_by_row[row] = []
        hazards_by_row[row].append(h)

    while current_row < target_row:
        # Phase 1: Lateral alignment on safe rows
        if current_row in SAFE_ROWS and current_col != target_col:
            while current_col != target_col:
                if current_col < target_col:
                    path.append({"wait": 0.0, "direction": "right"})
                    current_col += 1
                else:
                    path.append({"wait": 0.0, "direction": "left"})
                    current_col -= 1
                elapsed += HOP_DURATION

        # Phase 2: Hop forward into next row
        next_row = current_row + 1
        target_x = target_col * cell_size

        if next_row in SAFE_ROWS or next_row >= HOME_ROW:
            # Safe row or home — hop immediately
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += HOP_DURATION
            continue

        # Dangerous row — find safe window
        row_hazards = hazards_by_row.get(next_row, [])
        if not row_hazards:
            # No hazards in this row — hop immediately
            path.append({"wait": 0.0, "direction": "up"})
            current_row = next_row
            elapsed += HOP_DURATION
            continue

        wait = _find_safe_wait(row_hazards, target_x, elapsed, next_row,
                               cell_size, grid_cols, max_wait_per_row)
        path.append({"wait": wait, "direction": "up"})
        current_row = next_row
        elapsed += wait + HOP_DURATION

    return path


def execute_path(pu, path, hop_margin=0.10):
    """Execute a pre-computed hop sequence via PlayUnreal client.

    Sends hops with precise timing — no hazard polling during execution.
    Total RC API calls = len(path) (one hop() per step).

    Args:
        pu: PlayUnreal client instance
        path: list from plan_path()
        hop_margin: extra seconds to wait after each hop for animation
                    (HOP_DURATION is 0.15s, margin adds safety buffer)

    Returns:
        dict with execution stats:
            hops: number of hops executed
            elapsed: total time in seconds
            aborted: True if game state changed unexpectedly
    """
    import time

    total_hops = 0
    start_time = time.time()

    for i, step in enumerate(path):
        # Wait the computed delay
        if step["wait"] > 0.01:
            time.sleep(step["wait"])

        # Check game state periodically (every 3 hops) to detect death
        if i > 0 and i % 3 == 0:
            try:
                state = pu.get_state()
                gs = state.get("gameState", "")
                if gs not in ("Playing", "Spawning"):
                    return {
                        "hops": total_hops,
                        "elapsed": time.time() - start_time,
                        "aborted": True,
                        "reason": f"Game state changed to {gs}",
                        "state": state,
                    }
            except Exception:
                pass

        # Execute hop
        pu.hop(step["direction"])
        total_hops += 1
        time.sleep(HOP_DURATION + hop_margin)

    return {
        "hops": total_hops,
        "elapsed": time.time() - start_time,
        "aborted": False,
        "state": None,
    }


def navigate_to_home_slot(pu, target_col=6, max_attempts=5):
    """High-level navigation: plan path, execute, retry on death.

    Queries hazards once per attempt, computes full path, executes it.
    If the frog dies, re-queries and re-plans from the new position.

    Args:
        pu: PlayUnreal client instance
        target_col: home slot column (1, 4, 6, 8, 11)
        max_attempts: max death-and-retry cycles

    Returns:
        dict with:
            success: bool
            total_hops: int
            deaths: int
            elapsed: float
    """
    import time

    total_hops = 0
    deaths = 0

    for attempt in range(max_attempts):
        # Get current state
        state = pu.get_state()
        gs = state.get("gameState", "")

        # Wait for Playing state
        if gs != "Playing":
            try:
                state = pu.wait_for_state("Playing", timeout=5)
            except Exception:
                continue

        pos = state.get("frogPos", [6, 0])
        frog_col = int(pos[0]) if isinstance(pos, list) else 6
        frog_row = int(pos[1]) if isinstance(pos, list) and len(pos) > 1 else 0

        # Query hazards (single API call)
        hazards = pu.get_hazards()

        # Plan path (pure computation, no API calls)
        path = plan_path(hazards, frog_col=frog_col, frog_row=frog_row,
                         target_col=target_col)

        # Execute path (one API call per hop, no polling)
        result = execute_path(pu, path)
        total_hops += result["hops"]

        if not result["aborted"]:
            # Check if we reached the home slot
            final_state = pu.get_state()
            home_filled = final_state.get("homeSlotsFilledCount", 0)
            if home_filled > 0 or final_state.get("gameState") == "RoundComplete":
                return {
                    "success": True,
                    "total_hops": total_hops,
                    "deaths": deaths,
                    "elapsed": result["elapsed"],
                    "state": final_state,
                }

        # Frog likely died — wait for respawn and retry
        deaths += 1
        time.sleep(2.0)

    return {
        "success": False,
        "total_hops": total_hops,
        "deaths": deaths,
        "elapsed": 0.0,
        "state": pu.get_state() if pu else {},
    }
