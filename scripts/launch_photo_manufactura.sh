#!/bin/bash
# Photo Manufactura Launcher Script

# Get the directory where this script is located
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(dirname "$SCRIPT_DIR")"

# Set the path to the executable
EXECUTABLE="$PROJECT_ROOT/build/bin/photo_manufactura"

# Check if the executable exists
if [ ! -f "$EXECUTABLE" ]; then
    echo "Error: Photo Manufactura executable not found at: $EXECUTABLE"
    echo "Please build the project first by running:"
    echo "  cmake -B build -S . -G Ninja"
    echo "  cmake --build build"
    exit 1
fi

# Copy AI models to the build directory if not already there
AI_MODELS_SRC="$PROJECT_ROOT/AI_models"
AI_MODELS_DEST="$PROJECT_ROOT/build/bin/AI_models"

if [ -d "$AI_MODELS_SRC" ] && [ ! -d "$AI_MODELS_DEST" ]; then
    echo "Copying AI models to build directory..."
    cp -r "$AI_MODELS_SRC" "$AI_MODELS_DEST"
fi

# Launch the application
echo "Starting Photo Manufactura..."
cd "$PROJECT_ROOT"
"$EXECUTABLE" "$@"
