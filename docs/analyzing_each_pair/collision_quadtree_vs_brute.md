# Collision pair: Quadtree vs Brute-Force

This guide covers **only** the collision-detection comparison. Keep **pathfinding mode identical** on both sides (recommended: **A\*** via `./run.sh` defaults) so differences come from collision, not from pathfinding.

**Map:** `maps/benchmark_open_128.map` (open interior, collision stress).

---

## 1. Before you start

1. Work from the **repository root** (where `run.sh` lives).
2. Create a folder for CSV output (if you do not have one yet):

   ```bash
   mkdir -p benchmark_runs
   ```

3. Decide **minion count**, **duration** (e.g. 60 s of steady load), and whether you use **`--unlimited-fps`** — use the **same** choices for Quadtree and Brute-Force runs in a fair pair.

---

## 2. Build flags (via `run.sh`)

`run.sh` maps collision mode to CMake:

| Collision algorithm | `./run.sh` flags |
|---------------------|------------------|
| **Quadtree** | `--quadtree` or `--collision-quadtree` (default) |
| **Brute-Force** | `--brute` or `--collision-brute` |

Fix pathfinding so it does not change between the two builds:

| Pathfinding | `./run.sh` flags |
|-------------|------------------|
| **A\*** (recommended) | `--astar` or `--path-astar` (default) |
| Dijkstra | `--dijkstra` — **avoid** for this pair unless you intentionally test both dimensions |

**Example:** Quadtree + A\* (default stack):

```bash
./run.sh --quadtree --astar
```

**Example:** Brute-Force + A\*:

```bash
./run.sh --brute --astar
```

---

## 3. Run A — Quadtree collision

From the repo root. The game runs with working directory **`build/`**, so map and CSV paths use **`../`** to reach the repo root.

**Minimal example (open benchmark map + CSV log):**

```bash
./run.sh --quadtree --astar \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_quadtree_open128_100_run1.csv
```

**With uncapped FPS (use the same for run B if you use it here):**

```bash
./run.sh --quadtree --astar --unlimited-fps \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_quadtree_open128_100_run1.csv
```

**In-game:** spawn to your target minion count, keep load stable for your planned time (e.g. 60 s after warmup), use the same style of movement/orders you will use for run B.

---

## 4. Run B — Brute-Force collision

Same map, same target minion count, same duration, same FPS mode, same player actions — **only** collision mode changes.

```bash
./run.sh --brute --astar \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_brute_open128_100_run1.csv
```

(If you used `--unlimited-fps` on run A, add it here too.)

---

## 5. Analyse the pair

After both CSV files exist, from the repo root:

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/collision_quadtree_open128_100_run1.csv \
  benchmark_runs/collision_brute_open128_100_run1.csv \
  --labels Quadtree BruteForce \
  --focus collision
```

**Optional — JSON for a spreadsheet or thesis appendix:**

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/collision_quadtree_open128_100_run1.csv \
  benchmark_runs/collision_brute_open128_100_run1.csv \
  --labels Quadtree BruteForce \
  --focus collision \
  --json
```

**Single-run sanity check** (one CSV only):

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/collision_quadtree_open128_100_run1.csv
```

---

## 6. Scalability (multiple minion levels)

The `compare` script sets **scalability to a placeholder** until you provide **several CSV pairs** at different minion counts (e.g. 50, 100, 250, 500). Repeat sections 3–5 for each level, naming files clearly, e.g.:

- `collision_quadtree_open128_50_run1.csv`
- `collision_brute_open128_50_run1.csv`
- … same for `_100_`, `_250_`, etc.

Then summarise trends in your report (tables or graphs).

---

## 7. Checklist (fair collision comparison)

- [ ] Same `benchmark_open_128.map` for both runs  
- [ ] Same pathfinding side (`--astar` on both, unless you document a deliberate change)  
- [ ] Same `--unlimited-fps` behaviour on both  
- [ ] Same target minion count and similar gameplay script  
- [ ] Same approximate run length; discard warmup (default **7 s** in the analyser, adjustable with `--warmup-seconds`)  
- [ ] At least **three** repeats per condition if you need stable means  

---

## 8. Related docs

- `docs/Performance Analyze.md` — collision matrix and conclusion templates  
- `docs/Algorithm_Performance_Marking_Scheme.md` — collision factors (speed, scalability, CPU efficiency, stability)  
- `docs/Analyzation_guide.md` — full analysis pipeline and pseudocode  
