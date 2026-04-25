# Algorithmic Arena

Custom C++ game engine prototype built for BSc Computer Science final year research, focused on algorithmic performance comparison in real gameplay scenarios.

## Overview

Algorithmic Arena benchmarks two key subsystems:

- **Collision:** Quadtree vs Brute-Force
- **Pathfinding:** A* vs Dijkstra

The project includes:

- Playable game loop
- Automated benchmark mode
- CSV logging for metrics
- Python analysis tooling for pairwise and matrix comparisons

---

## Tech Stack

- **Language:** C++20
- **Build:** CMake
- **Graphics/Input:** SFML
- **Logging:** spdlog
- **Analysis:** Python 3 (standard library only)

---

## Repository Structure

- `src/` - core game and algorithm implementations
- `maps/` - benchmark and gameplay maps
- `assets/` - textures/fonts
- `scripts/` - benchmark runners and CSV analysis scripts
- `docs/` - benchmark methodology and marking references
- `run.sh` - main configure/build/run entry point

---

## Prerequisites

### Arch Linux

```bash
sudo pacman -S --needed base-devel cmake pkgconf sfml spdlog python
```

### Any Linux (generic)

Install equivalents for:

- C++ compiler (`g++`/`clang++`)
- `cmake`
- `pkg-config`/`pkgconf`
- SFML development libraries
- spdlog development libraries
- Python 3

---

## Quick Start

1. Clone and enter repository:
   ```bash
   git clone <your-repo-url> algorithmic-arena
   cd algorithmic-arena
   ```

2. Make scripts executable:
   ```bash
   chmod +x run.sh scripts/*.sh
   ```

3. Run default game build (**Quadtree + A***):
   ```bash
   ./run.sh
   ```

---

## Running the Game

`run.sh` handles configure + build + execute.

### Default (Quadtree + A*)

```bash
./run.sh
```

### Choose collision/pathfinding variants

```bash
./run.sh --quadtree --astar
./run.sh --brute --astar
./run.sh --quadtree --dijkstra
./run.sh --brute --dijkstra
```

### Useful runtime flags

- `--map <path>`
- `--csv <file>`
- `--unlimited-fps`
- `--benchmark-mode`
- `--benchmark-duration-s <seconds>`
- `--benchmark-warmup-s <seconds>`
- `--benchmark-target-minions <count>`
- `--benchmark-order-interval-s <seconds>`
- `--benchmark-seed <uint>`

Full help:

```bash
./run.sh --help
```

---

## Running Benchmarks

Create output folders first:

```bash
mkdir -p benchmark_runs benchmark_results
```

### Single benchmark run (example)

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

### Full matrix runs

#### Collision matrix (Quadtree vs Brute-Force)

```bash
./scripts/run_collision_matrix.sh
```

#### Pathfinding matrix (A* vs Dijkstra)

```bash
./scripts/run_pathfinding_matrix.sh
```

Both scripts output CSV files into `benchmark_runs/`.

---

## Analyzing Benchmark Output

### Analyze one CSV

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/your_run.csv
```

### Compare two CSVs

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/run_a.csv benchmark_runs/run_b.csv \
  --labels VariantA VariantB \
  --focus collision
```

`--focus` can be `collision`, `pathfinding`, or `both`.

### Matrix comparison (multi-level, multi-run)

```bash
python3 scripts/analyze_benchmark_csv.py compare-matrix \
  --csv-a-template "benchmark_runs/collision_quadtree_open128_{level}_run{run}.csv" \
  --csv-b-template "benchmark_runs/collision_brute_open128_{level}_run{run}.csv" \
  --labels Quadtree BruteForce \
  --focus collision \
  --levels 50 100 250 500 \
  --runs 1 2 3
```

### End-to-end post-processing

```bash
./scripts/analyze_all.sh
```

---

## Output Locations

- `benchmark_runs/*.csv` - raw benchmark data
- `benchmark_results/tables/` - human-readable summaries/comparisons
- `benchmark_results/json/` - machine-readable results
- `benchmark_results/repro/` - reproducibility logs and environment details

---

## Benchmark Maps

- `maps/benchmark_open_128.map` - collision-focused
- `maps/benchmark_maze_128.map` - pathfinding-focused
- `maps/nexus_siege_128.map` - integrated scenario validation

---

## Notes for Fair Comparisons

Keep these fixed when comparing algorithm pairs:

- Same map
- Same minion target/load
- Same warmup and duration
- Same seed and order interval
- Same FPS mode (capped vs `--unlimited-fps`)
- Same hardware/power profile

---

## Related Documentation

- `docs/Benchmark_Mode.md`
- `docs/Performance Analyze.md`
- `docs/Analyzation_guide.md`
- `docs/Algorithm_Performance_Marking_Scheme.md`

---

## Troubleshooting

- **`sfml-*` package not found:** install SFML dev libraries and `pkg-config`/`pkgconf`.
- **`spdlog` not found:** install spdlog dev package.
- **Map/CSV path not found:** paths passed through `run.sh` are resolved from `build/`, so examples use `../`.
- **No post-warmup data:** reduce warmup or increase benchmark duration.
