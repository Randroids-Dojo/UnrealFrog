"""PlayUnreal client — Python interface to UnrealFrog via Remote Control API.

Uses only urllib.request (no pip dependencies). Requires the editor running
with RemoteControl plugin enabled on localhost:30010.

Usage:
    from client import PlayUnreal
    pu = PlayUnreal()
    pu.reset_game()
    pu.hop("up")
    state = pu.get_state()
    print(state)
"""

import json
import time
import urllib.request
import urllib.error


class PlayUnrealError(Exception):
    """Base exception for PlayUnreal client errors."""
    pass


class ConnectionError(PlayUnrealError):
    """Editor is not running or Remote Control API is not responding."""
    pass


class CallError(PlayUnrealError):
    """A remote function call or property read failed."""
    pass


# Direction name -> FVector as dict for RC API
_DIRECTIONS = {
    "up":    {"X": 0.0, "Y": 1.0, "Z": 0.0},
    "down":  {"X": 0.0, "Y": -1.0, "Z": 0.0},
    "left":  {"X": -1.0, "Y": 0.0, "Z": 0.0},
    "right": {"X": 1.0, "Y": 0.0, "Z": 0.0},
}


class PlayUnreal:
    """Client for controlling UnrealFrog via Remote Control API.

    Args:
        host: RC API host (default localhost)
        port: RC API port (default 30010)
        timeout: HTTP request timeout in seconds (default 5)
    """

    def __init__(self, host="localhost", port=30010, timeout=5):
        self.base_url = f"http://{host}:{port}"
        self.timeout = timeout
        self._gm_path = None
        self._frog_path = None

    # -- Public API ----------------------------------------------------------

    def hop(self, direction):
        """Send a hop command to the frog.

        Args:
            direction: "up", "down", "left", or "right"
        """
        if direction not in _DIRECTIONS:
            raise ValueError(f"Invalid direction '{direction}'. Use: up, down, left, right")
        frog_path = self._get_frog_path()
        self._call_function(frog_path, "RequestHop", {
            "Direction": _DIRECTIONS[direction]
        })

    def get_state(self):
        """Get current game state as a dict.

        Returns:
            dict with keys: score, lives, wave, frogPos, gameState,
            timeRemaining, homeSlotsFilledCount

        If GetGameStateJSON() exists on the GameMode, uses that (single call).
        Otherwise falls back to reading individual properties.
        """
        gm_path = self._get_gm_path()

        # Try GetGameStateJSON first (Task 5)
        try:
            result = self._call_function(gm_path, "GetGameStateJSON")
            # The function returns an FString containing JSON
            ret_val = result.get("ReturnValue", "")
            if ret_val:
                return json.loads(ret_val)
        except (CallError, json.JSONDecodeError):
            pass

        # Fallback: read individual properties from GameMode and Frog
        state = {}
        try:
            state["gameState"] = self._read_property(gm_path, "CurrentState")
            state["wave"] = self._read_property(gm_path, "CurrentWave")
            state["homeSlotsFilledCount"] = self._read_property(gm_path, "HomeSlotsFilledCount")
            state["timeRemaining"] = self._read_property(gm_path, "RemainingTime")
        except CallError:
            pass

        try:
            frog_path = self._get_frog_path()
            grid_pos = self._read_property(frog_path, "GridPosition")
            if isinstance(grid_pos, dict):
                state["frogPos"] = [grid_pos.get("X", 0), grid_pos.get("Y", 0)]
            else:
                state["frogPos"] = [0, 0]
        except CallError:
            state["frogPos"] = [0, 0]

        return state

    def screenshot(self, path=None):
        """Take a screenshot of the current frame.

        Args:
            path: Optional file path. If None, uses engine default location.
        """
        gm_path = self._get_gm_path()
        # Use console command via RC API to trigger screenshot
        cmd = "HighResShot 1"
        if path:
            cmd = f"HighResShot 1 filename={path}"
        try:
            self._call_function(
                "/Engine/Transient.GameEngine",
                "ConsoleCommand",
                {"Command": cmd}
            )
        except CallError:
            # Fallback: try via the game mode's world
            # The screenshot might not work through RC API — document limitation
            pass

    def reset_game(self):
        """Reset the game to title screen and start a new game."""
        gm_path = self._get_gm_path()
        self._call_function(gm_path, "ReturnToTitle")
        time.sleep(0.5)
        self._call_function(gm_path, "StartGame")

    def wait_for_state(self, target_state, timeout=10):
        """Poll get_state() until gameState matches target.

        Args:
            target_state: Target game state string (e.g., "Playing", "Title")
            timeout: Max seconds to wait

        Returns:
            The matching state dict

        Raises:
            PlayUnrealError: If timeout reached without matching state
        """
        start = time.time()
        while time.time() - start < timeout:
            state = self.get_state()
            current = state.get("gameState", "")
            # Handle both string and enum representations
            if isinstance(current, str) and target_state.lower() in current.lower():
                return state
            if isinstance(current, int):
                # EGameState enum values: Title=0, Spawning=1, Playing=2, etc.
                state_map = {0: "Title", 1: "Spawning", 2: "Playing",
                             3: "Paused", 4: "Dying", 5: "RoundComplete", 6: "GameOver"}
                if state_map.get(current, "").lower() == target_state.lower():
                    return state
            time.sleep(0.2)
        raise PlayUnrealError(
            f"Timed out waiting for state '{target_state}' after {timeout}s. "
            f"Last state: {state.get('gameState', 'unknown')}")

    def is_alive(self):
        """Check if the Remote Control API is responding.

        Returns:
            True if the API responds, False otherwise.
        """
        try:
            self._get("/remote/info")
            return True
        except ConnectionError:
            return False

    # -- Object path discovery -----------------------------------------------

    def _get_gm_path(self):
        """Get the object path of the live GameMode instance."""
        if self._gm_path:
            return self._gm_path
        self._gm_path = self._discover_gm_path()
        return self._gm_path

    def _get_frog_path(self):
        """Get the object path of the live FrogCharacter instance."""
        if self._frog_path:
            return self._frog_path
        self._frog_path = self._discover_frog_path()
        return self._frog_path

    def _discover_gm_path(self):
        """Discover the GameMode object path by trying known patterns.

        In -game mode, actors live under the persistent level of the loaded map.
        Since SearchActor is not implemented in UE 5.7, we try common paths.
        The project's default map is FroggerMain (Config/DefaultEngine.ini).
        """
        # Try live instance paths first (these reflect runtime state)
        # then CDO as fallback (always exists but won't reflect runtime state)
        candidates = []

        # FroggerMain is the project's default map — try it first
        # Other map names as fallback in case the config changes
        for map_name in ["FroggerMain", "FroggerMap", "TestMap", "DefaultMap"]:
            candidates.append(
                f"/Game/Maps/{map_name}.{map_name}:PersistentLevel.UnrealFrogGameMode_0"
            )
            candidates.append(
                f"/Game/Maps/{map_name}.{map_name}:PersistentLevel.UnrealFrogGameMode_C_0"
            )

        # CDO fallback — works for function calls but state reads return defaults
        candidates.append("/Script/UnrealFrog.Default__UnrealFrogGameMode")

        for path in candidates:
            try:
                result = self._describe_object(path)
                if result:
                    return path
            except (CallError, ConnectionError):
                continue

        # Last resort: use CDO
        return "/Script/UnrealFrog.Default__UnrealFrogGameMode"

    def _discover_frog_path(self):
        """Discover the FrogCharacter object path."""
        # FroggerMain is the project's default map
        for map_name in ["FroggerMain", "FroggerMap", "TestMap", "DefaultMap"]:
            for suffix in ["FrogCharacter_0", "FrogCharacter_C_0"]:
                path = f"/Game/Maps/{map_name}.{map_name}:PersistentLevel.{suffix}"
                try:
                    result = self._describe_object(path)
                    if result:
                        return path
                except (CallError, ConnectionError):
                    continue

        # CDO fallback
        return "/Script/UnrealFrog.Default__FrogCharacter"

    # -- Low-level RC API calls ----------------------------------------------

    def _call_function(self, object_path, function_name, parameters=None):
        """Call a UFUNCTION via Remote Control API.

        Args:
            object_path: UE object path
            function_name: Name of the BlueprintCallable function
            parameters: Dict of parameters (optional)

        Returns:
            Response body as dict
        """
        body = {
            "ObjectPath": object_path,
            "FunctionName": function_name,
        }
        if parameters:
            body["Parameters"] = parameters

        return self._put("/remote/object/call", body)

    def _read_property(self, object_path, property_name):
        """Read a UPROPERTY value via Remote Control API.

        Args:
            object_path: UE object path
            property_name: Name of the property

        Returns:
            The property value (parsed from JSON)
        """
        body = {
            "ObjectPath": object_path,
            "PropertyName": property_name,
        }
        result = self._put("/remote/object/property", body)
        # RC API returns {"PropertyName": value} for property reads
        return result.get(property_name, result)

    def _describe_object(self, object_path):
        """Describe an object (list its properties and functions).

        Args:
            object_path: UE object path

        Returns:
            Description dict or None if object not found
        """
        body = {"ObjectPath": object_path}
        try:
            return self._put("/remote/object/describe", body)
        except CallError:
            return None

    # -- HTTP transport ------------------------------------------------------

    def _get(self, endpoint):
        """Send a GET request to the RC API."""
        url = f"{self.base_url}{endpoint}"
        try:
            req = urllib.request.Request(url, method="GET")
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                return json.loads(resp.read().decode("utf-8"))
        except urllib.error.URLError as e:
            raise ConnectionError(
                f"Cannot reach Remote Control API at {self.base_url}. "
                f"Is the editor running with -RCWebControlEnable? Error: {e}"
            )
        except json.JSONDecodeError:
            return {}

    def _put(self, endpoint, body):
        """Send a PUT request with JSON body to the RC API."""
        url = f"{self.base_url}{endpoint}"
        data = json.dumps(body).encode("utf-8")
        try:
            req = urllib.request.Request(
                url,
                data=data,
                method="PUT",
                headers={"Content-Type": "application/json"}
            )
            with urllib.request.urlopen(req, timeout=self.timeout) as resp:
                resp_body = resp.read().decode("utf-8")
                if resp_body:
                    return json.loads(resp_body)
                return {}
        except urllib.error.HTTPError as e:
            error_body = ""
            try:
                error_body = e.read().decode("utf-8")
            except Exception:
                pass
            raise CallError(
                f"RC API call failed: {e.code} {e.reason} "
                f"on {endpoint}. Body: {error_body}"
            )
        except urllib.error.URLError as e:
            raise ConnectionError(
                f"Cannot reach Remote Control API at {self.base_url}. "
                f"Is the editor running with -RCWebControlEnable? Error: {e}"
            )
        except json.JSONDecodeError:
            return {}


# -- CLI entrypoint for quick testing ----------------------------------------

if __name__ == "__main__":
    import sys

    pu = PlayUnreal()
    if not pu.is_alive():
        print("ERROR: Remote Control API is not responding on localhost:30010")
        print("       Launch the editor with: run-playunreal.sh")
        sys.exit(1)

    print("Remote Control API is alive.")
    print(f"GameMode path: {pu._get_gm_path()}")
    print(f"Frog path: {pu._get_frog_path()}")
    print()

    try:
        state = pu.get_state()
        print(f"Game state: {json.dumps(state, indent=2)}")
    except PlayUnrealError as e:
        print(f"Could not get state: {e}")
