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

---

## Maintenance notes

- Turning **F2** on intentionally re-enables heavy recording for path visualization; keep it off for benchmarks and large crowds unless you need the visualization.
- If new entity types need special collision pairing, extend `EntityKind` (or a similar discriminator) instead of adding hot-path RTTI.
- When adding new bulk-spawned drawable agents, **share** assets (textures, fonts) or load them once at startup rather than per instance.
