#!/usr/bin/env bash
set -euo pipefail

mkdir -p benchmark_results/json benchmark_results/tables benchmark_results/repro

WARMUP="${WARMUP:-7}"
REPEATS=(${REPEATS:-1 2 3})
LEVELS=(${LEVELS:-50 100 250 500})

for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    TXT="benchmark_results/tables/collision_compare_${N}_run${R}.txt"
    JSON="benchmark_results/json/collision_compare_${N}_run${R}.json"
    python3 scripts/analyze_benchmark_csv.py --warmup-seconds "${WARMUP}" compare \
      "benchmark_runs/collision_quadtree_open128_${N}_run${R}.csv" \
      "benchmark_runs/collision_brute_open128_${N}_run${R}.csv" \
      --labels Quadtree BruteForce --focus collision | tee "$TXT"

    python3 scripts/analyze_benchmark_csv.py --warmup-seconds "${WARMUP}" compare \
      "benchmark_runs/collision_quadtree_open128_${N}_run${R}.csv" \
      "benchmark_runs/collision_brute_open128_${N}_run${R}.csv" \
      --labels Quadtree BruteForce --focus collision --json > "$JSON"
  done
done

for N in "${LEVELS[@]}"; do
  for R in "${REPEATS[@]}"; do
    TXT="benchmark_results/tables/pathfinding_compare_${N}_run${R}.txt"
    JSON="benchmark_results/json/pathfinding_compare_${N}_run${R}.json"
    python3 scripts/analyze_benchmark_csv.py --warmup-seconds "${WARMUP}" compare \
      "benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv" \
      "benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv" \
      --labels AStar Dijkstra --focus pathfinding | tee "$TXT"

    python3 scripts/analyze_benchmark_csv.py --warmup-seconds "${WARMUP}" compare \
      "benchmark_runs/pathfinding_astar_maze128_${N}_run${R}.csv" \
      "benchmark_runs/pathfinding_dijkstra_maze128_${N}_run${R}.csv" \
      --labels AStar Dijkstra --focus pathfinding --json > "$JSON"
  done
done

python3 - <<'PY'
import csv
import glob
import json
import os
import statistics


def aggregate(files, key):
    vals = []
    for path in files:
        with open(path, "r", encoding="utf-8") as f:
            data = json.load(f)
        vals.append(data["marking"][key])

    out = {}
    fields = [
        "speed_a",
        "speed_b",
        "cpu_efficiency_a",
        "cpu_efficiency_b",
        "stability_a",
        "stability_b",
        "overall_a",
        "overall_b",
    ]
    for field in fields:
        series = [v[field] for v in vals]
        out[field + "_mean"] = sum(series) / len(series)
        out[field + "_std"] = statistics.stdev(series) if len(series) > 1 else 0.0
    return out


os.makedirs("benchmark_results/tables", exist_ok=True)
for pair in ["collision", "pathfinding"]:
    rows = []
    for n in [50, 100, 250, 500]:
        files = sorted(glob.glob(f"benchmark_results/json/{pair}_compare_{n}_run*.json"))
        if not files:
            continue
        summary = aggregate(files, pair)
        summary["minions"] = n
        rows.append(summary)
    if rows:
        outp = f"benchmark_results/tables/{pair}_mean_std_by_level.csv"
        with open(outp, "w", newline="", encoding="utf-8") as f:
            w = csv.DictWriter(f, fieldnames=list(rows[0].keys()))
            w.writeheader()
            w.writerows(rows)
        print("wrote", outp)
PY

python3 - <<'PY'
import glob
import json

for p in sorted(glob.glob("benchmark_results/json/*_compare_*_run*.json")):
    with open(p, "r", encoding="utf-8") as f:
        d = json.load(f)
    m = next(iter(d["marking"].values()))
    gap = abs(m["overall_a"] - m["overall_b"])
    if gap < 5.0:
        print(f"TIE-BREAK NEEDED (<5): {p}  gap={gap:.2f}")
PY

ls benchmark_runs/*.csv > benchmark_results/repro/raw_csv_appendix_list.txt

{
  echo "Date: $(date)"
  echo "Kernel: $(uname -a)"
  echo "Git commit: $(git rev-parse HEAD)"
} > benchmark_results/repro/environment.txt

echo "Analysis complete."
