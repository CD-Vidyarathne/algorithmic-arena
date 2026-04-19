#!/usr/bin/env python3
"""
Analyze benchmark CSV files produced by Algorithmic Arena (--csv <path>).

Aligned with:
  - docs/Algorithm_Performance_Marking_Scheme.md (factors, CSV columns, pairwise scoring)
  - docs/Performance Analyze.md (warmup, derived metrics, §6–§8 conclusion workflow)

CSV header (1 Hz rows):
  timestamp_s,fps,entity_count,collision_us_sum_1s,pathfinding_us_sum_1s,
  path_calls_sum_1s,minion_count

Examples:
  python3 scripts/analyze_benchmark_csv.py analyze benchmark_runs/collision_quad_open128.csv

  python3 scripts/analyze_benchmark_csv.py compare \\
      benchmark_runs/quad.csv benchmark_runs/brute.csv \\
      --labels Quadtree BruteForce --focus collision

  python3 scripts/analyze_benchmark_csv.py compare astar.csv dijkstra.csv \\
      --labels AStar Dijkstra --focus pathfinding
"""

from __future__ import annotations

import argparse
import csv
import json
import math
import statistics
import sys
from dataclasses import dataclass, asdict
from pathlib import Path
from typing import Any, Sequence

# --- Marking scheme (docs/Algorithm_Performance_Marking_Scheme.md) -----------------
# Pairwise scores use relative comparison between two runs at the same map/load
# (scheme: "Score relative to the other algorithm in the pair").
# Scalability needs multiple minion levels (Performance Analyze §4.1–4.2); single CSVs
# cannot show growth curves — we report a placeholder and a clear note.
# ---------------------------------------------------------------------------------


REQUIRED_COLUMNS = (
    "timestamp_s",
    "fps",
    "entity_count",
    "collision_us_sum_1s",
    "pathfinding_us_sum_1s",
    "path_calls_sum_1s",
    "minion_count",
)


@dataclass
class Row:
    timestamp_s: float
    fps: float
    entity_count: int
    collision_us_sum_1s: float
    pathfinding_us_sum_1s: float
    path_calls_sum_1s: int
    minion_count: int


