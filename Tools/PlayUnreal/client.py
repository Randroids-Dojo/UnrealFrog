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


class RCConnectionError(PlayUnrealError):
    """Editor is not running or Remote Control API is not responding."""
    pass

# Keep backward-compatible alias (shadows builtin, but existing code uses it)
ConnectionError = RCConnectionError


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

# Map name from Config/DefaultEngine.ini
_DEFAULT_MAP = "FroggerMain"


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

        Uses the RC API's HTTP endpoint to send a console command.
        HighResShot triggers the engine screenshot pipeline.

        Args:
            path: Optional file path. If None, uses engine default location.

        Returns:
            True if the command was sent (does not guarantee file was written).
        """
        # Build console command — HighResShot saves to Saved/Screenshots/
        cmd = "HighResShot 1920x1080"
        if path:
            cmd = f"HighResShot 1920x1080 filename=\"{path}\""

        # Try sending via the PlayerController (most reliable in -game mode)
        frog_path = self._get_frog_path()
        # The controller owns the viewport — use the GameMode to run a command
        gm_path = self._get_gm_path()

        # RC API /remote/object/call can invoke console commands through any
        # UObject that has a world context.  We try the GameMode first.
        for target in [gm_path, frog_path]:
            try:
                # UE RC API does not have a dedicated "console command" endpoint.
                # Instead, use the /remote/object/property endpoint to run
                # ScreenshotRequested or use a custom UFUNCTION.
                # For now, use a direct HTTP call via the engine's HTTP router.
                self._put("/remote/object/call", {
                    "ObjectPath": target,
                    "FunctionName": "ConsoleCommand",
                    "Parameters": {"Command": cmd},
                    "GenerateTransaction": False
                })
                return True
            except CallError:
                continue

        # Last resort: try undocumented batch endpoint
        try:
            self._put("/remote/object/call", {
                "ObjectPath": "/Engine/Transient.GameEngine",
                "FunctionName": "ConsoleCommand",
                "Parameters": {"Command": cmd}
            })
            return True
        except CallError:
            return False

    def reset_game(self):
        """Reset the game to title screen and start a new game.

        Calls ReturnToTitle, waits for Title state, then StartGame.
        After StartGame the game goes Spawning->Playing (SpawningDuration=1s).
        """
        gm_path = self._get_gm_path()
        self._call_function(gm_path, "ReturnToTitle")
        time.sleep(0.5)
        self._call_function(gm_path, "StartGame")
        # Wait for Playing state — StartGame enters Spawning first
        time.sleep(1.5)

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

    def diagnose(self):
        """Probe every aspect of the RC API connection and report findings.

        Returns a dict with detailed diagnostic information suitable for
        remote debugging.  Each section reports what was tested, what worked,
        and what failed.
        """
        report = {
            "connection": {},
            "routes": [],
            "gamemode_discovery": {},
            "frog_discovery": {},
            "get_game_state": {},
            "hop_test": {},
            "property_reads": {},
            "screenshot": {},
        }

        # 1. Connection check
        try:
            info = self._get("/remote/info")
            report["connection"] = {
                "status": "OK",
                "url": self.base_url,
                "response_keys": list(info.keys()) if isinstance(info, dict) else str(type(info)),
            }
            # Extract route list
            routes = info.get("Routes", info.get("routes", []))
            report["routes"] = [
                {"verb": r.get("Verb", r.get("verb", "?")),
                 "path": r.get("Path", r.get("path", "?"))}
                for r in (routes[:30] if isinstance(routes, list) else [])
            ]
        except RCConnectionError as e:
            report["connection"] = {"status": "FAILED", "error": str(e)}
            return report

        # 2. GameMode object path discovery — try every candidate
        gm_results = []
        found_gm = None
        for path in self._gm_candidates():
            try:
                result = self._describe_object(path)
                if result:
                    funcs = [f.get("Name", "?") for f in result.get("Functions", [])]
                    props = [p.get("Name", "?") for p in result.get("Properties", [])]
                    entry = {
                        "path": path,
                        "status": "FOUND",
                        "functions_count": len(funcs),
                        "properties_count": len(props),
                        "sample_functions": funcs[:10],
                        "has_GetGameStateJSON": "GetGameStateJSON" in funcs,
                        "has_StartGame": "StartGame" in funcs,
                        "has_ReturnToTitle": "ReturnToTitle" in funcs,
                        "has_HandleHopCompleted": "HandleHopCompleted" in funcs,
                    }
                    gm_results.append(entry)
                    if found_gm is None and "Default__" not in path:
                        found_gm = path
                    elif found_gm is None:
                        found_gm = path
                else:
                    gm_results.append({"path": path, "status": "NOT_FOUND (describe returned None)"})
            except CallError as e:
                gm_results.append({"path": path, "status": "ERROR", "error": str(e)})
            except RCConnectionError as e:
                gm_results.append({"path": path, "status": "CONN_ERROR", "error": str(e)})

        report["gamemode_discovery"] = {
            "candidates_tested": len(gm_results),
            "selected": found_gm,
            "results": gm_results,
        }

        # 3. Frog object path discovery
        frog_results = []
        found_frog = None
        for path in self._frog_candidates():
            try:
                result = self._describe_object(path)
                if result:
                    funcs = [f.get("Name", "?") for f in result.get("Functions", [])]
                    entry = {
                        "path": path,
                        "status": "FOUND",
                        "functions_count": len(funcs),
                        "has_RequestHop": "RequestHop" in funcs,
                        "has_GridToWorld": "GridToWorld" in funcs,
                    }
                    frog_results.append(entry)
                    if found_frog is None and "Default__" not in path:
                        found_frog = path
                    elif found_frog is None:
                        found_frog = path
                else:
                    frog_results.append({"path": path, "status": "NOT_FOUND"})
            except CallError as e:
                frog_results.append({"path": path, "status": "ERROR", "error": str(e)})
            except RCConnectionError as e:
                frog_results.append({"path": path, "status": "CONN_ERROR", "error": str(e)})

        report["frog_discovery"] = {
            "candidates_tested": len(frog_results),
            "selected": found_frog,
            "results": frog_results,
        }

        # 4. Test GetGameStateJSON
        if found_gm:
            try:
                result = self._call_function(found_gm, "GetGameStateJSON")
                ret_val = result.get("ReturnValue", "")
                if ret_val:
                    try:
                        parsed = json.loads(ret_val)
                        report["get_game_state"] = {
                            "status": "OK",
                            "raw_return": ret_val,
                            "parsed": parsed,
                            "has_expected_keys": all(
                                k in parsed for k in
                                ["score", "lives", "wave", "frogPos", "gameState"]
                            ),
                        }
                    except json.JSONDecodeError as e:
                        report["get_game_state"] = {
                            "status": "JSON_PARSE_ERROR",
                            "raw_return": ret_val,
                            "error": str(e),
                        }
                else:
                    report["get_game_state"] = {
                        "status": "EMPTY_RETURN",
                        "raw_result": result,
                    }
            except CallError as e:
                report["get_game_state"] = {"status": "CALL_FAILED", "error": str(e)}

        # 5. Test RequestHop parameter format (dry probe — check if function accepts params)
        if found_frog and "Default__" not in found_frog:
            try:
                # Send a zero-vector hop — should be rejected by game logic (no direction)
                # but the RC API call itself should succeed (200 OK)
                result = self._call_function(found_frog, "RequestHop", {
                    "Direction": {"X": 0.0, "Y": 0.0, "Z": 0.0}
                })
                report["hop_test"] = {
                    "status": "OK",
                    "note": "RequestHop accepted FVector parameter format",
                    "result": result,
                }
            except CallError as e:
                report["hop_test"] = {
                    "status": "CALL_FAILED",
                    "error": str(e),
                    "note": "FVector format {X,Y,Z} may be wrong. Try nested format.",
                }
        else:
            report["hop_test"] = {
                "status": "SKIPPED",
                "note": "No live FrogCharacter found (CDO only). Cannot test hop.",
            }

        # 6. Test property reads on live objects
        prop_tests = {}
        if found_gm and "Default__" not in found_gm:
            for prop in ["CurrentState", "CurrentWave", "RemainingTime", "HomeSlotsFilledCount"]:
                try:
                    val = self._read_property(found_gm, prop)
                    prop_tests[f"GM.{prop}"] = {"status": "OK", "value": val}
                except CallError as e:
                    prop_tests[f"GM.{prop}"] = {"status": "FAILED", "error": str(e)}
        if found_frog and "Default__" not in found_frog:
            for prop in ["GridPosition", "bIsHopping", "bIsDead", "HopDuration"]:
                try:
                    val = self._read_property(found_frog, prop)
                    prop_tests[f"Frog.{prop}"] = {"status": "OK", "value": val}
                except CallError as e:
                    prop_tests[f"Frog.{prop}"] = {"status": "FAILED", "error": str(e)}
        report["property_reads"] = prop_tests

        # 7. Screenshot test (just check if the call is accepted)
        report["screenshot"] = {"status": "SKIPPED", "note": "Screenshot test requires visual verification"}

        return report

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

    @staticmethod
    def _gm_candidates():
        """Return all candidate object paths for the GameMode, ordered by likelihood."""
        candidates = []
        # In -game mode the GameMode is auto-spawned by the engine into the
        # persistent level.  The naming convention differs based on whether
        # the class is native C++ (_0 suffix) or Blueprint-derived (_C_0).
        # The AuthorityGameMode sub-object path is another common pattern.
        for map_name in [_DEFAULT_MAP, "FroggerMap", "TestMap", "DefaultMap"]:
            prefix = f"/Game/Maps/{map_name}.{map_name}:PersistentLevel"
            candidates.append(f"{prefix}.UnrealFrogGameMode_0")
            candidates.append(f"{prefix}.UnrealFrogGameMode_C_0")
            # Some UE versions use the AuthorityGameMode sub-object
            candidates.append(f"{prefix}.UnrealFrogGameMode_0.AuthorityGameMode")
        # CDO — always exists for describe/call but state reads return defaults
        candidates.append("/Script/UnrealFrog.Default__UnrealFrogGameMode")
        return candidates

    @staticmethod
    def _frog_candidates():
        """Return all candidate object paths for the FrogCharacter."""
        candidates = []
        for map_name in [_DEFAULT_MAP, "FroggerMap", "TestMap", "DefaultMap"]:
            prefix = f"/Game/Maps/{map_name}.{map_name}:PersistentLevel"
            candidates.append(f"{prefix}.FrogCharacter_0")
            candidates.append(f"{prefix}.FrogCharacter_C_0")
            # Default pawn spawned by GameMode may use a numbered suffix > 0
            candidates.append(f"{prefix}.FrogCharacter_1")
        # CDO
        candidates.append("/Script/UnrealFrog.Default__FrogCharacter")
        return candidates

    def _discover_gm_path(self):
        """Discover the GameMode object path by probing candidates."""
        for path in self._gm_candidates():
            try:
                result = self._describe_object(path)
                if result:
                    return path
            except (CallError, RCConnectionError):
                continue
        return "/Script/UnrealFrog.Default__UnrealFrogGameMode"

    def _discover_frog_path(self):
        """Discover the FrogCharacter object path by probing candidates."""
        for path in self._frog_candidates():
            try:
                result = self._describe_object(path)
                if result:
                    return path
            except (CallError, RCConnectionError):
                continue
        return "/Script/UnrealFrog.Default__FrogCharacter"

    def _verify_live_path(self, path):
        """Check if a path points to a live instance (not CDO).

        Returns True if the path responds to describe and is not a CDO path.
        """
        if "Default__" in path:
            return False
        try:
            result = self._describe_object(path)
            return result is not None
        except (CallError, RCConnectionError):
            return False

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

    # Quick path discovery
    gm = pu._get_gm_path()
    frog = pu._get_frog_path()
    print(f"GameMode path: {gm}")
    print(f"  (live instance: {'Default__' not in gm})")
    print(f"Frog path: {frog}")
    print(f"  (live instance: {'Default__' not in frog})")
    print()

    try:
        state = pu.get_state()
        print(f"Game state: {json.dumps(state, indent=2)}")
    except PlayUnrealError as e:
        print(f"Could not get state: {e}")
