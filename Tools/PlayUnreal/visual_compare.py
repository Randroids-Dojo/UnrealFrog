"""Visual comparison tool — automates before/after screenshot workflow.

Checks out source files from a "before" git ref, builds, launches, screenshots,
then restores the "after" ref and repeats. Produces PNGs at multiple zoom levels
for side-by-side comparison so agents can prove visual changes are visible.

Usage:
    python3 Tools/PlayUnreal/visual_compare.py                      # HEAD~1 vs HEAD
    python3 Tools/PlayUnreal/visual_compare.py --before HEAD~3      # HEAD~3 vs HEAD
    python3 Tools/PlayUnreal/visual_compare.py --before abc123 --after def456
    python3 Tools/PlayUnreal/visual_compare.py --stash              # allow dirty tree

Exit codes:
    0 = both screenshots captured successfully
    1 = build or screenshot failure
    2 = invalid arguments or dirty working tree
"""

import argparse
import atexit
import os
import signal
import subprocess
import sys
import time

SCRIPT_DIR = os.path.dirname(os.path.abspath(__file__))
PROJECT_ROOT = os.path.abspath(os.path.join(SCRIPT_DIR, "..", ".."))
ENGINE_DIR = "/Users/Shared/Epic Games/UE_5.7"
BUILD_SH = os.path.join(ENGINE_DIR, "Engine", "Build", "BatchFiles", "Mac", "Build.sh")
EDITOR_APP = os.path.join(ENGINE_DIR, "Engine", "Binaries", "Mac",
                          "UnrealEditor.app", "Contents", "MacOS", "UnrealEditor")
PROJECT_FILE = os.path.join(PROJECT_ROOT, "UnrealFrog.uproject")

RC_PORT = 30010
RC_URL = f"http://localhost:{RC_PORT}/remote/info"
STARTUP_TIMEOUT = 120

# Zoom positions: name -> (X, Y, Z)
# Grid is 13x15 cells at 100 UU each. X=0..1200, Y=0..1400.
# Default camera: (600, 700, 2200) — full field view.
ZOOM_POSITIONS = {
    "full":  (600.0, 700.0, 2200.0),   # Full field (default gameplay view)
    "road":  (600.0, 250.0, 800.0),     # Close-up of road section (rows 1-5)
    "river": (600.0, 950.0, 800.0),     # Close-up of river section (rows 7-12)
    "frog":  (600.0, 50.0, 400.0),      # Tight on frog start position (row 0)
}

# Global editor PID for cleanup
_editor_pid = None


def log(msg):
    print(f"[VisualCompare] {msg}")


def run(cmd, check=True, capture=False, **kwargs):
    """Run a subprocess command."""
    if capture:
        result = subprocess.run(cmd, capture_output=True, text=True, **kwargs)
        if check and result.returncode != 0:
            raise RuntimeError(f"Command failed: {' '.join(cmd)}\n{result.stderr}")
        return result
    else:
        result = subprocess.run(cmd, **kwargs)
        if check and result.returncode != 0:
            raise RuntimeError(f"Command failed: {' '.join(cmd)}")
        return result


def git(*args, capture=True, check=True):
    """Run a git command in the project root."""
    result = run(["git", "-C", PROJECT_ROOT] + list(args),
                 capture=capture, check=check)
    if capture:
        return result.stdout.strip()
    return result


def validate_ref(ref):
    """Validate a git ref exists."""
    result = run(["git", "-C", PROJECT_ROOT, "rev-parse", "--verify", ref],
                 capture=True, check=False)
    if result.returncode != 0:
        log(f"ERROR: Invalid git ref: {ref}")
        sys.exit(2)
    return result.stdout.strip()


def is_tree_dirty():
    """Check if working tree has uncommitted changes."""
    status = git("status", "--porcelain", "--untracked-files=no")
    return len(status) > 0


def get_changed_source_files(before_ref, after_ref):
    """Get list of Source/ files changed between two refs."""
    output = git("diff", "--name-only", before_ref, after_ref, "--", "Source/")
    if not output:
        return []
    return [f for f in output.split("\n") if f.strip()]


def kill_editor():
    """Kill any running editor processes."""
    global _editor_pid
    if _editor_pid:
        try:
            os.kill(_editor_pid, signal.SIGTERM)
            time.sleep(2)
            os.kill(_editor_pid, signal.SIGKILL)
        except ProcessLookupError:
            pass
        _editor_pid = None

    subprocess.run(["pkill", "-f", "UnrealTraceServer"],
                   capture_output=True, check=False)
    subprocess.run(["pkill", "-f", "UnrealEditor"],
                   capture_output=True, check=False)
    time.sleep(3)


