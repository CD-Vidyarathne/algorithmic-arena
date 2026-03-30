# Identified Issues and Solutions

This note records gameplay and engine problems that showed up during stress testing (hundreds of minions, sustained pathfinding) and the fixes applied in code. It is meant to stay aligned with the implementation so future changes do not reintroduce the same bottlenecks.

---

## 1. Severe slowdown or apparent freeze with bulk spawns (on the order of 500+ entities)

### Symptom

Spawning many minions at once made the frame time spike badly or the application feel stuck.

### Root cause

Each `Minion` instance loaded `../assets/Characters/Minion/minion_up.png` from disk in its constructor and owned a dedicated `sf::Texture`. Bulk spawn therefore performed hundreds of synchronous loads and duplicated the same texture in memory many times.

### Solution

- Use a **single shared texture** for all minions: one static load in `Minion.cpp`, sprites reference that texture.
- **Files:** `src/Entities/Minion.cpp`, `src/Entities/Minion.h` (removed per-instance texture member).

---

## 2. Gradual collapse in frame rate after a few minions or short play time (“stuck” while algorithms run)

### Symptom

Performance degraded noticeably after ordering or moving a small number of minions, even when the entity count was still low.

### Root causes

1. **Pathfinding debug accumulators in the hot path**  
   Dijkstra and A* pushed every expanded / relaxed node into `lastClosedSet_` and `lastOpenSet_` on **every** `findPath` call, regardless of whether path debug drawing was enabled. The open-set list could grow extremely large on a big map because it recorded duplicate queue relaxations, not just unique tiles. That meant heavy allocation and memory traffic on each search.

2. **Console logging in the hot path**  
   `Logger::get()->info(...)` for cases such as “target not passable” or “no path found” could flood I/O when many orders failed or targets were invalid.

### Solutions

1. **Gate search visualization data behind F2**  
   - `setRecordSearchVisualization(bool)` on `IPathfindingSystem`; pathfinding implementations only push to `lastClosedSet_` / `lastOpenSet_` when recording is enabled.  
   - Default off; `Game` turns recording on only when path debug (`F2`) is active.  
   - **Files:** `src/Algorithms/Pathfinding/IPathfindingSystem.h`, `DijkstrasPathfindingSystem.*`, `AStarPathfindingSystem.*`, `src/Game.cpp` (constructor + F2 toggle).

2. **Demote noisy messages**  
   Non-warning path messages moved to `debug` so default log level stays quiet.  
   - **Files:** `DijkstrasPathfindingSystem.cpp`, `AStarPathfindingSystem.cpp`.

3. **Lightweight pathfinding timing**  
   `PathfindingPerf` records wall time per `findPath` invocation for HUD.  
   - **Files:** `src/Util/PathfindingPerf.h`, `src/Util/PathfindingPerf.cpp`, both pathfinding `.cpp` files, `CMakeLists.txt`.

---

## 3. Collision cost with many overlapping units (quadtree worst case)

### Symptom

With many entities clustered (for example in the deploy zone), collision could still cost more than expected even with a quadtree.

### Root causes

1. **Shallow tree**  
   Subdivision stopped at level 8, so crowded regions could leave large buckets and devolve toward many pairwise checks in one node.

2. **Per-pair dynamic casting**  
   Skipping commander–minion overlap used `dynamic_cast` for every candidate pair.

3. **Small pair buffer reserve**  
   The pair list started with a tiny `reserve`, causing reallocations when many pairs were collected.

### Solutions

1. **Deeper quadtree**  
   Maximum depth increased from 8 to 14 so large maps and tight clusters split more before bucket costs explode.  
   - **File:** `src/Algorithms/Collision/QuadtreeCollisionSystem.cpp`.

2. **Fast entity kind check**  
   `EntityKind` and `Entity::kind()` replace `dynamic_cast` for commander–minion filtering.  
   - **Files:** `src/Entities/Entity.h`, `PlayerCommander.h`, `Minion.h`, `QuadtreeCollisionSystem.cpp`.

3. **Smarter `pairs` reserve**  
   Heuristic reserve scales with entity count (capped by worst-case pair count).  
   - **File:** `QuadtreeCollisionSystem.cpp`.

4. **Brute-force parity**  
   Brute-force collision now **skips** commander–minion overlap resolution the same way the quadtree path does (commander can pass through minions without mutual push).  
   - **File:** `src/Algorithms/Collision/BruteForceCollisionSystem.cpp`.

---

## 4. Per-frame gameplay overhead with many minions on flags

### Symptom

`updateGameplay` scaled poorly when iterating every minion against every flag tile to see who was capturing.

### Root cause

Nested loop: for each minion, scan the full `flagTilePositions_` list.

### Solution

- Build a **hash map** from packed tile coordinates to flag index (`flagTileToIndex_`) when the map initializes.
- Per minion: one `O(1)` lookup to attribute capture contribution.
- Retargeting minions whose goal flag was captured uses the same map for goal index resolution.
- **Files:** `src/Game.h`, `src/Game.cpp` (`initializeTileMap`, `updateGameplay`, `retargetMinionsFromCapturedFlagGoals`).

