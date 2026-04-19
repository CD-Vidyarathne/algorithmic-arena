# Maps for Performance Analysis

This document plans **which maps to use or generate** so collision and pathfinding can be **measured, compared, and interpreted** in the report and viva. It aligns with the existing `maps/` assets and the engine’s behaviours (128×128 committed maps for small repo size, deploy flags, weighted terrain, `PathfindBudget`, CSV columns).

**Related:** [12_marking_scheme.md](12_marking_scheme.md), [7_Implementation_Plan_0302.md](7_Implementation_Plan_0302.md) §9g.

---

## 1. What each algorithm needs to show

| System | Primary stress | Map shape | Typical metric |
|--------|----------------|-----------|----------------|
| **Collision** (brute vs quadtree) | Many pairwise overlaps; **clustering** hurts brute-force and can stress quadtree buckets | Open interior with room to swarm; optional choke to force clustering | `collision_us_sum_1s`, FPS, `entity_count` / `minion_count` |
| **Pathfinding** (Dijkstra vs A\*) | Large **search front**; **weighted** tiles (mud vs grass) make A\*’s heuristic advantage visible | Long maze corridors; mud patches; flags far from spawn | `pathfinding_us_sum_1s`, `path_calls_sum_1s` |
| **Combined** (realistic) | Both systems busy | Mixed terrain + maze + many units | All CSV fields; narrative “game-like” stress |

---

## 2. Canonical map set (already in repo)

These three are the **minimum** set for a defensible evaluation. They are generated/committed under `maps/` (see `scripts/generate_maps.py` if present).

| Map file | Role | Why it isolates an effect |
|----------|------|---------------------------|
| **`benchmark_open_128.map`** | **Collision benchmark** | Grass interior, few obstacles; **deploy zone** + **2 flags**. Ideal for: spawn many minions, order movement, **high entity density** and **collision-heavy** frames without pathfinding dominating every cell. |
| **`benchmark_maze_128.map`** | **Pathfinding benchmark** | **Dense tree maze**, **mud** in corridors, **multiple flags** at far corners. **Weighted costs** → Dijkstra explores broadly; A\* should show lower `pathfinding_us_sum_1s` and often fewer expanded nodes (when F2 is enabled for qualitative screenshots only). |
| **`nexus_siege_128.map`** | **Integrated / narrative** | **Maze + deploy + multiple flags** → representative “full game” load for poster screenshots and discussion of **combined** cost. |

---

## 3. How to run each map for fair comparisons

### 3.1 Collision (brute-force vs quadtree)

- **Map:** `benchmark_open_128.map` (primary).
- **Builds:** `USE_QUADTREE_COLLISION=ON` vs `OFF`; **same** `USE_ASTAR_PATHFINDING` for both (pick one, e.g. ON).
- **Procedure:**  
  - Cap or uncapped FPS: **document** which (`--unlimited-fps` vs 60 cap).  
  - **Bulk spawn** or spawn to target **minion counts** (e.g. 50, 100, 250, 500) on the **same** map.  
  - Keep **F1/F2 off** for numeric CSV unless you add a separate “viz cost” row.  
  - Log **≥30–60 s** of CSV or multiple runs; average `collision_us_sum_1s` and FPS.

**What to plot:** minion count (x) vs collision μs sum per second (y); two series (brute vs quad).

### 3.2 Pathfinding (Dijkstra vs A\*)

- **Map:** `benchmark_maze_128.map` (primary).
- **Builds:** `USE_ASTAR_PATHFINDING=ON` vs `OFF` (Dijkstra); **same** collision mode for both (e.g. quadtree ON).
- **Procedure:**  
  - Issue **orders** that trigger many `findPath` calls (e.g. order all to far flags, or use **O** / right-click after bulk spawn).  
  - **PathfindBudget** spreads work across frames—report **sums over 1 s** (already in CSV), not single-frame spikes.  
  - For “explored nodes” narrative, use **F2** only for **screenshots**, not for the main timed CSV.

**What to plot:** scenario label vs `pathfinding_us_sum_1s` and/or `path_calls_sum_1s`; two series (Dijkstra vs A\*).

### 3.3 Combined story

- **Map:** `nexus_siege_128.map`.
- **Use:** One **table** or **bar chart** of FPS + collision + path sums at a **fixed** minion count; compare **collision** builds on one row and **pathfinding** builds on another (do not change two variables at once in a single series).

---

## 4. Optional extra maps (generate only if needed)

If you need **longer paths** or a **clustering choke** not visible on 128×128:

| Suggested name | Size | Purpose |
|----------------|------|---------|
| Regenerate via `scripts/generate_maps.py` with larger `W`/`H` | e.g. 512×512 | Heavier pathfinding / collision stress; larger `.map` files |
| `benchmark_cluster_512.map` (if added) | 512×512 | **Narrow corridor** from deploy to a single interior flag to force **clustering**; amplifies brute vs quadtree gap |

**Only add** these if the three canonical maps do not separate the algorithms clearly in your data.

---

## 5. Map header fields (consistency)

For every benchmark `.map`, keep the **same** `# tile_size=...` as production (e.g. 32) unless you are explicitly studying tile resolution. **Minion cap** in the header should be **≥** your max benchmark spawn so you are not clipping the experiment.

---

## 6. Report checklist (maps ↔ claims)

- [ ] Each **graph** in Chapter 5 names **map file**, **build flags**, **minion count**, and **FPS mode** (capped vs uncapped).
- [ ] **Collision** comparison uses **open** or **cluster** map; **pathfinding** comparison uses **maze** (or clearly justified alternative).
- [ ] **Integrated** claim (optional) references `nexus_siege_128.map` or equivalent.

---

## 7. Revision log

| Date | Change |
|------|--------|
| (add as you edit) | |
