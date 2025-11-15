#!/bin/bash
# Build script for 3D Platformer game

echo "Building 3D Platformer..."

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Navigate to build directory and run CMake
cd build
cmake ..
cmake --build .

echo ""
echo "Build complete! The executable is located at: build/PXX_YYYY"
echo "To run the game, execute: ./build/PXX_YYYY"
