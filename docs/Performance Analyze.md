# Performance Analysis Guide

This document is the **execution guide** for measuring, marking, analyzing, and concluding performance differences between:

- **Collision:** Quadtree vs Brute-Force
- **Pathfinding:** A* vs Dijkstra

It is intentionally written to be **compatible** with:

- `docs/Algorithm_Performance_Marking_Scheme.md` (primary scoring rubric)
- `docs/11_maps_for_performance_analysis.md` (map selection and benchmark intent)
- `docs/Benchmark_Mode.md` (automated execution mode and CLI)

---

## 1. Objective and hypothesis

### 1.1 Objective

Produce repeatable, fair, and evidence-based comparisons of algorithm performance inside the current game artefact.

### 1.2 Hypothesis

- **Collision:** Quadtree should outperform Brute-Force as minion/entity count increases, especially under clustering.
- **Pathfinding:** A* should outperform Dijkstra on large/maze-like weighted maps due to heuristic guidance.

---

## 2. Data sources and metrics

Use CSV produced by `--csv <path>` during benchmark runs.

### 2.1 CSV columns used

- `timestamp_s`
- `fps`
- `entity_count`
- `collision_us_sum_1s`
- `pathfinding_us_sum_1s`
- `path_calls_sum_1s`
- `minion_count`

### 2.2 Derived metrics (compute in analysis sheet/script)

- **Path cost per call (us/call):**  
  `pathfinding_us_sum_1s / path_calls_sum_1s` (only where calls > 0)
- **Collision cost per minion (us/minion):**  
  `collision_us_sum_1s / minion_count` (only where minion_count > 0)
- **Stability spread (FPS):**  
  `max_fps - min_fps` over the steady-state window

### 2.3 Automated analysis (`scripts/analyze_benchmark_csv.py`)

Use the same warmup window as §3 (default **7 s** in the script; override with `--warmup-seconds`).

- **Single run:** `python3 scripts/analyze_benchmark_csv.py analyze <run.csv>` — prints Performance Analyze §2.2 metrics and **absolute band hints** from `Algorithm_Performance_Marking_Scheme.md` (single-run only; pairwise scoring is authoritative).
- **Pair compare:** `python3 scripts/analyze_benchmark_csv.py compare <a.csv> <b.csv> --labels <NameA> <NameB> --focus collision|pathfinding|both` — prints **relative 0–100% factor scores** (Performance Analyze §6.1), overall average (§6.2), tie-break note (§6.3), and **report template lines** aligned with §8.1–§8.2. Scalability stays at placeholder 50% until you add CSVs across minion levels (§4.1–4.2).
- **JSON:** add `--json` on either subcommand for machine-readable output (includes `marking` on `compare`).

---

## 3. Fair test controls (must hold for all comparisons)

To keep scoring valid against `Algorithm_Performance_Marking_Scheme.md`:

1. Same machine and power mode.
2. Same map file for both algorithms in a pair.
3. Same minion target count for both algorithms in a pair.
4. Same runtime mode (either capped or `--unlimited-fps`), clearly reported.
5. Same benchmark duration per run.
6. Keep heavy debug visualizations off for numeric runs:
   - F2 path debug should be OFF unless the run is explicitly labelled “visualization cost run”.
7. Discard warm-up:
   - Remove first 5-10 seconds before averaging.

---

## 4. Benchmark matrix (what to run)

Reference maps:

- `maps/benchmark_open_128.map` (collision-focused)
- `maps/benchmark_maze_128.map` (pathfinding-focused)
- `maps/nexus_siege_128.map` (integrated narrative check)

### 4.1 Collision matrix (Quadtree vs Brute-Force)

- **Map:** `benchmark_open_128.map`
- **Pathfinding mode fixed:** keep constant for both runs (recommended A* ON)
- **Collision build pair:**
  - `USE_QUADTREE_COLLISION=ON`
  - `USE_QUADTREE_COLLISION=OFF`
- **Minion levels:** 50, 100, 250, 500 (or your feasible max)
- **Duration:** 60 seconds each run
- **Repeats:** minimum 3 runs per condition

Primary outputs for scoring:

- Mean `collision_us_sum_1s`
- Mean `fps`
- Growth trend as `minion_count` rises

### 4.2 Pathfinding matrix (A* vs Dijkstra)

- **Map:** `benchmark_maze_128.map`
- **Collision mode fixed:** keep constant for both runs (recommended Quadtree ON)
- **Pathfinding build pair:**
  - `USE_ASTAR_PATHFINDING=ON`
  - `USE_ASTAR_PATHFINDING=OFF` (Dijkstra)
- **Order pattern:** use same player actions per run (mass order to far objectives)
- **Minion levels:** 50, 100, 250, 500 (or feasible max)
- **Duration:** 60 seconds each run
- **Repeats:** minimum 3 runs per condition

Primary outputs for scoring:

