# Algorithm Performance Marking Scheme

## How to Use This Scheme

For each algorithm, look at the relevant CSV columns and use the band descriptors below to assign a **percentage score (0–100%)** per factor. Then average the four factors for an **overall score**.

---

## Collision Detection: Quadtree vs Brute-Force

### CSV Columns to Use

- `collision_us_sum_1s` — total collision CPU time per second
- `fps` — frame stability
- `minion_count` / `entity_count` — scale reference

---

### Factor 1: Speed (0–100%)

*How fast does the algorithm resolve collisions per second?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | `collision_us_sum_1s` is consistently **low** (e.g. under 5,000 μs/s at 100+ minions). Resolves collisions quickly with minimal frame impact. |
| **65–84%** | Moderate collision time. Noticeable but not damaging — stays under ~15,000 μs/s at mid-range entity counts. |
| **40–64%** | Collision time climbs noticeably with entity count. Some frame pressure visible in FPS. |
| **20–39%** | High collision cost. `collision_us_sum_1s` spikes significantly even at moderate counts. |
| **0–19%** | Extremely slow. Collision time dominates the frame budget; FPS clearly suffers. |

---

### Factor 2: Scalability (0–100%)

*How well does performance hold as minion count increases?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | `collision_us_sum_1s` grows **slowly and near-linearly** as `minion_count` increases. Doubling minions does not double cost significantly. |
| **65–84%** | Cost grows but remains manageable up to ~250 minions. Growth curve is sub-quadratic. |
| **40–64%** | Cost grows noticeably faster than entity count. Clear upward curve in the data. |
| **20–39%** | Cost grows sharply — roughly quadratic pattern. Performance degrades badly above ~100 minions. |
| **0–19%** | Near-quadratic or worse growth. Completely breaks down at high entity counts. |

---

### Factor 3: CPU Efficiency (0–100%)

*How much CPU is consumed relative to the number of collisions being handled?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | Low `collision_us_sum_1s` relative to `minion_count`. Cost-per-entity remains small and stable. |
| **65–84%** | Reasonable efficiency. Small overhead per entity but some waste (e.g. tree traversal cost at low counts). |
| **40–64%** | Moderate inefficiency. CPU cost per entity is higher than expected for the scenario. |
| **20–39%** | Poor efficiency. Significant CPU spent on redundant pair checks or tree rebuilding. |
| **0–19%** | Very wasteful. Algorithm burns CPU even on simple, low-density scenarios. |

---

### Factor 4: Stability (0–100%)

*How consistent is FPS while the collision system is active?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | FPS stays **smooth and consistent** (e.g. within ±5 FPS of cap or target) across all minion counts tested. |
| **65–84%** | Mostly stable. Occasional dips but recovers quickly; no sustained drops. |
| **40–64%** | Noticeable FPS variance as entity count rises. Some sustained dips at high counts. |
| **20–39%** | Frequent FPS drops. Gameplay would feel choppy. |
| **0–19%** | Severe instability. FPS drops significantly and does not recover at moderate-to-high counts. |

---

## Pathfinding: A\* vs Dijkstra

### CSV Columns to Use

- `pathfinding_us_sum_1s` — total pathfinding CPU time per second
- `path_calls_sum_1s` — number of paths calculated per second
- `fps` — frame stability

---

### Factor 1: Speed (0–100%)

*How fast does the algorithm find paths?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | `pathfinding_us_sum_1s` is **low** relative to `path_calls_sum_1s`. Paths are resolved quickly with little CPU cost per call. |
| **65–84%** | Reasonable speed. Path time is noticeable but not a bottleneck at typical call rates. |
| **40–64%** | Slower path resolution. `pathfinding_us_sum_1s` is high relative to call count — each search is expensive. |
| **20–39%** | Slow. Path calls take significant time; search front is clearly too wide. |
| **0–19%** | Very slow. Pathfinding dominates the frame budget; FPS clearly impacted. |

---

### Factor 2: Scalability (0–100%)

*How well does performance hold as path requests and map complexity increase?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | `pathfinding_us_sum_1s` grows **slowly** even as `path_calls_sum_1s` increases. Scales well on the maze map with mud terrain. |
| **65–84%** | Scales acceptably. Some increase in cost at high call rates but remains usable. |
| **40–64%** | Noticeable degradation as calls increase. Cost per call rises on complex terrain. |
| **20–39%** | Poor scaling. Each additional path request adds disproportionate cost — wide search fronts on maze. |
| **0–19%** | Breaks down under load. Pathfinding stalls or massively delays other systems. |

---

### Factor 3: CPU Efficiency (0–100%)

*How efficiently does the algorithm use CPU relative to paths resolved?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | Low `pathfinding_us_sum_1s` per call (`pathfinding_us_sum_1s / path_calls_sum_1s`). Heuristic (A\*) or expansion strategy is effective. |
| **65–84%** | Reasonable cost per call. Some unnecessary node expansions but manageable. |
| **40–64%** | Higher cost per call than expected. Algorithm explores more nodes than needed. |
| **20–39%** | Poor efficiency. Large search fronts or redundant exploration visible in high μs-per-call. |
| **0–19%** | Extremely inefficient. Near-complete graph exploration even for short paths. |

---

### Factor 4: Stability (0–100%)

*How consistent is FPS while pathfinding is active?*

| Score | Descriptor |
|-------|-----------|
| **85–100%** | FPS remains **stable** even during bursts of path requests. PathfindBudget spreading load is effective. |
| **65–84%** | Mostly stable. Short dips during large request bursts but generally smooth. |
| **40–64%** | Visible FPS dips when many paths are requested simultaneously. |
| **20–39%** | Frequent FPS instability tied to pathfinding load. |
| **0–19%** | Severe drops. Pathfinding freezes or near-freezes the game during active searches. |

---

## Scoring Summary Template

Once you have scored each factor from your CSV data, fill in this table:

| Algorithm | Speed | Scalability | CPU Efficiency | Stability | **Overall (avg)** |
|-----------|-------|-------------|----------------|-----------|-------------------|
| Quadtree | | | | | |
| Brute-Force | | | | | |
| A\* | | | | | |
| Dijkstra | | | | | |

---

## Tips for Assigning Scores Fairly

- **Always compare the same map and minion count** between the two algorithms in each pair — don't score Quadtree at 500 minions against Brute-Force at 50.
- **Discard the first 5–10 seconds** of each CSV run (warm-up period) before averaging rows.
- **Calculate per-call cost** for pathfinding as `pathfinding_us_sum_1s ÷ path_calls_sum_1s` per row — this is more meaningful than raw totals when call counts differ between runs.
- **Score relative to the other algorithm in the pair**, not against an absolute ideal — the goal is to show which performs better and by how much.
