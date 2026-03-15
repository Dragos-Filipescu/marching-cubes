#!/bin/bash
PRESET=${1:-relwithdebinfo}

# Initialize submodules if not present
if [ ! -f "external/glfw/CMakeLists.txt" ] || [ ! -f "external/glm/CMakeLists.txt" ]; then
    echo "Initializing submodules..."
    git submodule update --init --recursive
fi

cmake --preset $PRESET
cmake --build build/$PRESET
