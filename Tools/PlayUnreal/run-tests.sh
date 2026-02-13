#!/usr/bin/env bash
# run-tests.sh — Headless test runner for UnrealFrog automation tests
#
# Usage:
#   ./Tools/PlayUnreal/run-tests.sh                  # Run all UnrealFrog tests
#   ./Tools/PlayUnreal/run-tests.sh "UnrealFrog.Unit" # Run specific test group
#   ./Tools/PlayUnreal/run-tests.sh --list            # List available tests
#
# Requirements:
#   - UE 5.7 installed at /Users/Shared/Epic Games/UE_5.7/
#   - UnrealFrogEditor target built (Editor build)

set -euo pipefail

# -- Configuration -----------------------------------------------------------

ENGINE_DIR="/Users/Shared/Epic Games/UE_5.7"
EDITOR_CMD="${ENGINE_DIR}/Engine/Binaries/Mac/UnrealEditor-Cmd"
PROJECT_ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
PROJECT_FILE="${PROJECT_ROOT}/UnrealFrog.uproject"
LOG_DIR="${PROJECT_ROOT}/Saved/Logs"
TEST_LOG="${LOG_DIR}/TestRunner.log"

# Default test filter: all UnrealFrog tests
TEST_FILTER="${1:-UnrealFrog}"

# -- Validation --------------------------------------------------------------

if [ ! -f "${EDITOR_CMD}" ]; then
    echo "ERROR: UnrealEditor-Cmd not found at: ${EDITOR_CMD}"
    echo "       Is UE 5.7 installed?"
    exit 1
fi

if [ ! -f "${PROJECT_FILE}" ]; then
    echo "ERROR: Project file not found at: ${PROJECT_FILE}"
    exit 1
fi

# -- List mode ---------------------------------------------------------------

if [ "${TEST_FILTER}" = "--list" ]; then
    echo "Listing available UnrealFrog automation tests..."
    "${EDITOR_CMD}" \
        "${PROJECT_FILE}" \
        -NullRHI \
        -NoSound \
        -NoSplash \
        -Unattended \
        -ExecCmds="Automation List UnrealFrog; Quit" \
        -Log="${TEST_LOG}" \
        2>&1 | grep -E "UnrealFrog\." || true

    echo ""
    echo "Full log: ${TEST_LOG}"
    exit 0
fi

# -- Run tests ---------------------------------------------------------------

echo "============================================"
echo "  UnrealFrog Headless Test Runner"
echo "============================================"
echo "  Engine:  ${ENGINE_DIR}"
echo "  Project: ${PROJECT_FILE}"
echo "  Filter:  ${TEST_FILTER}"
echo "  Log:     ${TEST_LOG}"
echo "============================================"
echo ""

mkdir -p "${LOG_DIR}"

echo "Running tests..."
echo ""

# Launch editor in headless mode, run automation tests, then quit
"${EDITOR_CMD}" \
    "${PROJECT_FILE}" \
    -NullRHI \
    -NoSound \
    -NoSplash \
    -Unattended \
    -NoPause \
    -ExecCmds="Automation RunTests ${TEST_FILTER}; Quit" \
    -Log="${TEST_LOG}" \
    2>&1 | tee /dev/stderr | {
        # Capture output for exit code determination
        TEST_OUTPUT=$(cat)
    } || true

# -- Parse results -----------------------------------------------------------

echo ""
echo "============================================"
echo "  Test Results"
echo "============================================"

PASSED=0
FAILED=0
ERRORS=""

if [ -f "${TEST_LOG}" ]; then
    # Count passed/failed from log
    PASSED=$(grep -c "Test Completed\. Result={Success}" "${TEST_LOG}" 2>/dev/null || echo "0")
    FAILED=$(grep -c "Test Completed\. Result={Fail}" "${TEST_LOG}" 2>/dev/null || echo "0")

    # Extract failure details
    ERRORS=$(grep -A2 "Result={Fail}" "${TEST_LOG}" 2>/dev/null || true)
fi

TOTAL=$((PASSED + FAILED))

echo "  Total:  ${TOTAL}"
echo "  Passed: ${PASSED}"
echo "  Failed: ${FAILED}"
echo "============================================"

if [ -n "${ERRORS}" ]; then
    echo ""
    echo "FAILURES:"
    echo "${ERRORS}"
    echo ""
fi

echo "Full log: ${TEST_LOG}"

# Exit with failure if any tests failed
if [ "${FAILED}" -gt 0 ]; then
    echo ""
    echo "RESULT: FAIL"
    exit 1
fi

if [ "${TOTAL}" -eq 0 ]; then
    echo ""
    echo "WARNING: No tests matched filter '${TEST_FILTER}'"
    echo "         Try: $0 --list"
    exit 2
fi

echo ""
echo "RESULT: PASS"
exit 0
