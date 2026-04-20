# Benchmark Analysis Guide

This guide describes **end-to-end** how to collect benchmark data from Algorithmic Arena, analyse CSV output, and interpret results in line with the project marking materials. It complements:

- `docs/Performance Analyze.md` — execution and reporting workflow  
- `docs/Algorithm_Performance_Marking_Scheme.md` — rubric (four factors per algorithm pair)  
- `docs/11_maps_for_performance_analysis.md` — which maps isolate collision vs pathfinding  
- `docs/Benchmark_Mode.md` — automated benchmark runs (fixed duration/load)  
- `scripts/analyze_benchmark_csv.py` — reference implementation (Python 3, standard library only)

---

## 1. What gets measured

While the game runs with `--csv <path>`, it appends **one row per second** with:

| Column | Meaning |
|--------|---------|
| `timestamp_s` | Game time in seconds |
| `fps` | Smoothed frames per second |
| `entity_count` | Entities in the world |
| `collision_us_sum_1s` | Collision CPU time summed over the last second (microseconds) |
| `pathfinding_us_sum_1s` | Pathfinding CPU time summed over the last second (microseconds) |
| `path_calls_sum_1s` | Number of path requests completed in that second |
| `minion_count` | Minions (used as load proxy) |

**Derived quantities** (computed after discarding warmup) match Performance Analyze:

- **Path µs per call (aggregate):**  
  \(\displaystyle \frac{\sum \texttt{pathfinding\_us\_sum\_1s}}{\sum \texttt{path\_calls\_sum\_1s}}\) over the steady window (if total calls > 0).

- **Collision µs per minion (aggregate):**  
  \(\displaystyle \frac{\sum \texttt{collision\_us\_sum\_1s}}{\sum \texttt{minion\_count}}\) over the steady window (if total minions > 0).

- **FPS spread:** \(\max(\texttt{fps}) - \min(\texttt{fps})\) over the steady window.

- **Per-row path µs/call (mean of ratios):** for each row with `path_calls_sum_1s > 0`, compute `pathfinding_us_sum_1s / path_calls_sum_1s`, then take the mean (useful for cross-checking the aggregate).

---

## 2. Prerequisites

- **Python 3** on your PATH (no third-party packages required).
- **Built game binary** (e.g. `build/AlgorithmicArena` or your IDE output).
- **Fair controls:** same machine, power mode, map, minion target, duration, and FPS mode (`--unlimited-fps` or capped) for both algorithms in a comparison pair.

---

## 3. Recommended folder layout

From the project root:

```text
algorithmic-arena/
  benchmark_runs/
    collision_open128_quad_100_run1.csv
    collision_open128_brute_100_run1.csv
    maze128_astar_100_run1.csv
    maze128_dijkstra_100_run1.csv
  scripts/
    analyze_benchmark_csv.py
  maps/
    benchmark_open_128.map
    benchmark_maze_128.map
```

Use a **clear filename pattern** (Performance Analyze suggests):

`{pair}_{algorithm}_{map}_{minions}_run{N}.csv`

Example: `collision_quadtree_open128_250_run2.csv`

---

## 4. Step-by-step: capture a CSV from the game

### 4.1 Build the variant under test

Examples (CMake options are illustrative; use your project’s exact flags):

- **Collision pair:** build once with Quadtree collision ON, once with OFF (Brute-Force).
- **Pathfinding pair:** build once with A* ON, once with Dijkstra (A* OFF).

Keep **everything else equal** between the two runs in a pair (especially pathfinding mode when testing collision, and collision mode when testing pathfinding).

### 4.2 Run the executable with map and CSV path

From the `build` directory (adjust paths if your layout differs):

**Collision-focused map (open field):**

```bash
./AlgorithmicArena \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_quadtree_open128_100_run1.csv
```

**Pathfinding-focused map (maze):**

```bash
./AlgorithmicArena \
  --map ../maps/benchmark_maze_128.map \
  --csv ../benchmark_runs/pathfinding_astar_maze128_100_run1.csv
```

