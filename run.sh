#!/bin/bash

set -e

# From repo root: configures build/, runs AlgorithmicArena. See ./run.sh --help

USE_QUADTREE_COLLISION=ON
USE_ASTAR_PATHFINDING=ON
RUN_ARGS=()

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      cat <<'EOF'
Usage: ./run.sh [build options] [game options]

Build options (CMake — rebuilds when changed):
  --quadtree, --collision-quadtree   Quadtree collision (default)
  --brute,   --collision-brute      Brute-force collision
  --astar,   --path-astar            A* pathfinding (default)
  --dijkstra, --path-dijkstra        Dijkstra pathfinding

Game options (passed to AlgorithmicArena):
  --csv <file>          Benchmark log
  --unlimited-fps       Disable 60 FPS cap
  --map <path>          Map file
  --benchmark-mode
  --benchmark-duration-s <seconds>
  --benchmark-warmup-s <seconds>
  --benchmark-target-minions <count>
  --benchmark-order-interval-s <seconds>
  --benchmark-seed <uint>

Examples:
  ./run.sh --brute --dijkstra
  ./run.sh --quadtree --astar --csv ../results/run1.csv
  ./run.sh --quadtree --astar --map ../maps/benchmark_open_128.map \
    --csv ../benchmark_runs/auto.csv --benchmark-mode \
    --benchmark-duration-s 60 --benchmark-warmup-s 7 \
    --benchmark-target-minions 500 --benchmark-order-interval-s 1.5
EOF
      exit 0
      ;;
    --collision-brute|--brute)
      USE_QUADTREE_COLLISION=OFF
      shift
      ;;
    --collision-quadtree|--quadtree)
      USE_QUADTREE_COLLISION=ON
      shift
      ;;
    --path-dijkstra|--dijkstra)
      USE_ASTAR_PATHFINDING=OFF
      shift
      ;;
    --path-astar|--astar)
      USE_ASTAR_PATHFINDING=ON
      shift
      ;;
    *)
      RUN_ARGS+=("$1")
      shift
      ;;
  esac
done

if [[ ! -d build ]]; then
  mkdir build
fi

cd build

echo "--- Configuring (USE_QUADTREE_COLLISION=${USE_QUADTREE_COLLISION} USE_ASTAR_PATHFINDING=${USE_ASTAR_PATHFINDING}) ---"
cmake .. -DCMAKE_BUILD_TYPE=Release \
  -DUSE_QUADTREE_COLLISION="${USE_QUADTREE_COLLISION}" \
  -DUSE_ASTAR_PATHFINDING="${USE_ASTAR_PATHFINDING}"

echo "--- Building ---"
cmake --build . -j"$(nproc)"

echo "--- Running Game ---"
exec ./AlgorithmicArena "${RUN_ARGS[@]}"
