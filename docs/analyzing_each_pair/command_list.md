# Thesis Benchmark Command List

Use this file as the exact command checklist for:

- Collision pair runs (Quadtree vs Brute), multi-level, repeated
- Pathfinding pair runs (A* vs Dijkstra), multi-level, repeated
- Mean/std + trend-ready data exports
- Raw CSV appendix + reproducibility command log

## 0) Setup once

From repo root:

```bash
mkdir -p benchmark_runs benchmark_results benchmark_results/json benchmark_results/tables benchmark_results/repro
```

Create a reproducibility log file for all commands:

```bash
LOG="benchmark_results/repro/commands_$(date +%Y%m%d_%H%M%S).log"
echo "# Benchmark command log" | tee -a "$LOG"
date | tee -a "$LOG"
git rev-parse HEAD | tee -a "$LOG"
```

Recommended shared benchmark parameters (edit once, reuse everywhere):

```bash
WARMUP=7
DURATION=60
ORDER_INTERVAL=1.5
SEED=1337
REPEATS=(1 2 3)
LEVELS=(50 100 250 500)
```

---

## 1) Collision pair matrix (Quadtree vs Brute, map=open_128)

### 1.1 Run Quadtree (A* fixed)

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --quadtree --astar --unlimited-fps \
      --map ../maps/benchmark_open_128.map \
      --csv ../benchmark_runs/collision_quadtree_open128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done
```

### 1.2 Run Brute-Force (A* fixed)

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --brute --astar --unlimited-fps \
      --map ../maps/benchmark_open_128.map \
      --csv ../benchmark_runs/collision_brute_open128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done
```

### 1.3 Pairwise compare each level/repeat

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    TXT="benchmark_results/tables/collision_compare_${N}_run${R}.txt"
    JSON="benchmark_results/json/collision_compare_${N}_run${R}.json"
    python3 scripts/analyze_benchmark_csv.py compare \
      "benchmark_runs/collision_quadtree_open128_${N}_run${R}.csv" \
      "benchmark_runs/collision_brute_open128_${N}_run${R}.csv" \
      --labels Quadtree BruteForce --focus collision \
      --warmup-seconds "${WARMUP}" | tee "$TXT"

    python3 scripts/analyze_benchmark_csv.py compare \
      "benchmark_runs/collision_quadtree_open128_${N}_run${R}.csv" \
      "benchmark_runs/collision_brute_open128_${N}_run${R}.csv" \
      --labels Quadtree BruteForce --focus collision \
      --warmup-seconds "${WARMUP}" --json > "$JSON"
  done
done
```

---

## 2) Pathfinding pair matrix (A* vs Dijkstra, collision=Quadtree fixed)

### 2.1 Run A* (Quadtree fixed)

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --quadtree --astar --unlimited-fps \
      --map ../maps/benchmark_maze_128.map \
      --csv ../benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done
```

### 2.2 Run Dijkstra (Quadtree fixed)

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    CMD="./run.sh --quadtree --dijkstra --unlimited-fps \
      --map ../maps/benchmark_maze_128.map \
      --csv ../benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv \
      --benchmark-mode --benchmark-duration-s ${DURATION} --benchmark-warmup-s ${WARMUP} \
      --benchmark-target-minions ${N} --benchmark-order-interval-s ${ORDER_INTERVAL} --benchmark-seed ${SEED}"
    echo "$CMD" | tee -a "$LOG"
    eval "$CMD"
  done
done
```

### 2.3 Pairwise compare each level/repeat

```bash
for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    TXT="benchmark_results/tables/pathfinding_compare_${N}_run${R}.txt"
    JSON="benchmark_results/json/pathfinding_compare_${N}_run${R}.json"
    python3 scripts/analyze_benchmark_csv.py compare \
      "benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv" \
      "benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv" \
      --labels AStar Dijkstra --focus pathfinding \
      --warmup-seconds "${WARMUP}" | tee "$TXT"

    python3 scripts/analyze_benchmark_csv.py compare \
      "benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv" \
      "benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv" \
      --labels AStar Dijkstra --focus pathfinding \
      --warmup-seconds "${WARMUP}" --json > "$JSON"
  done
done
```

---

## 3) Mean/std + trend-ready aggregate tables

Generate one CSV for collision and one for pathfinding, ready for plotting in spreadsheet/LaTeX tools:

```bash
python3 - <<'PY'
import csv, json, glob, os, statistics

def agg(files, key):
    vals = []
    for p in files:
        with open(p, "r", encoding="utf-8") as f:
            d = json.load(f)
        vals.append(d["marking"][key])
    out = {}
    fields = ["speed_a","speed_b","cpu_efficiency_a","cpu_efficiency_b",
              "stability_a","stability_b","overall_a","overall_b"]
    for f in fields:
        series = [v[f] for v in vals]
        out[f+"_mean"] = sum(series)/len(series)
        out[f+"_std"] = statistics.stdev(series) if len(series) > 1 else 0.0
    return out

os.makedirs("benchmark_results/tables", exist_ok=True)

for pair in ["collision","pathfinding"]:
    rows = []
    for n in [50,100,250,500]:
        files = sorted(glob.glob(f"benchmark_results/json/{pair}_compare_{n}_run*.json"))
        if not files:
            continue
        s = agg(files, pair)
        s["minions"] = n
        rows.append(s)
    if rows:
        outp = f"benchmark_results/tables/{pair}_mean_std_by_level.csv"
        with open(outp, "w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            w.writeheader()
            w.writerows(rows)
        print("wrote", outp)
PY
```

These two files are your trend-graph source:

- `benchmark_results/tables/collision_mean_std_by_level.csv`
- `benchmark_results/tables/pathfinding_mean_std_by_level.csv`

---

## 4) Tie-break logic when close (within 5 points)

Quick check for close overalls from per-run JSON:

```bash
python3 - <<'PY'
import glob, json
for p in sorted(glob.glob("benchmark_results/json/*_compare_*_run*.json")):
    d = json.load(open(p, encoding="utf-8"))
    m = next(iter(d["marking"].values()))
    gap = abs(m["overall_a"] - m["overall_b"])
    if gap < 5.0:
        print(f"TIE-BREAK NEEDED (<5): {p}  gap={gap:.2f}")
PY
```

---

## 5) Raw CSV appendix + reproducibility table exports

List all raw CSV files for appendix:

```bash
ls benchmark_runs/*.csv > benchmark_results/repro/raw_csv_appendix_list.txt
```

Create a command reproducibility table (command + timestamp lines):

```bash
cp "$LOG" benchmark_results/repro/command_reproducibility_table.txt
```

Record environment info for thesis appendix:

```bash
{
  echo "Date: $(date)"
  echo "Kernel: $(uname -a)"
  echo "Git commit: $(git rev-parse HEAD)"
} > benchmark_results/repro/environment.txt
```

---

## 6) Minimal sanity check commands

```bash
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/collision_quadtree_open128_100_run1.csv --warmup-seconds "${WARMUP}"
python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/pathfinding_astar_maze128_100_run1.csv --warmup-seconds "${WARMUP}"
```

