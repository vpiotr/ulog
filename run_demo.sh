#!/bin/bash

# run_demo.sh - Script to execute all ulog demos

set -e  # Exit on any error

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
DEMO_EXECUTABLE="$BUILD_DIR/ulog_demo"

echo "=== ulog Demo Execution Script ==="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run ./rebuild.sh first."
    exit 1
fi

# Check if demo executable exists
if [ ! -f "$DEMO_EXECUTABLE" ]; then
    echo "Error: Demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

echo "Running demo..."
echo "Demo executable: $DEMO_EXECUTABLE"
echo ""

# Run demo
cd "$PROJECT_ROOT"
"$DEMO_EXECUTABLE"

DEMO_RESULT=$?

echo ""
if [ $DEMO_RESULT -eq 0 ]; then
    echo "=== Demo completed successfully! ==="
else
    echo "=== Demo failed! ==="
    echo "Exit code: $DEMO_RESULT"
fi

exit $DEMO_RESULT