def build_editor():
    """Build the Editor target. Returns True on success."""
    log("Building Editor target...")
    result = subprocess.run(
        [BUILD_SH, "UnrealFrogEditor", "Mac", "Development", PROJECT_FILE],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        log("ERROR: Editor build failed.")
        lines = result.stdout.strip().split("\n")
        for line in lines[-5:]:
            log(f"  {line}")
        return False
    log("Editor build succeeded.")
    return True


def launch_editor():
    """Launch editor with RC API. Returns (subprocess.Popen, PlayUnreal) or None."""
    global _editor_pid

    kill_editor()

    log_path = os.path.join(PROJECT_ROOT, "Saved", "PlayUnreal",
                            f"compare_{int(time.time())}.log")
    os.makedirs(os.path.dirname(log_path), exist_ok=True)

    log("Launching editor...")
    with open(log_path, "w") as log_file:
        proc = subprocess.Popen(
            [EDITOR_APP, PROJECT_FILE,
             "-game", "-windowed", "-resx=1280", "-resy=720",
             "-log", "-RCWebControlEnable",
             "-ExecCmds=WebControl.EnableServerOnStartup 1"],
            stdout=log_file, stderr=log_file
        )
    _editor_pid = proc.pid
    log(f"  Editor PID: {_editor_pid}")

    # Wait for RC API
    log("Waiting for RC API...")
    elapsed = 0
    rc_ready = False
    while elapsed < STARTUP_TIMEOUT:
        try:
            result = subprocess.run(
                ["curl", "-s", "--connect-timeout", "2", RC_URL],
                capture_output=True, timeout=5
            )
            if result.returncode == 0:
                rc_ready = True
                break
        except subprocess.TimeoutExpired:
            pass

        if proc.poll() is not None:
            log("ERROR: Editor exited before RC API was ready.")
            return None

        time.sleep(2)
        elapsed += 2
        if elapsed % 10 == 0:
            log(f"  Still waiting... ({elapsed}s / {STARTUP_TIMEOUT}s)")

    if not rc_ready:
        log(f"ERROR: RC API did not respond within {STARTUP_TIMEOUT}s.")
        kill_editor()
        return None

    log(f"RC API ready (took ~{elapsed}s). Waiting 3s for actors...")
    time.sleep(3)

    sys.path.insert(0, SCRIPT_DIR)
    from client import PlayUnreal
    pu = PlayUnreal()

    if not pu.is_alive():
        log("ERROR: RC API not responding after launch.")
        kill_editor()
        return None

    # Reset game to get a clean state
    try:
        pu.reset_game()
        time.sleep(1.0)
    except Exception as e:
        log(f"WARNING: reset_game failed: {e}")

    return pu


def take_zoom_screenshots(pu, output_dir, prefix):
    """Take screenshots at all zoom levels. Returns list of (name, path) tuples."""
    results = []

    for name, (x, y, z) in ZOOM_POSITIONS.items():
        path = os.path.join(output_dir, f"{prefix}_{name}.png")

        try:
            pu.set_camera_position(x, y, z)
            time.sleep(0.5)  # Let the camera settle and frame render
        except Exception as e:
            log(f"  WARNING: set_camera_position failed for {name}: {e}")
            log(f"  Falling back to default camera position")

        success = pu.screenshot(path)
        if success and os.path.exists(path):
            abs_path = os.path.abspath(path)
            log(f"  [{name}] {abs_path}")
            results.append((name, abs_path))
        else:
            log(f"  [{name}] FAILED")

    # Reset camera to default after zoom shots
    try:
        pu.reset_camera_position()
    except Exception:
        pass

    return results


def main():
    parser = argparse.ArgumentParser(
        description="Visual before/after comparison for UnrealFrog")
    parser.add_argument("--before", default="HEAD~1",
                        help="Git ref for 'before' state (default: HEAD~1)")
    parser.add_argument("--after", default="HEAD",
                        help="Git ref for 'after' state (default: HEAD)")
    parser.add_argument("--stash", action="store_true",
                        help="Stash dirty working tree before starting")
    args = parser.parse_args()

    log("=== Visual Comparison Tool ===")
    log(f"  Before: {args.before}")
    log(f"  After:  {args.after}")
    log("")

    # Validate refs
    before_sha = validate_ref(args.before)
    after_sha = validate_ref(args.after)
    log(f"  Before SHA: {before_sha[:8]}")
    log(f"  After SHA:  {after_sha[:8]}")

    if before_sha == after_sha:
        log("ERROR: Before and after refs resolve to the same commit.")
        sys.exit(2)

    # Check dirty tree
    stashed = False
    if is_tree_dirty():
        if args.stash:
            log("Stashing dirty working tree...")
            git("stash", "push", "-m", "visual_compare auto-stash")
            stashed = True
        else:
            log("ERROR: Working tree has uncommitted changes.")
            log("       Commit or stash first, or use --stash flag.")
            sys.exit(2)

    # Find changed source files
    changed_files = get_changed_source_files(before_sha, after_sha)
    if not changed_files:
        log("WARNING: No Source/ files changed between refs.")
        log("         Comparison may not show visual differences.")
    else:
        log(f"Changed source files ({len(changed_files)}):")
        for f in changed_files[:10]:
            log(f"  {f}")
        if len(changed_files) > 10:
            log(f"  ... and {len(changed_files) - 10} more")
    log("")

    # Output directory
    timestamp = time.strftime("%Y%m%d_%H%M%S")
    output_dir = os.path.join(PROJECT_ROOT, "Saved", "Screenshots", "compare", timestamp)
    os.makedirs(output_dir, exist_ok=True)

    # Register cleanup to always restore files
    atexit.register(kill_editor)

    def restore_files():
        """Restore source files to after-ref state."""
        if changed_files:
            log("Restoring files to after state...")
            git("checkout", after_sha, "--", *changed_files, check=False)
        if stashed:
            log("Restoring stash...")
            git("stash", "pop", check=False)

    before_shots = []
    after_shots = []

    try:
        # === BEFORE phase ===
        log("=" * 50)
        log("BEFORE phase")
        log("=" * 50)

        if changed_files:
            log("Checking out before-ref source files...")
            git("checkout", before_sha, "--", *changed_files)
        else:
            log("No changed files — checking out full before-ref...")
            git("checkout", before_sha, "--", "Source/")

        if not build_editor():
            log("ERROR: Before-ref build failed. Restoring files.")
            restore_files()
            sys.exit(1)

        pu = launch_editor()
        if not pu:
            log("ERROR: Before-ref launch failed. Restoring files.")
            restore_files()
            sys.exit(1)

        log("Taking before screenshots at all zoom levels...")
        before_shots = take_zoom_screenshots(pu, output_dir, "before")
        kill_editor()

        if not before_shots:
            log("ERROR: No before screenshots captured. Restoring files.")
            restore_files()
            sys.exit(1)

        # === AFTER phase ===
        log("")
        log("=" * 50)
        log("AFTER phase")
        log("=" * 50)

        if changed_files:
            log("Restoring after-ref source files...")
            git("checkout", after_sha, "--", *changed_files)
        else:
            git("checkout", after_sha, "--", "Source/")

        if not build_editor():
            log("ERROR: After-ref build failed.")
            restore_files()
            sys.exit(1)

        pu = launch_editor()
        if not pu:
            log("ERROR: After-ref launch failed.")
            restore_files()
            sys.exit(1)

        log("Taking after screenshots at all zoom levels...")
        after_shots = take_zoom_screenshots(pu, output_dir, "after")
        kill_editor()

        if not after_shots:
            log("ERROR: No after screenshots captured.")
            restore_files()
            sys.exit(1)

    except Exception as e:
        log(f"ERROR: {e}")
        restore_files()
        sys.exit(1)

    # Restore stash if needed
    if stashed:
        log("Restoring stash...")
        git("stash", "pop", check=False)

    # === Output ===
    log("")
    log("=" * 50)
    log("COMPARISON COMPLETE")
    log("=" * 50)
    log("")

    # Build a lookup for easy pairing
    before_by_name = {name: path for name, path in before_shots}
    after_by_name = {name: path for name, path in after_shots}

    for name in ZOOM_POSITIONS:
        b = before_by_name.get(name, "MISSING")
        a = after_by_name.get(name, "MISSING")
        print(f"[{name.upper()}]")
        print(f"  [BEFORE] {b}")
        print(f"  [AFTER]  {a}")
        print()

    log("IMPORTANT: Do NOT trust commit messages. Examine each zoom level pair.")
    log("If you cannot see the claimed difference in a zoom shot, it is not visible.")
    log("")

    return 0


if __name__ == "__main__":
    try:
        sys.exit(main())
    except KeyboardInterrupt:
        log("Interrupted.")
        kill_editor()
        sys.exit(130)