**Optional — uncapped FPS** (if you use it for one run, use it for all runs in a fair comparison):

```bash
./AlgorithmicArena --unlimited-fps \
  --map ../maps/benchmark_open_128.map \
  --csv ../benchmark_runs/collision_quadtree_open128_100_run1.csv
```

### 4.3 Play the benchmark scenario

1. Start the match.
2. Reach your **target minion count** and keep it as stable as possible for the planned duration (e.g. 60 seconds).
3. Perform the **same player actions** across compared runs (e.g. same mass-move orders on maze tests).
4. Exit or stop when the timer matches your plan.

### 4.4 Repeat

- At least **3 runs per condition** (Performance Analyze), more if variance is high.
- For **scalability**, repeat at several minion levels (e.g. 50, 100, 250, 500) and keep CSVs separate.

---

## 5. Step-by-step: analyse CSVs with the Python script

The script lives at `scripts/analyze_benchmark_csv.py`. Default **warmup skip** is **7 seconds** (Performance Analyze recommends discarding roughly 5–10 seconds).

### 5.1 Summarise a single run

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/collision_quadtree_open128_100_run1.csv
```

Shorthand (same result):

```bash
python3 scripts/analyze_benchmark_csv.py benchmark_runs/collision_quadtree_open128_100_run1.csv
```

**Custom warmup:**

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/your_run.csv --warmup-seconds 10
```

**Machine-readable JSON:**

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/your_run.csv --json
```

Output includes: means, FPS spread, aggregate derived metrics, **absolute band hints** (single-run only; pairwise scoring is authoritative for comparing algorithms).

### 5.2 Compare two runs (pairwise marking)

**Collision pair (Quadtree vs Brute-Force):**

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/collision_quadtree_open128_100_run1.csv \
  benchmark_runs/collision_brute_open128_100_run1.csv \
  --labels Quadtree BruteForce \
  --focus collision
```

**Pathfinding pair (A* vs Dijkstra):**

```bash
python3 scripts/analyze_benchmark_csv.py compare \
  benchmark_runs/maze128_astar_100_run1.csv \
  benchmark_runs/maze128_dijkstra_100_run1.csv \
  --labels AStar Dijkstra \
  --focus pathfinding
```

**Both tables (collision + pathfinding factors) on the same two files:**

```bash
python3 scripts/analyze_benchmark_csv.py compare run_a.csv run_b.csv \
  --labels VariantA VariantB \
  --focus both
```

**JSON export (includes `marking.collision` / `marking.pathfinding` score dicts):**

```bash
python3 scripts/analyze_benchmark_csv.py compare quad.csv brute.csv \
  --labels Quadtree BruteForce --focus collision --json
```

### 5.3 How to read the `compare` output

1. **Headline metrics** — side-by-side means; which side is “favourable” for lower-cost or higher-FPS metrics.
2. **Marking table** — four factors (Speed, Scalability placeholder, CPU, Stability) as **relative 0–100%** scores that sum to 100% **per factor column** between the two algorithms (pairwise split). **Overall** is the arithmetic mean of the four factor scores for each algorithm.
3. **Scalability** — the script sets **50% / 50%** and prints a note: true scalability requires **multiple CSVs** at different minion counts (see Performance Analyze benchmark matrix). For the thesis, add a table or graph from those sweeps.
4. **Conclusion lines** — short templates aligned with Performance Analyze report wording; if overall scores are within **5 points**, a tie-break note appears (use factor detail and multi-minion data).

---

## 6. Mapping script output to the marking scheme

| Marking factor | Collision pair (script) | Pathfinding pair (script) |
|----------------|-------------------------|---------------------------|
| Speed | Lower mean `collision_us_sum_1s` | Lower mean `pathfinding_us_sum_1s` |
| Scalability | Placeholder 50% until multi-level data | Same |
| CPU efficiency | Lower aggregate collision µs per minion | Lower aggregate path µs per call |
| Stability | Higher mean FPS; lower FPS spread | Same |

