# Benchmark Mode (Automated Runs)

This document explains how to run Algorithmic Arena in fully automated benchmark mode to reduce human error and improve repeatability.

## What benchmark mode does

When `--benchmark-mode` is enabled, the game:

1. Starts the match automatically (no manual Enter key).
2. Spawns minions automatically until a target count is reached.
3. Issues periodic automated orders to keep pathfinding/collision load active.
4. Logs benchmark stats to CSV (when `--csv` is provided).
5. Automatically restarts the match if it enters win/loss/non-playing state.
6. Stops automatically after `warmup + duration` seconds.

This is designed for thesis-quality repeatable performance runs.

## CLI options

Use these game arguments (can be passed via `./run.sh`):

- `--benchmark-mode`
- `--benchmark-duration-s <seconds>` measured window length after warmup (default `60`)
- `--benchmark-warmup-s <seconds>` warmup time before measured window (default `7`)
- `--benchmark-target-minions <count>` maintained load target (default `250`)
- `--benchmark-order-interval-s <seconds>` auto-order cadence (default `1.5`)
- `--benchmark-seed <uint>` RNG seed for reproducible spawn distribution (default `1337`)

Existing options still apply:

- `--csv <file>`
- `--map <path>`
- `--unlimited-fps`

## Recommended command templates

From repository root:

### Collision run (Quadtree + A*)

```bash
./run.sh --quadtree --astar \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_quadtree_open128_500_run1.csv \
  --benchmark-mode \
  --benchmark-duration-s 60 \
  --benchmark-warmup-s 7 \
  --benchmark-target-minions 500 \
  --benchmark-order-interval-s 1.5 \
  --benchmark-seed 1337
```

### Collision run (Brute-Force + A*)

```bash
./run.sh --brute --astar \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_brute_open128_500_run1.csv \
  --benchmark-mode \
  --benchmark-duration-s 60 \
  --benchmark-warmup-s 7 \
  --benchmark-target-minions 500 \
  --benchmark-order-interval-s 1.5 \
  --benchmark-seed 1337
```

### Pathfinding run (Quadtree + Dijkstra)

```bash
./run.sh --quadtree --dijkstra \
  --map ../maps/benchmark_maze_128.map \
  --csv ../benchmark_runs/pathfinding_dijkstra_maze128_500_run1.csv \
  --benchmark-mode \
  --benchmark-duration-s 60 \
  --benchmark-warmup-s 7 \
  --benchmark-target-minions 500 \
  --benchmark-order-interval-s 1.5 \
  --benchmark-seed 1337
```

## Fair-comparison rules

For each pair, keep these identical:

- Map
- Target minion count
- Warmup and measured duration
- Order interval
- Seed
- FPS mode (`--unlimited-fps` usage)
- Machine and power profile

Only change the algorithm under test.

## Notes

- In benchmark mode, debug overlays are disabled by default to avoid skewing timing.
- If gameplay exits `PLAYING` (for example, all flags captured), benchmark mode immediately restarts the match and continues until the benchmark timer ends.
- If all flags are captured while still in `PLAYING`, benchmark mode treats that as a completed cycle and immediately restarts the match.
- CSV rows remain 1 Hz snapshots of per-second aggregate work:
  - `collision_us_sum_1s`
  - `pathfinding_us_sum_1s`
  - `path_calls_sum_1s`

## Post-run analysis

Use existing analysis scripts:

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/your_run.csv
python3 scripts/analyze_benchmark_csv.py compare run_a.csv run_b.csv --labels A B --focus collision
```

