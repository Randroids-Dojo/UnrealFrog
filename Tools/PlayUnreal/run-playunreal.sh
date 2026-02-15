#!/usr/bin/env bash
# run-playunreal.sh — Launch game with Remote Control API and run a Python test script
#
# Usage:
#   ./Tools/PlayUnreal/run-playunreal.sh acceptance_test.py   # Run specific test
#   ./Tools/PlayUnreal/run-playunreal.sh                      # Run acceptance test (default)
#
# Exit codes:
#   0 = test passed
#   1 = test failed
#   2 = launch failure or timeout waiting for Remote Control API
#
# Prerequisites:
#   - UE 5.7 installed at /Users/Shared/Epic Games/UE_5.7/
#   - UnrealFrogEditor target built with RemoteControl plugin enabled
#   - Python 3 available

set -euo pipefail

# -- Configuration -----------------------------------------------------------

ENGINE_DIR="/Users/Shared/Epic Games/UE_5.7"
EDITOR_APP="${ENGINE_DIR}/Engine/Binaries/Mac/UnrealEditor.app/Contents/MacOS/UnrealEditor"
PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROJECT_FILE="${PROJECT_ROOT}/UnrealFrog.uproject"
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"

RC_PORT=30010
RC_URL="http://localhost:${RC_PORT}/remote/info"
STARTUP_TIMEOUT=120  # seconds to wait for editor + RC API
SCRIPT_NAME="${1:-acceptance_test.py}"
SCRIPT_PATH="${SCRIPT_DIR}/${SCRIPT_NAME}"

# -- Validation --------------------------------------------------------------

if [ ! -f "${EDITOR_APP}" ]; then
    echo "ERROR: UnrealEditor not found at: ${EDITOR_APP}"
    exit 2
fi

if [ ! -f "${PROJECT_FILE}" ]; then
    echo "ERROR: Project file not found at: ${PROJECT_FILE}"
    exit 2
fi

if [ ! -f "${SCRIPT_PATH}" ]; then
    echo "ERROR: Test script not found at: ${SCRIPT_PATH}"
    exit 2
fi

# -- Kill stale editor processes ---------------------------------------------

STALE_PIDS=$(pgrep -f "UnrealEditor|UnrealTraceServer" 2>/dev/null || true)
if [ -n "${STALE_PIDS}" ]; then
    echo "Killing stale editor processes..."
    pkill -f "UnrealTraceServer" 2>/dev/null || true
    pkill -f "UnrealEditor" 2>/dev/null || true
    sleep 3
    echo "Done."
fi

# -- Launch editor with Remote Control API -----------------------------------

echo "============================================"
echo "  PlayUnreal Test Runner"
echo "============================================"
echo "  Engine:  ${ENGINE_DIR}"
echo "  Project: ${PROJECT_FILE}"
echo "  Script:  ${SCRIPT_NAME}"
echo "  RC Port: ${RC_PORT}"
echo "  Timeout: ${STARTUP_TIMEOUT}s"
echo "============================================"
echo ""

echo "Launching editor in -game mode with Remote Control API..."

"${EDITOR_APP}" \
    "${PROJECT_FILE}" \
    -game \
    -windowed \
    -resx=1280 \
    -resy=720 \
    -log \
    -RCWebControlEnable \
    -ExecCmds="WebControl.EnableServerOnStartup 1" \
    &
EDITOR_PID=$!

# Cleanup on exit — always kill the editor
cleanup() {
    if kill -0 "${EDITOR_PID}" 2>/dev/null; then
        echo ""
        echo "Shutting down editor (PID: ${EDITOR_PID})..."
        kill "${EDITOR_PID}" 2>/dev/null || true
        sleep 2
        kill -9 "${EDITOR_PID}" 2>/dev/null || true
    fi
}
trap cleanup EXIT INT TERM

# -- Wait for Remote Control API to respond ----------------------------------

echo "Waiting for Remote Control API on port ${RC_PORT}..."
echo ""

ELAPSED=0
RC_READY=false
while [ "${ELAPSED}" -lt "${STARTUP_TIMEOUT}" ]; do
    if curl -s --connect-timeout 2 "${RC_URL}" > /dev/null 2>&1; then
        RC_READY=true
        break
    fi

    # Check if editor is still running
    if ! kill -0 "${EDITOR_PID}" 2>/dev/null; then
        echo "ERROR: Editor process exited before Remote Control API was ready."
        exit 2
    fi

    sleep 2
    ELAPSED=$((ELAPSED + 2))
    if [ $((ELAPSED % 10)) -eq 0 ]; then
        echo "  Still waiting... (${ELAPSED}s / ${STARTUP_TIMEOUT}s)"
    fi
done

if [ "${RC_READY}" = false ]; then
    echo "ERROR: Remote Control API did not respond within ${STARTUP_TIMEOUT}s."
    echo "       The editor may have failed to start or the RC plugin is not loading."
    exit 2
fi

echo "Remote Control API is ready (took ~${ELAPSED}s)."
echo ""

# -- Run the Python test script ----------------------------------------------

echo "Running: python3 ${SCRIPT_PATH}"
echo "============================================"
echo ""

SCRIPT_EXIT=0
python3 "${SCRIPT_PATH}" || SCRIPT_EXIT=$?

echo ""
echo "============================================"

if [ "${SCRIPT_EXIT}" -eq 0 ]; then
    echo "RESULT: PASS"
else
    echo "RESULT: FAIL (exit code: ${SCRIPT_EXIT})"
fi

exit "${SCRIPT_EXIT}"
