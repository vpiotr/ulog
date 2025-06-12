#!/bin/bash

# run_tests.sh - Script to execute all ulog tests

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== ulog Test Execution Script ==="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run ./rebuild.sh first."
    exit 1
fi

# List of test executables
TEST_EXECUTABLES=(
    "test_logger"
    "test_buffer"
    "test_formatter"
    "test_observer"
)

echo "Running individual test modules..."
echo ""

TOTAL_TESTS=0
PASSED_TESTS=0
FAILED_TESTS=0

# Run each test executable
for TEST_NAME in "${TEST_EXECUTABLES[@]}"; do
    TEST_EXECUTABLE="$BUILD_DIR/$TEST_NAME"
    
    if [ ! -f "$TEST_EXECUTABLE" ]; then
        echo "Error: Test executable not found: $TEST_EXECUTABLE"
        echo "Please run ./rebuild.sh first."
        exit 1
    fi
    
    echo "Running $TEST_NAME..."
    cd "$PROJECT_ROOT"
    
    # Run test and capture exit code
    if "$TEST_EXECUTABLE"; then
        echo "✓ $TEST_NAME PASSED"
        ((PASSED_TESTS++))
    else
        echo "✗ $TEST_NAME FAILED"
        ((FAILED_TESTS++))
    fi
    
    ((TOTAL_TESTS++))
    echo ""
done

echo "=== Test Summary ==="
echo "Total test modules: $TOTAL_TESTS"
echo "Passed: $PASSED_TESTS"
echo "Failed: $FAILED_TESTS"

if [ $FAILED_TESTS -eq 0 ]; then
    echo "=== All tests PASSED! ==="
    exit 0
else
    echo "=== Some tests FAILED! ==="
    exit 1
fi