- Mean `pathfinding_us_sum_1s`
- Mean `path_calls_sum_1s`
- Mean `pathfinding_us_sum_1s / path_calls_sum_1s`
- Mean `fps`

### 4.3 Integrated validation (optional but recommended)

- **Map:** `nexus_siege_128.map`
- Use one fixed minion count and compare pairwise builds to support narrative conclusions.
- Do not use this as the only evidence for pairwise scoring; it is supporting evidence.

---

## 5. Run procedure (per single run)

1. Build the selected algorithm configuration.
2. Launch with:
   - `--map <map_path>`
   - `--csv <output_csv>`
   - optional `--unlimited-fps` (if chosen, use for all runs)
3. Start match and execute the same action sequence.
4. Maintain target load (minion level) for full run.
5. End at fixed duration (60s recommended).
6. Save CSV with clear naming:
   - `<pair>_<algo>_<map>_<minions>_run<N>.csv`
   - example: `collision_quadtree_open128_250_run2.csv`

---

## 6. Marking method (compatible with algorithm marking scheme)

Use `docs/Algorithm_Performance_Marking_Scheme.md` directly for factor bands.

### 6.1 Score each algorithm on 4 factors (0-100%)

For each algorithm, score:

1. **Speed**
2. **Scalability**
3. **CPU Efficiency**
4. **Stability**

Do this separately for:

- Quadtree and Brute-Force (collision section of scheme)
- A* and Dijkstra (pathfinding section of scheme)

### 6.2 Overall per-algorithm score

`overall = (speed + scalability + cpu_efficiency + stability) / 4`

### 6.3 Winner per pair

- Collision winner = higher overall between Quadtree and Brute-Force
- Pathfinding winner = higher overall between A* and Dijkstra

If overall scores are close (within 5%), use factor-level interpretation (especially scalability and stability) to justify the final call.

---

## 7. Analysis workflow (how to interpret results)

### 7.1 Collision analysis (Quadtree vs Brute-Force)

Analyze in this order:

1. **Speed:** compare mean `collision_us_sum_1s` at each minion level.
2. **Scalability:** inspect growth curve from 50 -> 500 minions.
3. **CPU efficiency:** compare `collision_us_sum_1s / minion_count`.
4. **Stability:** compare FPS means and FPS spread.

Expected pattern:

- Brute-Force curve grows sharply with load.
- Quadtree has better growth profile and smoother FPS at medium/high counts.

### 7.2 Pathfinding analysis (A* vs Dijkstra)

Analyze in this order:

1. **Speed:** compare `pathfinding_us_sum_1s` under same scenario.
2. **Call-normalized efficiency:** compare `us/call`.
3. **Scalability:** see if cost rise is controlled as calls/load increase.
4. **Stability:** inspect FPS impact during heavy ordering.

Expected pattern:

- A* lower `us/call` and/or lower total pathfinding cost in maze-weighted scenarios.
- Dijkstra higher expansion effort in complex maps.

### 7.3 Statistical handling (simple, report-friendly)

For each condition:

- Report mean and standard deviation across runs.
- If variance is high, add one extra repeat and note likely source (e.g., spawn clustering randomness).

---

## 8. Conclusion method (how to arrive at final claims)

Use this four-step structure in report and viva:

1. **State result:** which algorithm scored higher and by how much.
2. **Support with data:** quote key metrics and graphs.
3. **Link to theory:** explain with complexity/heuristic behavior and implementation realities.
4. **State limitations:** machine-specific results, frame budget effects, debug overhead considerations.

### 8.1 Conclusion template (collision)

"On `benchmark_open_128.map`, Quadtree achieved lower collision time and better FPS stability than Brute-Force as minion count increased, resulting in a higher overall marking score. The scalability trend supports expected spatial partitioning advantages at higher densities."

### 8.2 Conclusion template (pathfinding)

"On `benchmark_maze_128.map`, A* achieved lower pathfinding cost per call and better frame stability under sustained orders than Dijkstra, producing a higher overall marking score. This aligns with heuristic-guided search reducing unnecessary exploration on weighted maze terrain."

---

## 9. Deliverables checklist

- [ ] Raw CSV files for every benchmark run stored and named consistently
- [ ] Aggregated results table (mean/std)
- [ ] Factor scores per algorithm using `Algorithm_Performance_Marking_Scheme.md`
- [ ] Final score summary table:
  - Quadtree vs Brute-Force
  - A* vs Dijkstra
- [ ] At least 2 charts per pair (cost trend + FPS/stability)
- [ ] Written conclusions with limitations and confidence level

---

## 10. Scoring summary table template

| Algorithm | Speed | Scalability | CPU Efficiency | Stability | Overall (avg) |
|-----------|-------|-------------|----------------|-----------|---------------|
| Quadtree | | | | | |
| Brute-Force | | | | | |
| A* | | | | | |
| Dijkstra | | | | | |

