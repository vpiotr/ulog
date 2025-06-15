#!/bin/bash

# rebuild.sh - Script to rebuild the entire ulog project (tests, demos)

set -e  # Exit on any error

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== ulog Project Rebuild Script ==="
echo "Project root: $PROJECT_ROOT"
echo "Build directory: $BUILD_DIR"

# Clean and create build directory
echo "Cleaning build directory..."
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

# Configure project
echo "Configuring project with CMake..."
cd "$BUILD_DIR"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build project
echo "Building project..."
make -j$(nproc)

echo "=== Build completed successfully! ==="
echo ""
echo "Available executables:"
echo "  - Tests: $BUILD_DIR/test_logger, $BUILD_DIR/test_buffer, $BUILD_DIR/test_formatter, $BUILD_DIR/test_observer"
echo "  - Demo:  $BUILD_DIR/ulog_demo"
echo ""
echo "To run tests: ./run_tests.sh"
echo "To run demo:  ./run_demos.sh"
echo "To run benchmarks: ./run_benchmarks.sh"
