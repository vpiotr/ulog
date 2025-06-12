#!/bin/bash

# run_demos.sh - Script to execute all ulog demos

set -e  # Exit on any error

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
BUILD_DIR="$PROJECT_ROOT/build"
MAIN_DEMO_EXECUTABLE="$BUILD_DIR/ulog_demo"
FILE_OBSERVER_DEMO_EXECUTABLE="$BUILD_DIR/demo_file_observer"
LOG_LEVEL_FILTERING_DEMO_EXECUTABLE="$BUILD_DIR/demo_log_level_filtering"
CUSTOM_FORMATTING_DEMO_EXECUTABLE="$BUILD_DIR/demo_custom_formatting"
CERR_OBSERVER_DEMO_EXECUTABLE="$BUILD_DIR/demo_cerr_observer"
EXCEPTION_FORMATTING_DEMO_EXECUTABLE="$BUILD_DIR/demo_exception_formatting"
CONTAINER_FORMATTING_DEMO_EXECUTABLE="$BUILD_DIR/demo_container_formatting"

echo "=== ulog Demos Execution Script ==="

# Check if build directory exists
if [ ! -d "$BUILD_DIR" ]; then
    echo "Error: Build directory not found. Please run ./rebuild.sh first."
    exit 1
fi

# Check if demo executables exist
if [ ! -f "$MAIN_DEMO_EXECUTABLE" ]; then
    echo "Error: Main demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$FILE_OBSERVER_DEMO_EXECUTABLE" ]; then
    echo "Error: File observer demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$LOG_LEVEL_FILTERING_DEMO_EXECUTABLE" ]; then
    echo "Error: Log level filtering demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$CUSTOM_FORMATTING_DEMO_EXECUTABLE" ]; then
    echo "Error: Custom formatting demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$CERR_OBSERVER_DEMO_EXECUTABLE" ]; then
    echo "Error: Cerr observer demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$EXCEPTION_FORMATTING_DEMO_EXECUTABLE" ]; then
    echo "Error: Exception formatting demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

if [ ! -f "$CONTAINER_FORMATTING_DEMO_EXECUTABLE" ]; then
    echo "Error: Container formatting demo executable not found. Please run ./rebuild.sh first."
    exit 1
fi

echo "Running demos..."
echo "Main demo executable: $MAIN_DEMO_EXECUTABLE"
echo "File observer demo executable: $FILE_OBSERVER_DEMO_EXECUTABLE"
echo "Log level filtering demo executable: $LOG_LEVEL_FILTERING_DEMO_EXECUTABLE"
echo "Custom formatting demo executable: $CUSTOM_FORMATTING_DEMO_EXECUTABLE"
echo "Cerr observer demo executable: $CERR_OBSERVER_DEMO_EXECUTABLE"
echo "Exception formatting demo executable: $EXCEPTION_FORMATTING_DEMO_EXECUTABLE"
echo "Container formatting demo executable: $CONTAINER_FORMATTING_DEMO_EXECUTABLE"
echo ""

# Run main demo
echo "=== Running Main Demo ==="
cd "$PROJECT_ROOT"
"$MAIN_DEMO_EXECUTABLE"

MAIN_DEMO_RESULT=$?

echo ""
echo "=== Running File Observer Demo ==="
"$FILE_OBSERVER_DEMO_EXECUTABLE"

FILE_OBSERVER_DEMO_RESULT=$?

echo ""
echo "=== Running Log Level Filtering Demo ==="
"$LOG_LEVEL_FILTERING_DEMO_EXECUTABLE"

LOG_LEVEL_FILTERING_DEMO_RESULT=$?

echo ""
echo "=== Running Custom Formatting Demo ==="
"$CUSTOM_FORMATTING_DEMO_EXECUTABLE"

CUSTOM_FORMATTING_DEMO_RESULT=$?

echo ""
echo "=== Running Cerr Observer Demo ==="
"$CERR_OBSERVER_DEMO_EXECUTABLE"

CERR_OBSERVER_DEMO_RESULT=$?

echo ""
echo "=== Running Exception Formatting Demo ==="
"$EXCEPTION_FORMATTING_DEMO_EXECUTABLE"

EXCEPTION_FORMATTING_DEMO_RESULT=$?

echo ""
echo "=== Running Container Formatting Demo ==="
"$CONTAINER_FORMATTING_DEMO_EXECUTABLE"

CONTAINER_FORMATTING_DEMO_RESULT=$?

echo ""
if [ $MAIN_DEMO_RESULT -eq 0 ] && [ $FILE_OBSERVER_DEMO_RESULT -eq 0 ] && [ $LOG_LEVEL_FILTERING_DEMO_RESULT -eq 0 ] && [ $CUSTOM_FORMATTING_DEMO_RESULT -eq 0 ] && [ $CERR_OBSERVER_DEMO_RESULT -eq 0 ] && [ $EXCEPTION_FORMATTING_DEMO_RESULT -eq 0 ] && [ $CONTAINER_FORMATTING_DEMO_RESULT -eq 0 ]; then
    echo "=== All demos completed successfully! ==="
    echo ""
    echo "Generated log files from file observer demo:"
    if [ -f "demo_basic.log" ]; then
        echo "1. demo_basic.log"
    fi
    if [ -f "demo_raii.log" ]; then
        echo "2. demo_raii.log"
    fi
    if [ -f "demo_general.log" ]; then
        echo "3. demo_general.log"
    fi
    if [ -f "demo_errors.log" ]; then
        echo "4. demo_errors.log"
    fi
    if [ -f "demo_errors_only.log" ]; then
        echo "5. demo_errors_only.log"
    fi
    echo ""
    echo "You can examine these files to see the file output functionality in action."
else
    echo "=== One or more demos failed! ==="
    echo "Main demo exit code: $MAIN_DEMO_RESULT"
    echo "File observer demo exit code: $FILE_OBSERVER_DEMO_RESULT"
    echo "Log level filtering demo exit code: $LOG_LEVEL_FILTERING_DEMO_RESULT"
    echo "Custom formatting demo exit code: $CUSTOM_FORMATTING_DEMO_RESULT"
    echo "Cerr observer demo exit code: $CERR_OBSERVER_DEMO_RESULT"
    echo "Exception formatting demo exit code: $EXCEPTION_FORMATTING_DEMO_RESULT"
    exit 1
fi