**Overall (script):** average of the four factor scores for each algorithm. This is a **pairwise relative** summary, consistent with the marking scheme’s instruction to score **relative to the other algorithm in the pair**.

---

## 7. Thesis checklist

- [ ] Store raw CSVs with consistent names under `benchmark_runs/` (or appendix folder).
- [ ] Document build flags, map, minion count, duration, FPS mode for every run.
- [ ] For each pair, run `compare` and paste or summarise tables in the thesis.
- [ ] Add **scalability** evidence from **multiple minion levels** (not only a single CSV).
- [ ] Report mean ± spread across repeated runs (Performance Analyze statistical handling).
- [ ] Cite `Algorithm_Performance_Marking_Scheme.md` for qualitative factor bands where you interpret absolute performance.

---

## 8. Pseudocode: benchmark CSV analysis pipeline

The following pseudocode abstracts `scripts/analyze_benchmark_csv.py` for use in a thesis **Methods** or **Evaluation** section. It is notation-focused and omits file I/O details.

### 8.1 Data structures

```text
RECORD:
    timestamp_s
    fps
    entity_count
    collision_us_sum_1s
    pathfinding_us_sum_1s
    path_calls_sum_1s
    minion_count

RUN_SUMMARY:
    mean_fps
    fps_spread                    // max(fps) − min(fps) on steady rows
    mean_collision_us_sum_1s
    mean_pathfinding_us_sum_1s
    path_us_per_call_agg          // Σ pathfinding / Σ path_calls
    collision_us_per_minion_agg   // Σ collision / Σ minion_count
    ...                           // other means as needed
```

### 8.2 Load and steady window

```text
function LOAD_CSV(path):
    rows ← read comma-separated records with required column headers
    return rows

function STEADY_ROWS(rows, warmup_seconds):
    return all r in rows such that r.timestamp_s ≥ warmup_seconds
```

### 8.3 Summarise one run

```text
function SUMMARIZE(rows_steady):
    mean_fps ← arithmetic mean of rows_steady.fps
    fps_spread ← max(fps) − min(fps) over rows_steady
    mean_collision ← mean(rows_steady.collision_us_sum_1s)
    mean_pathfinding ← mean(rows_steady.pathfinding_us_sum_1s)
    total_pathfinding ← Σ rows_steady.pathfinding_us_sum_1s
    total_path_calls ← Σ rows_steady.path_calls_sum_1s
    if total_path_calls > 0:
        path_us_per_call_agg ← total_pathfinding / total_path_calls
    else:
        path_us_per_call_agg ← null

    total_collision ← Σ rows_steady.collision_us_sum_1s
    total_minions ← Σ rows_steady.minion_count
    if total_minions > 0:
        collision_us_per_minion_agg ← total_collision / total_minions
    else:
        collision_us_per_minion_agg ← null

    return RUN_SUMMARY(...)
```

### 8.4 Pairwise relative scores (two algorithms, one metric)

For any metric where **lower is better** (e.g. collision time):

```text
function PAIRWISE_SPLIT_LOWER(vA, vB):
    // Returns (scoreA, scoreB) in [0, 100], scoreA + scoreB = 100
    s ← vA + vB
    if s ≤ 0:
        return (50, 50)
    scoreA ← 100 × (vB / s)
    scoreB ← 100 × (vA / s)
    return (scoreA, scoreB)
```

For **higher is better** (e.g. mean FPS):

```text
function PAIRWISE_SPLIT_HIGHER(vA, vB):
    s ← vA + vB
    if s ≤ 0:
        return (50, 50)
    scoreA ← 100 × (vA / s)
    scoreB ← 100 × (vB / s)
    return (scoreA, scoreB)
```

**Stability** combines mean FPS (higher better) and FPS spread (lower better):

