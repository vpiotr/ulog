#!/bin/bash

# build_docs.sh - Script to rebuild Doxygen documentation

set -e  # Exit on any error

# Script directory
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$SCRIPT_DIR"
DOCS_DIR="$PROJECT_ROOT/docs"
DOCS_BUILD_DIR="$DOCS_DIR/html"

echo "=== ulog Documentation Build Script ==="

# Check if Doxygen is installed
if ! command -v doxygen &> /dev/null; then
    echo "Error: Doxygen is not installed. Please install it first:"
    echo "  Ubuntu/Debian: sudo apt-get install doxygen"
    echo "  CentOS/RHEL:   sudo yum install doxygen"
    echo "  Arch Linux:    sudo pacman -S doxygen"
    exit 1
fi

# Create docs directory if it doesn't exist
mkdir -p "$DOCS_DIR"

# Generate Doxygen configuration file if it doesn't exist
DOXYFILE="$DOCS_DIR/Doxyfile"
if [ ! -f "$DOXYFILE" ]; then
    echo "Creating Doxygen configuration file..."
    cd "$DOCS_DIR"
    doxygen -g Doxyfile
    
    # Configure Doxygen settings
    sed -i "s/PROJECT_NAME           = \"My Project\"/PROJECT_NAME           = \"ulog\"/" Doxyfile
    sed -i "s/PROJECT_NUMBER         =/PROJECT_NUMBER         = 1.0.0/" Doxyfile
    sed -i "s/PROJECT_BRIEF          =/PROJECT_BRIEF          = \"Lightweight C++ Logging Library\"/" Doxyfile
    sed -i "s|INPUT                  =|INPUT                  = $PROJECT_ROOT/include/ulog|" Doxyfile
    sed -i "s/RECURSIVE              = NO/RECURSIVE              = YES/" Doxyfile
    sed -i "s/EXTRACT_ALL            = NO/EXTRACT_ALL            = YES/" Doxyfile
    sed -i "s/EXTRACT_PRIVATE        = NO/EXTRACT_PRIVATE        = YES/" Doxyfile
    sed -i "s/EXTRACT_STATIC         = NO/EXTRACT_STATIC         = YES/" Doxyfile
    sed -i "s/GENERATE_LATEX         = YES/GENERATE_LATEX         = NO/" Doxyfile
    sed -i "s/HAVE_DOT               = NO/HAVE_DOT               = YES/" Doxyfile
    sed -i "s/UML_LOOK               = NO/UML_LOOK               = YES/" Doxyfile
    sed -i "s/CALL_GRAPH             = NO/CALL_GRAPH             = YES/" Doxyfile
    sed -i "s/CALLER_GRAPH           = NO/CALLER_GRAPH           = YES/" Doxyfile
    
    echo "Doxygen configuration file created: $DOXYFILE"
fi

# Clean previous documentation
echo "Cleaning previous documentation..."
rm -rf "$DOCS_BUILD_DIR"

# Generate documentation
echo "Generating documentation..."
cd "$DOCS_DIR"
doxygen Doxyfile

# Check if documentation was generated successfully
if [ -f "$DOCS_BUILD_DIR/index.html" ]; then
    echo "=== Documentation generated successfully! ==="
    echo "Documentation location: $DOCS_BUILD_DIR"
    echo "Open in browser: file://$DOCS_BUILD_DIR/index.html"
    
    # Try to open documentation in default browser if available
    if command -v xdg-open &> /dev/null; then
        echo "Opening documentation in default browser..."
        xdg-open "file://$DOCS_BUILD_DIR/index.html" &
    fi
else
    echo "=== Documentation generation failed! ==="
    exit 1
fi
