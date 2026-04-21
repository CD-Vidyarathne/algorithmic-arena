#!/usr/bin/env bash
set -euo pipefail

mkdir -p benchmark_runs benchmark_results/repro

WARMUP="${WARMUP:-7}"
DURATION="${DURATION:-60}"
ORDER_INTERVAL="${ORDER_INTERVAL:-1.5}"
SEED="${SEED:-1337}"
REPEATS=(${REPEATS:-1 2 3})
LEVELS=(${LEVELS:-50 100 250 500})
# FPS_MODE="${FPS_MODE:---unlimited-fps}"
FPS_MODE=""

LOG="benchmark_results/repro/commands_pathfinding_$(date +%Y%m%d_%H%M%S).log"
echo "# Pathfinding benchmark command log" | tee -a "$LOG"
date | tee -a "$LOG"
git rev-parse HEAD | tee -a "$LOG"
echo "WARMUP=$WARMUP DURATION=$DURATION ORDER_INTERVAL=$ORDER_INTERVAL SEED=$SEED FPS_MODE=$FPS_MODE" | tee -a "$LOG"
echo "REPEATS=${REPEATS[*]} LEVELS=${LEVELS[*]}" | tee -a "$LOG"

for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --quadtree --astar ${FPS_MODE} \
      --map ../maps/benchmark_maze_128.map \
      --csv ../benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done

for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --quadtree --dijkstra ${FPS_MODE} \
      --map ../maps/benchmark_maze_128.map \
      --csv ../benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done

echo "Pathfinding matrix complete." | tee -a "$LOG"