---


## Summary table

| Issue | Primary cause | Main fix |
|--------|----------------|----------|
| Bulk spawn freeze | Per-minion texture load | Shared minion texture |
| Degradation after pathfinding | Unbounded debug vectors + log noise | Gate viz behind F2; debug logs; perf counters |
| Clustered collision hotspots | Shallow quadtree, casting, small reserves | Deeper tree, `EntityKind`, better reserve; brute-force skip |
| Many minions on objectives | Flag scans per minion | `flagTileToIndex_` map |
| CSV path columns always 0 | Last-frame snapshot + rare `findPath` | Per-second summed metrics after full frame |
| Multi-second hitches with crowds | Thousands of `findPath` in one frame | `PathfindBudget` + pending paths across frames |
| A* / Dijkstra “start or end out of bounds” warnings | `worldToTile` can yield invalid start indices at edges | `TileMap::clampTile`; clamp start in `findPath`; warn only if end invalid |

---

## 5. CSV shows `pathfinding_us` / `path_calls` as zero while minions still move

### Symptom

Benchmark CSV rows had `pathfinding_us` and `path_calls` always 0 even though minions followed routes correctly.

### Root causes

1. **`findPath` is not called every frame** — after `setTarget`, the minion stores world waypoints and only **steers** in `Minion::update`. Weeks of steady movement can pass with **no new search**, so many frames legitimately do no pathfinding.
2. **CSV sampled last frame only** — each row logged instantaneous `PathfindingPerf` for a single frame. Unless that frame happened to issue orders or retargets, the snapshot was always zero.

### Solution

- Log **sums over each ~1 second interval**: collision μs, pathfinding μs, and path call counts accumulated over all frames in that window, sampled **after `render()`** so work triggered in `processEvents`, `update`, or path-debug (`F2`) is included.
- CSV header columns renamed to `collision_us_sum_1s`, `pathfinding_us_sum_1s`, `path_calls_sum_1s` so reports do not confuse them with single-frame values.

---

## 6. Long freezes / “stuck” game with hundreds of minions after bulk order or flag capture

### Symptom

With ~1000 units, the window stops responding for seconds; CSV shows enormous `pathfinding_us_sum_1s` (multi‑second CPU) in a single row.

### Root cause

Every `Minion::setTarget` runs a full grid search (`findPath`) on the main thread. Events such as **right‑click move to all**, **J bulk spawn + order**, or **`retargetMinionsFromCapturedFlagGoals`** after a flag is captured can trigger **hundreds or thousands of searches in one frame** on a 512×512 map, which blocks input and rendering.

### Solution

- Introduce a per‑frame **`PathfindBudget`** (default **64** searches per frame, see `Game::kPathfindsPerFrameBudget`).
- `Game::run` refills the budget at the start of each frame; `Minion` defers overflow with `pendingPath_` and drains pending work across subsequent `update` ticks.
- New orders may spread across many frames instead of one; the game stays responsive.

**Files:** `src/Util/PathfindBudget.h`, `src/Entities/Minion.*`, `src/Game.h`, `src/Game.cpp`.

---

## 7. Log warnings: `A*: start or end out of bounds` (or Dijkstra equivalent)

### Symptom

The logger repeatedly warns that the pathfinding start or end tile is out of bounds, even during normal play (units near map edges, or after converting world position to tile coordinates).

### Root cause

`TileMap::worldToTile` maps world coordinates to tile indices with integer division. A unit whose centre sits on the **right or bottom** edge of the map (for example, world x equal to map width in pixels) maps to tile index **equal to** `width` or `height`, which is **not** a valid tile index (valid range is `0 … width - 1`). Similarly, positions far enough **left or above** the origin can yield **negative** tile indices. Pathfinding correctly rejected those starts, but the warning was noisy and looked like a bug.

### Solution

- Add **`TileMap::clampTile(sf::Vector2i)`** to clamp a tile coordinate into the valid rectangle for the current map dimensions.
- At the start of **`findPath`** in both A* and Dijkstra, set **`start = map.clampTile(start)`** so searches always begin from an in-bounds tile when the unit is still on or just inside the playable area.
- Keep validation for **`end`**: if the goal tile is still out of bounds, log a warning (message updated to **end tile** only, since start is now clamped) and return no path.

**Files:** `src/World/TileMap.h`, `src/Algorithms/Pathfinding/AStarPathfindingSystem.cpp`, `src/Algorithms/Pathfinding/DijkstrasPathfindingSystem.cpp`.

---

## Maintenance notes

- Turning **F2** on intentionally re-enables heavy recording for path visualization; keep it off for benchmarks and large crowds unless you need the visualization. With **F2 on**, CSV path sums also include the extra `findPath` invoked for debug drawing each frame.
- If new entity types need special collision pairing, extend `EntityKind` (or a similar discriminator) instead of adding hot-path RTTI.
- When adding new bulk-spawned drawable agents, **share** assets (textures, fonts) or load them once at startup rather than per instance.