def load_rows(path: Path) -> list[Row]:
    with path.open(newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        if reader.fieldnames is None:
            raise ValueError(f"Empty or invalid CSV: {path}")
        missing = [c for c in REQUIRED_COLUMNS if c not in reader.fieldnames]
        if missing:
            raise ValueError(f"Missing columns {missing} in {path}")
        rows: list[Row] = []
        for raw in reader:
            rows.append(
                Row(
                    timestamp_s=float(raw["timestamp_s"]),
                    fps=float(raw["fps"]),
                    entity_count=int(float(raw["entity_count"])),
                    collision_us_sum_1s=float(raw["collision_us_sum_1s"]),
                    pathfinding_us_sum_1s=float(raw["pathfinding_us_sum_1s"]),
                    path_calls_sum_1s=int(float(raw["path_calls_sum_1s"])),
                    minion_count=int(float(raw["minion_count"])),
                )
            )
    return rows


def apply_warmup(rows: Sequence[Row], warmup_s: float) -> list[Row]:
    if not rows:
        return []
    return [r for r in rows if r.timestamp_s >= warmup_s]


def mean(xs: Sequence[float]) -> float | None:
    if not xs:
        return None
    return sum(xs) / len(xs)


def safe_stdev(xs: Sequence[float]) -> float | None:
    if len(xs) < 2:
        return None
    return statistics.stdev(xs)


def aggregate_path_us_per_call(rows: Sequence[Row]) -> float | None:
    sp = sum(r.pathfinding_us_sum_1s for r in rows)
    sc = sum(r.path_calls_sum_1s for r in rows)
    if sc <= 0:
        return None
    return sp / sc


def aggregate_collision_us_per_minion(rows: Sequence[Row]) -> float | None:
    sc = sum(r.collision_us_sum_1s for r in rows)
    sm = sum(r.minion_count for r in rows)
    if sm <= 0:
        return None
    return sc / sm


def mean_path_us_per_row(steady: Sequence[Row]) -> float | None:
    """Scheme: per-row pathfinding_us_sum_1s ÷ path_calls_sum_1s where calls > 0."""
    ratios: list[float] = []
    for r in steady:
        if r.path_calls_sum_1s > 0:
            ratios.append(r.pathfinding_us_sum_1s / r.path_calls_sum_1s)
    return mean(ratios) if ratios else None


def pairwise_scores_lower_is_better(va: float, vb: float) -> tuple[float, float]:
    """Return (score_a, score_b) in [0,100], sum 100; lower metric is better."""
    if va < 0 or vb < 0:
        return 50.0, 50.0
    s = va + vb
    if s <= 0:
        return 50.0, 50.0
    return 100.0 * vb / s, 100.0 * va / s


def pairwise_scores_higher_is_better(va: float, vb: float) -> tuple[float, float]:
    """Return (score_a, score_b) in [0,100], sum 100; higher metric is better."""
    s = va + vb
    if s <= 0:
        return 50.0, 50.0
    return 100.0 * va / s, 100.0 * vb / s


def stability_scores(
    mean_fps_a: float,
    mean_fps_b: float,
    spread_a: float,
    spread_b: float,
) -> tuple[float, float]:
    """
    Stability (scheme): smooth FPS — higher mean FPS is better; lower FPS spread is better.
    Average the two pairwise splits for each side.
    """
    sm_a, sm_b = pairwise_scores_higher_is_better(mean_fps_a, mean_fps_b)
    # Avoid div by zero if no spread
    if spread_a <= 0 and spread_b <= 0:
        return sm_a, sm_b
    ss_a, ss_b = pairwise_scores_lower_is_better(spread_a, spread_b)
    return (sm_a + ss_a) / 2.0, (sm_b + ss_b) / 2.0


def band_collision_speed_abs(mean_collision_us: float, mean_minions: float) -> str:
    """Rough absolute band from Algorithm_Performance_Marking_Scheme (collision speed)."""
    if mean_minions < 50:
        return "absolute band n/a (mean minions < 50 — scheme bands target 100+ minions)"
    if mean_minions >= 100:
        if mean_collision_us < 5000:
            return "≈ 85–100% band (scheme: collision_us_sum_1s low at 100+ minions)"
        if mean_collision_us < 15000:
            return "≈ 65–84% band (moderate cost)"
        if mean_collision_us < 35000:
            return "≈ 40–64% band (noticeable climb)"
        if mean_collision_us < 80000:
            return "≈ 20–39% band (high cost)"
        return "≈ 0–19% band (collision dominates)"
    # 50–99 minions: softer interpretation
    if mean_collision_us < 8000:
        return "moderate (use 100+ minions for scheme-aligned absolute bands)"
    if mean_collision_us < 25000:
        return "elevated collision cost"
    return "high collision cost"


def band_path_speed_abs(us_per_call_agg: float | None) -> str:
    """Qualitative note from aggregate µs/call (scheme: speed vs path_calls_sum_1s)."""
    if us_per_call_agg is None:
        return "n/a (no path calls in steady window)"
    if us_per_call_agg < 80:
        return "≈ 85–100% band (path cost low vs call rate)"
    if us_per_call_agg < 200:
        return "≈ 65–84% band (reasonable)"
    if us_per_call_agg < 450:
        return "≈ 40–64% band (expensive per call)"
    if us_per_call_agg < 900:
        return "≈ 20–39% band (slow searches)"
    return "≈ 0–19% band (pathfinding bottleneck)"


SCALABILITY_PLACEHOLDER = 50.0
SCALABILITY_NOTE = (
    "Scalability held at 50% each — measure growth across minion levels "
    "(Performance Analyze §4.1–4.2); feed multiple CSVs into your report sheet."
)


def compute_collision_pair_scores(a: Summary, b: Summary) -> dict[str, Any]:
    """
    Algorithm_Performance_Marking_Scheme.md — Collision pair.
    Speed: mean collision_us_sum_1s (lower better).
    CPU efficiency: collision µs per minion aggregate (lower better).
    Stability: mean FPS (higher) + FPS spread (lower).
    Scalability: not derivable from one load level — placeholder.
    """
    if a.mean_collision_us_sum_1s is None or b.mean_collision_us_sum_1s is None:
        sp_a, sp_b = 50.0, 50.0
    else:
        sp_a, sp_b = pairwise_scores_lower_is_better(
            a.mean_collision_us_sum_1s, b.mean_collision_us_sum_1s
        )

    if a.collision_us_per_minion_agg is None or b.collision_us_per_minion_agg is None:
        cpu_a, cpu_b = 50.0, 50.0
    else:
        cpu_a, cpu_b = pairwise_scores_lower_is_better(
            a.collision_us_per_minion_agg, b.collision_us_per_minion_agg
        )

    if a.mean_fps is None or b.mean_fps is None:
        st_a, st_b = 50.0, 50.0
    else:
        sa = float(a.fps_spread) if a.fps_spread is not None else 0.0
        sb = float(b.fps_spread) if b.fps_spread is not None else 0.0
        st_a, st_b = stability_scores(a.mean_fps, b.mean_fps, sa, sb)

    sc_a, sc_b = SCALABILITY_PLACEHOLDER, SCALABILITY_PLACEHOLDER
    oa = (sp_a + sc_a + cpu_a + st_a) / 4.0
    ob = (sp_b + sc_b + cpu_b + st_b) / 4.0

    return {
        "pair": "collision",
        "speed_a": sp_a,
        "speed_b": sp_b,
        "scalability_a": sc_a,
        "scalability_b": sc_b,
        "cpu_efficiency_a": cpu_a,
        "cpu_efficiency_b": cpu_b,
        "stability_a": st_a,
        "stability_b": st_b,
        "overall_a": oa,
        "overall_b": ob,
        "scalability_note": SCALABILITY_NOTE,
    }


def compute_pathfinding_pair_scores(a: Summary, b: Summary) -> dict[str, Any]:
    """
    Algorithm_Performance_Marking_Scheme.md — Pathfinding pair.
    Speed: mean pathfinding_us_sum_1s (lower better).
    CPU efficiency: aggregate µs/call (lower better).
    Stability: same as collision.
    Scalability: placeholder.
    """
    if a.mean_pathfinding_us_sum_1s is None or b.mean_pathfinding_us_sum_1s is None:
        sp_a, sp_b = 50.0, 50.0
    else:
        sp_a, sp_b = pairwise_scores_lower_is_better(
            a.mean_pathfinding_us_sum_1s, b.mean_pathfinding_us_sum_1s
        )

    if a.path_us_per_call_agg is None or b.path_us_per_call_agg is None:
        cpu_a, cpu_b = 50.0, 50.0
    else:
        cpu_a, cpu_b = pairwise_scores_lower_is_better(
            a.path_us_per_call_agg, b.path_us_per_call_agg
        )

    if a.mean_fps is None or b.mean_fps is None:
        st_a, st_b = 50.0, 50.0
    else:
        sa = float(a.fps_spread) if a.fps_spread is not None else 0.0
        sb = float(b.fps_spread) if b.fps_spread is not None else 0.0
        st_a, st_b = stability_scores(a.mean_fps, b.mean_fps, sa, sb)

    sc_a, sc_b = SCALABILITY_PLACEHOLDER, SCALABILITY_PLACEHOLDER
    oa = (sp_a + sc_a + cpu_a + st_a) / 4.0
    ob = (sp_b + sc_b + cpu_b + st_b) / 4.0

    return {
        "pair": "pathfinding",
        "speed_a": sp_a,
        "speed_b": sp_b,
        "scalability_a": sc_a,
        "scalability_b": sc_b,
        "cpu_efficiency_a": cpu_a,
        "cpu_efficiency_b": cpu_b,
        "stability_a": st_a,
        "stability_b": st_b,
        "overall_a": oa,
        "overall_b": ob,
        "scalability_note": SCALABILITY_NOTE,
    }


def print_marking_table(
    label_a: str,
    label_b: str,
    m: dict[str, Any],
    title: str,
) -> None:
    """Performance Analyze §6 / §10 style table."""
    print(f"  --- {title} ---")
    hdr = f"  {'Algorithm':<22} {'Speed':>8} {'Scale':>8} {'CPU':>8} {'Stab.':>8} {'Overall':>8}"
    print(hdr)
    print(
        f"  {label_a:<22} {m['speed_a']:>7.1f}% {m['scalability_a']:>7.1f}% "
        f"{m['cpu_efficiency_a']:>7.1f}% {m['stability_a']:>7.1f}% {m['overall_a']:>7.1f}%"
    )
    print(
        f"  {label_b:<22} {m['speed_b']:>7.1f}% {m['scalability_b']:>7.1f}% "
        f"{m['cpu_efficiency_b']:>7.1f}% {m['stability_b']:>7.1f}% {m['overall_b']:>7.1f}%"
    )
    print(f"  Note: {m['scalability_note']}")
    print()


def winner_by_overall(
    oa: float,
    ob: float,
    la: str,
    lb: str,
) -> tuple[str, str]:
    """Returns (winner_label, tie_note). tie_note empty unless within 5 points (Performance Analyze §6.3)."""
    if abs(oa - ob) < 5.0:
        w = la if oa > ob else lb if ob > oa else "tie"
        return w, (
            "Overall scores within 5 points — use factor detail (esp. scalability from "
            "multi-minion runs) per Performance Analyze §6.3."
        )
    if oa > ob:
        return la, ""
    if ob > oa:
        return lb, ""
    return "tie", ""


@dataclass
class Summary:
    file: str
    rows_total: int
    rows_used: int
    warmup_seconds: float
    time_span_s: float | None
    mean_fps: float | None
    stdev_fps: float | None
    fps_spread: float | None
    mean_entity_count: float | None
    mean_minion_count: float | None
    mean_collision_us_sum_1s: float | None
    mean_pathfinding_us_sum_1s: float | None
    mean_path_calls_sum_1s: float | None
    path_us_per_call_agg: float | None
    mean_path_us_per_row: float | None
    collision_us_per_minion_agg: float | None


def summarize(path: Path, rows: Sequence[Row], warmup_s: float) -> Summary:
    total = len(rows)
    steady = apply_warmup(rows, warmup_s)
    used = len(steady)
    if not steady:
        return Summary(
            str(path),
            total,
            0,
            warmup_s,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
            None,
        )

    ts = [r.timestamp_s for r in steady]
    time_span = max(ts) - min(ts) if len(ts) > 1 else 0.0
    fps_list = [r.fps for r in steady]
    spread = max(fps_list) - min(fps_list) if fps_list else None

    return Summary(
        file=str(path),
        rows_total=total,
        rows_used=used,
        warmup_seconds=warmup_s,
        time_span_s=time_span,
        mean_fps=mean(fps_list),
        stdev_fps=safe_stdev(fps_list),
        fps_spread=spread,
        mean_entity_count=mean([float(r.entity_count) for r in steady]),
        mean_minion_count=mean([float(r.minion_count) for r in steady]),
        mean_collision_us_sum_1s=mean([r.collision_us_sum_1s for r in steady]),
        mean_pathfinding_us_sum_1s=mean([r.pathfinding_us_sum_1s for r in steady]),
        mean_path_calls_sum_1s=mean([float(r.path_calls_sum_1s) for r in steady]),
        path_us_per_call_agg=aggregate_path_us_per_call(steady),
        mean_path_us_per_row=mean_path_us_per_row(steady),
        collision_us_per_minion_agg=aggregate_collision_us_per_minion(steady),
    )


def pct_change(old: float, new: float) -> float:
    if old == 0:
        return math.copysign(float("inf"), new) if new != 0 else 0.0
    return (new - old) / old * 100.0


def fmt_pct(x: float | None) -> str:
    if x is None:
        return "n/a"
    if math.isinf(x):
        return "inf"
    return f"{x:+.1f}%"


def fmt_num(x: float | None, nd: int = 2) -> str:
    if x is None:
        return "n/a"
    return f"{x:.{nd}f}"


def print_single(s: Summary) -> None:
    print()
    print("=" * 72)
    print(" BENCHMARK SUMMARY (steady window after warmup)")
    print("=" * 72)
    print(f"  File:              {s.file}")
    print(f"  Rows (total/used): {s.rows_total} / {s.rows_used}  (warmup {s.warmup_seconds:g}s discarded)")
    if s.rows_used == 0:
        print("  No data after warmup — extend the run or lower --warmup-seconds.")
        print("=" * 72)
        return
    print(f"  Time span (used):  {fmt_num(s.time_span_s, 1)} s")
    print()
    print("  --- Rates (lower cost is better unless noted) ---")
    print(f"  Mean FPS:                    {fmt_num(s.mean_fps, 2)}  (stdev {fmt_num(s.stdev_fps, 2)})")
    print(f"  FPS spread (max−min):       {fmt_num(s.fps_spread, 2)}")
    print(f"  Mean entity_count:           {fmt_num(s.mean_entity_count, 1)}")
    print(f"  Mean minion_count:           {fmt_num(s.mean_minion_count, 1)}")
    print(f"  Mean collision_us_sum_1s:    {fmt_num(s.mean_collision_us_sum_1s, 2)}")
    print(f"  Mean pathfinding_us_sum_1s: {fmt_num(s.mean_pathfinding_us_sum_1s, 2)}")
    print(f"  Mean path_calls_sum_1s:     {fmt_num(s.mean_path_calls_sum_1s, 2)}")
    print()
    print("  --- Aggregated derived metrics (whole steady window) ---")
    print(
        "  Path µs / path call:        "
        f"{fmt_num(s.path_us_per_call_agg, 2)}  "
        "(Σ pathfinding / Σ path_calls)"
    )
    print(
        "  Collision µs / minion·s:    "
        f"{fmt_num(s.collision_us_per_minion_agg, 2)}  "
        "(Σ collision / Σ minion_count; Performance Analyze §2.2)"
    )
    print(
        "  Path µs/call (mean of rows): "
        f"{fmt_num(s.mean_path_us_per_row, 2)}  "
        "(scheme: per-row pathfinding ÷ path_calls where calls > 0)"
    )
    print("=" * 72)
    print_outcome_single(s)
    print()


def print_outcome_single(s: Summary) -> None:
    if s.rows_used == 0:
        return
    print()
    print("  MARKING SCHEME — absolute hints (single run; pairwise compare is authoritative)")
    if s.mean_collision_us_sum_1s is not None and s.mean_minion_count is not None:
        print(
            f"  Collision speed: {band_collision_speed_abs(s.mean_collision_us_sum_1s, s.mean_minion_count)}"
        )
    if s.path_us_per_call_agg is not None:
        print(f"  Pathfinding speed: {band_path_speed_abs(s.path_us_per_call_agg)}")
    print()
    print("  OUTCOME (single run — use `compare` for Quadtree vs Brute or A* vs Dijkstra scores)")
    if s.mean_fps is not None and s.mean_fps >= 55:
        print(f"  • Frame rate looks healthy (mean FPS {s.mean_fps:.1f}).")
    elif s.mean_fps is not None and s.mean_fps < 30:
        print(f"  • Frame rate is low (mean FPS {s.mean_fps:.1f}) — check load or cap settings.")
    if s.fps_spread is not None and s.fps_spread > 15:
        print(f"  • FPS is volatile (spread {s.fps_spread:.1f}) — consider longer runs or stable load.")
    if s.path_us_per_call_agg is not None and s.path_us_per_call_agg > 500:
        print("  • Path cost per call is high — pathfinding may dominate; maze-style maps amplify this.")
    if s.collision_us_per_minion_agg is not None and s.collision_us_per_minion_agg > 200:
        print("  • Collision cost per minion is high — open-field / many units stress collision.")
    print()
    print(
        "  Scalability (scheme factor 2) needs several CSVs across minion levels "
        "(Performance Analyze §4.1–4.2) — not computable from one file alone."
    )


def compare_summaries(
    a: Summary,
    b: Summary,
    label_a: str,
    label_b: str,
    focus: str,
) -> None:
    print()
    print("=" * 72)
    print(" PAIRWISE COMPARISON")
    print("=" * 72)
    print(f"  A: {label_a}  ←  {a.file}")
    print(f"  B: {label_b}  ←  {b.file}")
    print(f"  Focus: {focus}")
    if a.rows_used == 0 or b.rows_used == 0:
        print("  One side has no post-warmup rows — cannot compare.")
        print("=" * 72)
        return

    def row(name: str, va: float | None, vb: float | None, lower_is_better: bool) -> None:
        if va is None or vb is None:
            print(f"  {name}: n/a")
            return
        diff = pct_change(va, vb)
        better = None
        if lower_is_better:
            better = label_b if vb < va else label_a if va < vb else "tie"
        else:
            better = label_b if vb > va else label_a if va > vb else "tie"
        sense = "lower is better" if lower_is_better else "higher is better"
        print(f"  {name}:")
        print(f"    {label_a}: {fmt_num(va, 2)}")
        print(f"    {label_b}: {fmt_num(vb, 2)}  ({fmt_pct(diff)} vs A, {sense})")
        print(f"    → Favourable: {better}")

    print()
    print("  --- Headline metrics ---")
    row("Mean FPS", a.mean_fps, b.mean_fps, lower_is_better=False)
    row("Mean collision_us_sum_1s", a.mean_collision_us_sum_1s, b.mean_collision_us_sum_1s, lower_is_better=True)
    row("Mean pathfinding_us_sum_1s", a.mean_pathfinding_us_sum_1s, b.mean_pathfinding_us_sum_1s, lower_is_better=True)
    row("Path µs/call (aggregate)", a.path_us_per_call_agg, b.path_us_per_call_agg, lower_is_better=True)
    row("Collision µs/minion (aggregate)", a.collision_us_per_minion_agg, b.collision_us_per_minion_agg, lower_is_better=True)

    print()
    col_m: dict[str, Any] | None = None
    path_m: dict[str, Any] | None = None
    if focus in ("collision", "both"):
        col_m = compute_collision_pair_scores(a, b)
        print_marking_table(
            label_a,
            label_b,
            col_m,
            "MARKING SCHEME — Collision pair (relative 0–100%, Performance Analyze §6.1)",
        )
    if focus in ("pathfinding", "both"):
        path_m = compute_pathfinding_pair_scores(a, b)
        print_marking_table(
            label_a,
            label_b,
            path_m,
            "MARKING SCHEME — Pathfinding pair (relative 0–100%, Performance Analyze §6.1)",
        )

    print("=" * 72)
    print_verdict(a, b, label_a, label_b, focus, col_m, path_m)
    print("=" * 72)
    print()


def print_verdict(
    a: Summary,
    b: Summary,
    la: str,
    lb: str,
    focus: str,
    collision_m: dict[str, Any] | None,
    pathfinding_m: dict[str, Any] | None,
) -> None:
    """Metric winners + Performance Analyze §6.3 / §8 style conclusions."""

    def winner_cost(va: float | None, vb: float | None) -> str | None:
        if va is None or vb is None:
            return None
        if va < vb:
            return la
        if vb < va:
            return lb
        return None

    def winner_fps(va: float | None, vb: float | None) -> str | None:
        if va is None or vb is None:
            return None
        if va > vb:
            return la
        if vb > va:
            return lb
        return None

    lines: list[str] = []

    if focus in ("collision", "both"):
        wc = winner_cost(a.mean_collision_us_sum_1s, b.mean_collision_us_sum_1s)
        wf = winner_fps(a.mean_fps, b.mean_fps)
        cm = winner_cost(a.collision_us_per_minion_agg, b.collision_us_per_minion_agg)
        if wc:
            lines.append(f"  COLLISION (mean µs sum / s): favourable → {wc}")
        if cm:
            lines.append(f"  COLLISION (µs per minion aggregate): favourable → {cm}")
        if wf:
            lines.append(f"  FPS (mean): favourable → {wf}")
        if not any([wc, cm, wf]):
            lines.append(
                "  COLLISION metrics: no clear winner (tied or n/a on compared fields)."
            )

    if focus in ("pathfinding", "both"):
        wp = winner_cost(a.mean_pathfinding_us_sum_1s, b.mean_pathfinding_us_sum_1s)
        wpc = winner_cost(a.path_us_per_call_agg, b.path_us_per_call_agg)
        wf = winner_fps(a.mean_fps, b.mean_fps)
        if wp:
            lines.append(f"  PATHFINDING (mean µs sum / s): favourable → {wp}")
        if wpc:
            lines.append(f"  PATHFINDING (µs per call aggregate): favourable → {wpc}")
        if wf and focus == "pathfinding":
            lines.append(f"  FPS (mean): favourable → {wf}")
        if focus == "pathfinding" and not any([wp, wpc]):
            lines.append(
                "  PATHFINDING metrics: no clear winner (tied or n/a on compared fields)."
            )

    if not lines:
        lines.append("  No verdict — check CSV contents and --focus.")

    print("  OUTCOME (raw metric winners)")
    for line in lines:
        print(line)

    print()
    print("  CONCLUSION (Performance Analyze §6.2–§6.3, §8)")
    if collision_m is not None:
        w, tie_note = winner_by_overall(
            collision_m["overall_a"], collision_m["overall_b"], la, lb
        )
        print(
            f"  Collision pair overall (avg of 4 factors): "
            f"{la} {collision_m['overall_a']:.1f}% vs {lb} {collision_m['overall_b']:.1f}% "
            f"→ higher is better → {w}"
        )
        if tie_note:
            print(f"  {tie_note}")
        if w == "tie":
            print(
                "  Report template §8.1: Overall tied — apply Performance Analyze §6.3 "
                "(factor-level tie-break, especially scalability from multi-minion CSVs)."
            )
        else:
            print(
                f"  Report template §8.1: On benchmark_open_128.map (same map for both builds), "
                f"{w} can be argued as the collision winner when overall is higher with "
                f"lower mean collision_us_sum_1s and stable FPS; support scalability with "
                f"extra CSVs at 50–500 minions."
            )
    if pathfinding_m is not None:
        w, tie_note = winner_by_overall(
            pathfinding_m["overall_a"], pathfinding_m["overall_b"], la, lb
        )
        print(
            f"  Pathfinding pair overall (avg of 4 factors): "
            f"{la} {pathfinding_m['overall_a']:.1f}% vs {lb} {pathfinding_m['overall_b']:.1f}% "
            f"→ higher is better → {w}"
        )
        if tie_note:
            print(f"  {tie_note}")
        if w == "tie":
            print(
                "  Report template §8.2: Overall tied — use §6.3 (µs/call, FPS stability) "
                "and repeated runs on benchmark_maze_128.map."
            )
        else:
            print(
                f"  Report template §8.2: On benchmark_maze_128.map, cite lower pathfinding µs/call "
                f"and pathfinding µs sum for {w}; tie-break with FPS stability if overall is close."
            )


def summary_to_json(s: Summary) -> dict[str, Any]:
    d = asdict(s)
    for k, v in d.items():
        if isinstance(v, float) and (math.isnan(v) or math.isinf(v)):
            d[k] = None
    return d


def cmd_analyze(args: argparse.Namespace) -> int:
    path = Path(args.csv)
    if not path.is_file():
        print(f"Error: file not found: {path}", file=sys.stderr)
        return 1
    rows = load_rows(path)
    s = summarize(path, rows, args.warmup_seconds)
    if args.json:
        print(json.dumps(summary_to_json(s), indent=2))
    else:
        print_single(s)
    return 0


def cmd_compare(args: argparse.Namespace) -> int:
    pa, pb = Path(args.csv_a), Path(args.csv_b)
    for p in (pa, pb):
        if not p.is_file():
            print(f"Error: file not found: {p}", file=sys.stderr)
            return 1
    la, lb = args.labels[0], args.labels[1]
    sa = summarize(pa, load_rows(pa), args.warmup_seconds)
    sb = summarize(pb, load_rows(pb), args.warmup_seconds)
    if args.json:
        payload: dict[str, Any] = {
            "label_a": la,
            "label_b": lb,
            "focus": args.focus,
            "summary_a": summary_to_json(sa),
            "summary_b": summary_to_json(sb),
            "marking": {},
        }
        if args.focus in ("collision", "both"):
            payload["marking"]["collision"] = compute_collision_pair_scores(sa, sb)
        if args.focus in ("pathfinding", "both"):
            payload["marking"]["pathfinding"] = compute_pathfinding_pair_scores(sa, sb)
        print(json.dumps(payload, indent=2))
    else:
        compare_summaries(sa, sb, la, lb, args.focus)
    return 0


def build_parser() -> argparse.ArgumentParser:
    p = argparse.ArgumentParser(
        description="Analyze Algorithmic Arena benchmark CSV files.",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog=__doc__,
    )
    p.add_argument(
        "--warmup-seconds",
        type=float,
        default=7.0,
        help="Ignore CSV rows with timestamp_s below this (default: 7).",
    )
    p.add_argument(
        "--json",
        action="store_true",
        help="Emit machine-readable JSON instead of text tables.",
    )

    sub = p.add_subparsers(dest="command", required=True)

    pa = sub.add_parser("analyze", help="Summarize a single CSV run.")
    pa.add_argument("csv", type=str, help="Path to benchmark .csv")
    pa.set_defaults(func=cmd_analyze)

    pc = sub.add_parser("compare", help="Compare two CSV runs (same map/load recommended).")
    pc.add_argument("csv_a", type=str, help="First run (e.g. baseline)")
    pc.add_argument("csv_b", type=str, help="Second run (e.g. candidate)")
    pc.add_argument(
        "--labels",
        nargs=2,
        metavar=("A", "B"),
        required=True,
        help="Human names for csv_a and csv_b (e.g. Quadtree BruteForce)",
    )
    pc.add_argument(
        "--focus",
        choices=("collision", "pathfinding", "both"),
        default="both",
        help="Which OUTCOME block to emphasise (default: both).",
    )
    pc.set_defaults(func=cmd_compare)

    return p


def main(argv: Sequence[str] | None = None) -> int:
    argv = list(sys.argv[1:] if argv is None else argv)
    parser = build_parser()

    # Backward-compatible: `script.py file.csv` → analyze
    if argv and not argv[0].startswith("-") and "analyze" not in argv and "compare" not in argv:
        argv = ["analyze"] + argv

    args = parser.parse_args(argv)
    return int(args.func(args))


if __name__ == "__main__":
    raise SystemExit(main())
