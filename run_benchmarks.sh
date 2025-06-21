#!/bin/bash

# run_benchmarks.sh - Script to execute all ulog benchmarks

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== ulog Benchmark Execution Script ==="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run ./rebuild.sh first."
    exit 1
fi

# List of benchmark executables
BENCHMARK_EXECUTABLES=(
    "demo_buffer_benchmark_with_mutex"
    "demo_buffer_benchmark_no_mutex"
    "demo_observer_benchmark_with_mutex"
    "demo_observer_benchmark_no_mutex"
)

# Check if all benchmark executables exist
MISSING_EXECUTABLES=()
for executable in "${BENCHMARK_EXECUTABLES[@]}"; do
    if [ ! -f "$BUILD_DIR/$executable" ]; then
        MISSING_EXECUTABLES+=("$executable")
    fi
done

if [ ${#MISSING_EXECUTABLES[@]} -ne 0 ]; then
    echo "Error: The following benchmark executables are missing:"
    for executable in "${MISSING_EXECUTABLES[@]}"; do
        echo "  - $executable"
    done
    echo ""
    echo "Please build the project first with: ./rebuild.sh"
    exit 1
fi

echo "Found all benchmark executables."
echo ""

# Function to run a benchmark with separator
run_benchmark() {
    local executable=$1
    local description=$2
    
    echo "========================================"
    echo "Running: $executable"
    echo "Description: $description"
    echo "========================================"
    echo ""
    
    cd "$BUILD_DIR" || exit 1
    
    if ./"$executable"; then
        echo ""
        echo "SUCCESS: $executable completed successfully"
    else
        echo ""
        echo "FAILED: $executable failed with exit code $?"
        return 1
    fi
    
    echo ""
    echo "----------------------------------------"
    echo ""
}

# Run all benchmarks
FAILED_BENCHMARKS=()

echo "Starting benchmark execution..."
echo ""

run_benchmark "demo_buffer_benchmark_with_mutex" "Buffer performance with mutex protection (single-threaded)" || FAILED_BENCHMARKS+=("demo_buffer_benchmark_with_mutex")

run_benchmark "demo_buffer_benchmark_no_mutex" "Buffer performance without mutex protection (single-threaded)" || FAILED_BENCHMARKS+=("demo_buffer_benchmark_no_mutex")

run_benchmark "demo_observer_benchmark_with_mutex" "Observer performance with mutex protection (single-threaded)" || FAILED_BENCHMARKS+=("demo_observer_benchmark_with_mutex")

run_benchmark "demo_observer_benchmark_no_mutex" "Observer performance without mutex protection (single-threaded)" || FAILED_BENCHMARKS+=("demo_observer_benchmark_no_mutex")

# Summary
echo "========================================"
echo "Benchmark Summary"
echo "========================================"
echo ""
echo "Total benchmarks: ${#BENCHMARK_EXECUTABLES[@]}"
echo "Successful: $((${#BENCHMARK_EXECUTABLES[@]} - ${#FAILED_BENCHMARKS[@]}))"
echo "Failed: ${#FAILED_BENCHMARKS[@]}"

if [ ${#FAILED_BENCHMARKS[@]} -eq 0 ]; then
    echo ""
    echo "=== All benchmarks PASSED! ==="
    exit 0
else
    echo ""
    echo "=== Some benchmarks FAILED! ==="
    echo "Failed benchmarks:"
    for benchmark in "${FAILED_BENCHMARKS[@]}"; do
        echo "  - $benchmark"
    done
    exit 1
fi
