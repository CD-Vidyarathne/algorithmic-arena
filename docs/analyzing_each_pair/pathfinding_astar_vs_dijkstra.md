# Pathfinding pair: A\* vs Dijkstra

This guide covers **only** the pathfinding comparison. Keep **collision mode identical** on both sides (recommended: **Quadtree** via `./run.sh` defaults) so differences come from pathfinding, not from collision.

**Map:** `maps/benchmark_maze_128.map` (maze + weighted terrain).

---

## 1. Before you start

1. Work from the **repository root** (where `run.sh` lives).
2. Create a folder for CSV output if needed:

   ```bash
   mkdir -p benchmark_runs
   ```

3. Decide **minion count**, **run duration**, and **`--unlimited-fps`** — keep them **the same** for the A\* run and the Dijkstra run in a fair pair.

4. Plan **repeatable orders** (e.g. bulk select → move to far flags) so both runs stress pathfinding similarly.

---

## 2. Build flags (via `run.sh`)

`run.sh` maps pathfinding mode to CMake:

| Pathfinding | `./run.sh` flags |
|-------------|------------------|
| **A\*** | `--astar` or `--path-astar` (default) |
| **Dijkstra** | `--dijkstra` or `--path-dijkstra` |

Fix collision so it does not change between the two builds:

| Collision | `./run.sh` flags |
|-----------|------------------|
| **Quadtree** (recommended) | `--quadtree` (default) |
| Brute-Force | `--brute` — **avoid** for this pair unless you intentionally test both dimensions |

**Example:** Quadtree + A\*:

```bash
./run.sh --quadtree --astar
```

**Example:** Quadtree + Dijkstra:

```bash
./run.sh --quadtree --dijkstra
```

---

## 3. Run A — A\* pathfinding

From the repo root. Paths use **`../`** because the process cwd for the game is **`build/`**.

**Minimal example (maze map + CSV log):**

```bash
./run.sh --quadtree --astar \
  --map ../maps/benchmark_maze_128.map \
  --csv ../benchmark_runs/pathfinding_astar_maze128_100_run1.csv
```

**With uncapped FPS (use the same for run B if you use it here):**

```bash
./run.sh --quadtree --astar --unlimited-fps \
  --map ../maps/benchmark_maze_128.map \
  --csv ../benchmark_runs/pathfinding_astar_maze128_100_run1.csv
```

**In-game:** reach your target minion count, then issue the **same pattern of orders** you will use in run B (e.g. repeated long paths into the maze).

---

## 4. Run B — Dijkstra pathfinding

Same map, same minion target, same duration, same FPS mode, same order pattern — **only** pathfinding mode changes.

```bash
./run.sh --quadtree --dijkstra \
  --map ../maps/benchmark_maze_128.map \
  --csv ../benchmark_runs/pathfinding_dijkstra_maze128_100_run1.csv
```

(Add `--unlimited-fps` if run A used it.)

---

## 5. Analyse the pair

From the repo root:

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/pathfinding_astar_maze128_100_run1.csv \
  benchmark_runs/pathfinding_dijkstra_maze128_100_run1.csv \
  --labels AStar Dijkstra \
  --focus pathfinding
```

**Optional — JSON:**

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/pathfinding_astar_maze128_100_run1.csv \
  benchmark_runs/pathfinding_dijkstra_maze128_100_run1.csv \
  --labels AStar Dijkstra \
  --focus pathfinding \
  --json
```

**Single-run check:**

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/pathfinding_astar_maze128_100_run1.csv
```

---

## 6. Scalability and call load

Interpret **`pathfinding_us_sum_1s`**, **`path_calls_sum_1s`**, and **µs per call** together. For **scalability**, repeat at several minion levels and/or sustained order bursts, saving CSVs with clear names, e.g.:

- `pathfinding_astar_maze128_50_run1.csv` / `pathfinding_dijkstra_maze128_50_run1.csv`
- Repeat for `_100_`, `_250_`, etc.

The analyser’s scalability column stays at **50/50** until you combine multiple runs in your report (see `docs/Analyzation_guide.md`).

---

## 7. Checklist (fair pathfinding comparison)

- [ ] Same `benchmark_maze_128.map` for both runs  
- [ ] Same collision side (`--quadtree` on both, unless documented)  
- [ ] Same `--unlimited-fps` behaviour on both  
- [ ] Same target minion count and **same order pattern**  
- [ ] Warmup discarded in analysis (default **7 s**; override with `--warmup-seconds`)  
- [ ] Several repeats if variance is high  

---

## 8. Related docs

- `docs/Performance Analyze.md` — pathfinding matrix and conclusion templates  
- `docs/Algorithm_Performance_Marking_Scheme.md` — pathfinding factors  
- `docs/Analyzation_guide.md` — metrics, pseudocode, thesis checklist  