```text
function STABILITY_SCORES(mean_fps_A, mean_fps_B, spread_A, spread_B):
    (mA, mB) ← PAIRWISE_SPLIT_HIGHER(mean_fps_A, mean_fps_B)
    if spread_A ≤ 0 and spread_B ≤ 0:
        return (mA, mB)
    (sA, sB) ← PAIRWISE_SPLIT_LOWER(spread_A, spread_B)
    stabA ← (mA + sA) / 2
    stabB ← (mB + sB) / 2
    return (stabA, stabB)
```

### 8.5 Four-factor marking for collision (Quadtree vs Brute-Force)

```text
function MARK_COLLISION_PAIR(summaryA, summaryB):
    (speedA, speedB) ← PAIRWISE_SPLIT_LOWER(
        summaryA.mean_collision_us_sum_1s,
        summaryB.mean_collision_us_sum_1s
    )

    (cpuA, cpuB) ← PAIRWISE_SPLIT_LOWER(
        summaryA.collision_us_per_minion_agg,
        summaryB.collision_us_per_minion_agg
    )   // if aggregates missing, use (50, 50)

    (stabA, stabB) ← STABILITY_SCORES(
        summaryA.mean_fps, summaryB.mean_fps,
        summaryA.fps_spread, summaryB.fps_spread
    )

    scaleA ← 50   // placeholder: true scalability needs multiple load levels
    scaleB ← 50

    overallA ← (speedA + scaleA + cpuA + stabA) / 4
    overallB ← (speedB + scaleB + cpuB + stabB) / 4

    return (speedA, speedB, scaleA, scaleB, cpuA, cpuB, stabA, stabB, overallA, overallB)
```

### 8.6 Four-factor marking for pathfinding (A* vs Dijkstra)

```text
function MARK_PATHFINDING_PAIR(summaryA, summaryB):
    (speedA, speedB) ← PAIRWISE_SPLIT_LOWER(
        summaryA.mean_pathfinding_us_sum_1s,
        summaryB.mean_pathfinding_us_sum_1s
    )

    (cpuA, cpuB) ← PAIRWISE_SPLIT_LOWER(
        summaryA.path_us_per_call_agg,
        summaryB.path_us_per_call_agg
    )

    (stabA, stabB) ← STABILITY_SCORES(
        summaryA.mean_fps, summaryB.mean_fps,
        summaryA.fps_spread, summaryB.fps_spread
    )

    scaleA ← 50
    scaleB ← 50

    overallA ← (speedA + scaleA + cpuA + stabA) / 4
    overallB ← (speedB + scaleB + cpuB + stabB) / 4

    return (..., overallA, overallB)
```

### 8.7 Winner and tie rule (overall)

```text
function OVERALL_WINNER(overallA, overallB, nameA, nameB):
    if |overallA − overallB| < 5:
        winner ← nameA if overallA > overallB else nameB if overallB > overallA else "tie"
        note ← "Within 5 points: use factor detail and multi-minion scalability data"
        return (winner, note)
    else:
        return (nameA if overallA > overallB else nameB, "")
```

### 8.8 End-to-end `compare` workflow

```text
procedure COMPARE_TWO_CSV(pathA, pathB, warmup_seconds, focus):
    rowsA ← LOAD_CSV(pathA)
    rowsB ← LOAD_CSV(pathB)
    steadyA ← STEADY_ROWS(rowsA, warmup_seconds)
    steadyB ← STEADY_ROWS(rowsB, warmup_seconds)

    summaryA ← SUMMARIZE(steadyA)
    summaryB ← SUMMARIZE(steadyB)

    print headline metrics (means, derived aggregates)

    if focus includes collision:
        scores ← MARK_COLLISION_PAIR(summaryA, summaryB)
        print factor table and overalls for collision pair

    if focus includes pathfinding:
        scores ← MARK_PATHFINDING_PAIR(summaryA, summaryB)
        print factor table and overalls for pathfinding pair

    apply OVERALL_WINNER to the relevant overall scores
    print short conclusion templates for the thesis report
```

---

## 9. Version note

The pseudocode matches the logic in `scripts/analyze_benchmark_csv.py` at the time this document was written. If the script changes, update the pseudocode section to stay consistent.
