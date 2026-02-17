"""Focused VFX visibility test — triggers each VFX type and captures screenshots.

Tests: death puff, hop dust, home slot sparkle.
Each test takes a before/after screenshot pair to show the VFX effect.

Usage:
    ./Tools/PlayUnreal/run-playunreal.sh test_vfx_visibility.py
"""
import os
import sys
import time

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from client import PlayUnreal

SCREENSHOT_DIR = os.path.join(
    os.path.dirname(os.path.abspath(__file__)),
    "..", "..", "Saved", "Screenshots", "vfx_test"
)

def log(msg):
    print(f"[VFXTest] {msg}")

def screenshot(pu, name):
    path = os.path.join(SCREENSHOT_DIR, f"{name}.png")
    pu.screenshot(path)
    log(f"  [SCREENSHOT] {os.path.abspath(path)}")
    return path

def main():
    log("=== VFX Visibility Test ===")

    pu = PlayUnreal()
    if not pu.is_alive():
        log("FATAL: RC API not responding")
        return 1

    os.makedirs(SCREENSHOT_DIR, exist_ok=True)
    gm_path = pu._get_gm_path()

    # --- Test 1: Hop Dust ---
    log("")
    log("--- Test 1: Hop Dust VFX ---")
    pu._call_function(gm_path, "ReturnToTitle")
    time.sleep(0.5)
    pu._call_function(gm_path, "StartGame")
    time.sleep(2.0)

    screenshot(pu, "01_before_hop")
    pu.hop("right")
    time.sleep(0.08)  # Capture during hop (dust is at origin)
    screenshot(pu, "02_during_hop_dust")
    time.sleep(0.2)
    screenshot(pu, "03_after_hop")

    state = pu.get_state()
    log(f"  Frog pos: {state.get('frogPos')}, state: {state.get('gameState')}")

    # --- Test 2: Death Puff ---
    log("")
    log("--- Test 2: Death Puff VFX ---")
    screenshot(pu, "04_before_death")

    # Hop into traffic (row 1)
    pu.hop("up")
    time.sleep(0.2)
    screenshot(pu, "05_forward_hop")

    # Wait a moment for death
    time.sleep(1.0)
    screenshot(pu, "06_death_puff")

    state = pu.get_state()
    log(f"  State: {state.get('gameState')}, lives: {state.get('lives')}")

    # Wait for respawn and try again to get a clear death shot
    time.sleep(2.0)
    state = pu.get_state()
    if state.get("gameState") == "Playing":
        pu.hop("up")
        time.sleep(0.15)  # Right as hop finishes
        screenshot(pu, "07_death_puff_2")
        time.sleep(0.5)
        screenshot(pu, "08_death_puff_expanding")

    # --- Test 3: Home Slot Sparkle ---
    log("")
    log("--- Test 3: Home Slot Sparkle VFX ---")
    # Use invincibility for reliable navigation
    pu.set_invincible(True)
    pu._call_function(gm_path, "ReturnToTitle")
    time.sleep(0.5)
    pu._call_function(gm_path, "StartGame")
    time.sleep(2.0)
    pu.set_invincible(True)  # Re-set after restart

    # Navigate to home slot row (14 hops up)
    for i in range(14):
        pu.hop("up")
        time.sleep(0.2)

    state = pu.get_state()
    log(f"  After 14 hops: pos={state.get('frogPos')}, state={state.get('gameState')}")

    screenshot(pu, "09_at_home_row")
    time.sleep(0.5)
    screenshot(pu, "10_home_sparkle")
    time.sleep(0.5)
    screenshot(pu, "11_home_sparkle_2")

    final = pu.get_state()
    log(f"  Final: score={final.get('score')}, homeFilled={final.get('homeSlotsFilledCount')}")
    log(f"  Wave: {final.get('wave')}, state: {final.get('gameState')}")

    log("")
    log("=== VFX Test Complete ===")
    log(f"Screenshots saved to: {os.path.abspath(SCREENSHOT_DIR)}")
    return 0

if __name__ == "__main__":
    sys.exit(main())
