#!/bin/bash
PRESET=${1:-relwithdebinfo}
EXECUTABLE="build/${PRESET}/marching-cubes"

if [ ! -f "$EXECUTABLE" ]; then
    echo "Executable not found: $EXECUTABLE"
    echo "Run ./build.sh ${PRESET} first"
    exit 1
fi

(cd "build/${PRESET}" && ./marching-cubes)
