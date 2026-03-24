#!/bin/bash

set -e

# Build from repo root, then run the game from build/ (so default ../maps/... paths work).
# Examples:
#   ./run.sh
#   ./run.sh --csv /tmp/bench.csv
#   ./run.sh --unlimited-fps --csv ../out/run1.csv
#   ./run.sh --map ../maps/benchmark_open_512.map --unlimited-fps --csv ../out/open.csv

if [ ! -d "build" ]; then
  mkdir build
fi

cd build

echo "--- Configuring ---"
cmake .. -DCMAKE_BUILD_TYPE=Release

echo "--- Building ---"
cmake --build . -j$(nproc)

echo "--- Running Game ---"
exec ./AlgorithmicArena "$@"
