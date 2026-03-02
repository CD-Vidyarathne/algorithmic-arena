---
name: Algorithmic Arena
overview: Complete the Algorithmic Arena engine and "The Nexus War" game by implementing the missing entity types (Commander, Minions, Nexus Cores), both collision and pathfinding algorithm systems with build-time switching, debug visualizations, performance overlay, scenario support, and win/lose gameplay—aligned with the docs and current codebase.
isProject: false
---

# Plan: Finish Engine and Game (Algorithmic Arena)

## Critical Review of Previous Plan (Changes Made)

The original plan was structurally sound but had several gaps and misordering issues that would have caused wasted rework:

1. **`TileMap::setTile` performance bug ignored** — every `setTile` call rebuilds the entire `VertexArray` (O(width×height)). `initializeTileMap` calls it 128+ times, making startup O(N²) in tile count. This must be fixed before adding dynamic map features or benchmarking.
2. **`Entity` color_ accessor was noted but not actioned** — listed as a bullet in the entity section but no concrete fix was prescribed. Omitting it forces every derived class to work around it immediately.
3. **`EntityManager` has no removal mechanism** — dead entities (killed Minions, captured Nexus state transitions) cannot be removed from the vector. This will cause use-after-free and stale-update bugs as soon as the gameplay loop is added. Must be fixed before collision or gameplay work.
4. **Collision ordering was ambiguous** — the plan said "after or before `updateAll`" without deciding. The correct order is: update positions → resolve collisions → render. Leaving it open causes inconsistent behaviour.
5. **Pathfinding system ownership was unclear** — Minions need to call `findPath` but the plan said "Minion needs a reference or path requests go through Game" without resolving it. This needs a concrete decision (reference to system passed at construction) to avoid a circular-dependency anti-pattern.
6. **`ICollisionSystem::update` signature included `float dt`** — collision systems shouldn't need dt; they operate on current state. Including dt couples them incorrectly to time semantics. Removed from interface.
7. **Section 4 (Gameplay) placed before Sections 5–7 (Debug/Overlay/Scenarios)** was correct, but **Section 3 (Pathfinding) was placed before Section 4 (Quadtree)** — the Quadtree is a collision concern and belongs immediately after BruteForce. Dijkstra/A* need a working collision system so Minions can actually navigate a populated world meaningfully.
8. **No mention of font loading for DebugOverlay** — `sf::Text` requires a loaded `sf::Font`; omitting this causes a silent blank overlay or a crash. Added to the overlay section.
9. **`ScenarioManager` was "optional but recommended"** — given the Mar 15 report deadline requires at least one working algorithm to reflect on, a minimal hardcoded scenario loader is essential, not optional.
10. **No `initializeEntities()` method prescribed** — the plan mentioned adding entities in `initializeTileMap()` which conflates map construction with entity placement. A separate method is needed.
11. **`TileMap` missing world-to-tile and tile-to-world coordinate helpers** — pathfinding returns tile coords; Minions operate in world coords. Without explicit conversion helpers, every caller will inline the `tileSize_` multiplication, creating scattered magic numbers.
12. **A\* section had a typo** — `*A:` instead of `A*:` in the pathfinding section.

---

## Current State vs Goal

**Done:**

- CMake with SFML/spdlog, build-time options `USE_QUADTREE_COLLISION` and `USE_ASTAR_PATHFINDING` ([CMakeLists.txt](CMakeLists.txt))
- Game loop with delta time, window, event handling ([Game.cpp](src/Game.cpp))
- TileMap: grid storage, walkable/blocked, VertexArray rendering, test map — **but `setTile` rebuilds entire VertexArray on every call** ([World/TileMap.cpp](src/World/TileMap.cpp) line 15)
- Entity base (position, velocity, size, color, `update`/`render`/`getBounds`) and EntityManager ([Entities/Entity.h](src/Entities/Entity.h), [Entities/EntityManager.h](src/Entities/EntityManager.h)) — **but `color_` has no accessor and EntityManager has no removal**
- Game owns TileMap and EntityManager and calls `updateAll`/`renderAll`

**Missing (from [docs/5_Summary.md](docs/5_Summary.md), [docs/4_Game_Concept.md](docs/4_Game_Concept.md), [docs/3_Initial_Development_Context.md](docs/3_Initial_Development_Context.md)):**

- Concrete entities: PlayerCommander, Minion, NexusCore
- Collision: `ICollisionSystem`, BruteForce, Quadtree, Game integration
- Pathfinding: grid coordinate helpers, `IPathfindingSystem`, Dijkstra, A*, Minion AI
- Gameplay: spawn minions, orders (capture/defend/move), capture logic, win/lose
- Visualizations: bboxes, quadtree node boundaries, pathfinding grid/open-closed/paths
- Performance overlay: FPS, entity count, collision/pathfinding timings
- Core utilities: Timer for benchmarking, font resource for DebugOverlay
- ScenarioManager / configurable test scenarios
- CSV logging for benchmark data export

---

## 0. Pre-condition fixes (do these first, before any new feature work)

These are bugs or design gaps in existing code that will cascade into rework if deferred.

### 0a. Fix `TileMap::setTile` — stop full rebuild on every call

**File:** `src/World/TileMap.cpp`

Replace `updateVertices()` in `setTile` with a single-tile update helper:

```cpp
void TileMap::updateTileVertices(unsigned int x, unsigned int y) {
    unsigned int tileIndex = y * width_ + x;
    unsigned int vertexIndex = tileIndex * 6;
    sf::Color color = getTileColor(tiles_[tileIndex]);
    float posX = static_cast<float>(x * tileSize_);
    float posY = static_cast<float>(y * tileSize_);
    // ... set the 6 vertices for this tile only
}
```

Keep the full `updateVertices()` only in the constructor. Change `setTile` to call `updateTileVertices(x, y)` instead.

**Why now:** `initializeTileMap` calls `setTile` ~130 times. Each call currently rebuilds 40×22=880 tiles worth of vertices. That's ~114,400 vertex assignments at startup for no reason. It also makes any future dynamic map mutations (lava hazards from the game concept) prohibitively expensive.

### 0b. Fix `Entity` — add `color_` accessor and make Entity non-copyable

**File:** `src/Entities/Entity.h`

Add to the public interface:
```cpp
sf::Color getColor() const { return color_; }
```

Also add deleted copy constructor and assignment to enforce ownership semantics (Entity is managed via `unique_ptr`; accidental copies would be bugs):
```cpp
Entity(const Entity&) = delete;
Entity& operator=(const Entity&) = delete;
```

### 0c. Fix `EntityManager` — add deferred removal support

**File:** `src/Entities/EntityManager.h`

Add an `alive_` flag mechanism or a `markForRemoval` approach. The safest pattern is to add a virtual `isAlive()` method to Entity (default `true`) and let EntityManager sweep dead entities after `updateAll`:

```cpp
// In Entity.h, add:
bool isAlive() const { return alive_; }
void destroy() { alive_ = false; }

// In EntityManager.h, after updateAll():
void removeDeadEntities() {
    entities_.erase(
        std::remove_if(entities_.begin(), entities_.end(),
            [](const auto& e) { return !e->isAlive(); }),
        entities_.end());
}
```

Call `entityManager_.removeDeadEntities()` at the end of `Game::update(dt)`.

**Why now:** Without this, killed Minions or consumed Nexus states cannot be cleaned up. Adding it after collision and gameplay are wired in requires touching every system again.

### 0d. Add coordinate conversion helpers to `TileMap`

**File:** `src/World/TileMap.h`

```cpp
sf::Vector2i worldToTile(sf::Vector2f worldPos) const {
    return { static_cast<int>(worldPos.x / tileSize_),
             static_cast<int>(worldPos.y / tileSize_) };
}
sf::Vector2f tileToWorld(sf::Vector2i tilePos) const {
    return { static_cast<float>(tilePos.x * tileSize_),
             static_cast<float>(tilePos.y * tileSize_) };
}
sf::Vector2f tileCentre(sf::Vector2i tilePos) const {
    float half = tileSize_ / 2.f;
    return { static_cast<float>(tilePos.x * tileSize_) + half,
             static_cast<float>(tilePos.y * tileSize_) + half };
}
```

**Why now:** Every pathfinding call and Minion movement update needs these. Inlining `tileSize_` arithmetic everywhere produces scattered magic-number bugs. Centralise once.

---

## 1. Entity layer: concrete types

Once pre-condition fixes (0a–0d) are done:

### 1a. `PlayerCommander`

New file: `src/Entities/PlayerCommander.h` (header-only is fine for a small class; add `.cpp` if it grows).

- Inherit `Entity`.
- Constructor: takes position, sets `size_` to e.g. `{28.f, 28.f}`, color to `sf::Color::Blue`.
- `update(float dt)`: read `sf::Keyboard::isKeyPressed` for WASD/arrows; compute velocity direction vector, normalise it, multiply by `speed_` constant (e.g. 150.f pixels/sec); call `setVelocity`; then `setPosition(getPosition() + getVelocity() * dt)`.
  - **Do not** clamp to map bounds here — collision resolution (Step 2) handles that.
- `render(sf::RenderWindow& window)`: draw `sf::RectangleShape` at `getPosition()` with `getSize()` and `getColor()`.

### 1b. `Minion`

New file: `src/Entities/Minion.h/.cpp`.

- Inherit `Entity`. Constructor: takes position, `IPathfindingSystem*` reference (set at spawn time), `TileMap*` reference (const).
- Members: `std::vector<sf::Vector2f> path_` (world positions of waypoints), `int pathIndex_`, `sf::Vector2i targetTile_`, `float speed_` (e.g. 80.f pixels/sec).
- `update(float dt)`:
  1. If `path_` is not empty and `pathIndex_` is valid, steer toward `path_[pathIndex_]`.
  2. When within arrival threshold (e.g. 4px), advance `pathIndex_`. When `pathIndex_ >= path_.size()`, clear path and stop.
  3. Apply velocity to position.
- `setTarget(sf::Vector2i tile)`: calls `pathfindingSystem_->findPath(tileMap_->worldToTile(getPosition()), tile, *tileMap_)`, converts returned tile coords to world positions via `tileMap_->tileCentre(...)`, stores in `path_`, resets `pathIndex_`.
- `render`: draw a smaller `sf::RectangleShape` (e.g. `{20.f, 20.f}`), color `sf::Color::Cyan`.

**Pathfinding system ownership decision:** `Game` owns the `unique_ptr<IPathfindingSystem>`; Minion holds a raw (non-owning) pointer to it. This is safe because Game outlives all entities. Pass the raw pointer when spawning: `std::make_unique<Minion>(pos, pathfindingSystem_.get(), tileMap_.get())`.

### 1c. `NexusCore`

New file: `src/Entities/NexusCore.h`.

- Inherit `Entity`. Not moveable (`update` is a no-op for position). Members: `float captureProgress_` (0.0–1.0), `bool captured_`.
- `startCapture(float dt)`: increments `captureProgress_` by `dt * captureRate_`; clamps to 1.0; sets `captured_` when 1.0 reached.
- `render`: draw `sf::RectangleShape`; color lerps from `sf::Color::White` (neutral) to `sf::Color::Green` (captured) based on `captureProgress_`.
- Expose `isCaptured()` and `getCaptureProgress()`.

### 1d. Game integration

Add `initializeEntities()` method to `Game` (separate from `initializeTileMap()`):

```cpp
void Game::initializeEntities() {
    // Commander at tile (2,2) centre
    auto commander = std::make_unique<PlayerCommander>(tileMap_->tileCentre({2, 2}));
    entityManager_.addEntity(std::move(commander));

    // A few Minions for initial testing
    for (int i = 0; i < 5; ++i) {
        auto minion = std::make_unique<Minion>(
            tileMap_->tileCentre({3 + i, 3}),
            pathfindingSystem_.get(),
            tileMap_.get());
        entityManager_.addEntity(std::move(minion));
    }

    // Nexus Cores at predefined positions
    entityManager_.addEntity(std::make_unique<NexusCore>(tileMap_->tileCentre({20, 5})));
    entityManager_.addEntity(std::make_unique<NexusCore>(tileMap_->tileCentre({35, 18})));
}
```

Call `initializeEntities()` in `Game` constructor, **after** both `initializeTileMap()` and algorithm system construction (because Minions need the pathfinding pointer).

---

## 2. Collision system

### 2a. Interface

**File:** `src/Algorithms/Collision/ICollisionSystem.h`

```cpp
#pragma once
#include "../../Entities/EntityManager.h"
#include "../../World/TileMap.h"
#include <SFML/Graphics/RenderWindow.hpp>

class ICollisionSystem {
  public:
    virtual ~ICollisionSystem() = default;
    // Resolve collisions between entities and map; mutates entity positions
    virtual void update(EntityManager& entities, const TileMap& map) = 0;
    virtual void drawDebug(sf::RenderWindow& window) = 0;
};
```

Note: **no `float dt` parameter**. Collision resolution operates on current state (positions after movement); it does not need elapsed time.

### 2b. `BruteForceCollisionSystem`

**File:** `src/Algorithms/Collision/BruteForceCollisionSystem.h/.cpp`

- `update`: iterate all pairs (i, j) with j > i; AABB test via `getBounds().findIntersection(...)`; if overlap, push both entities apart by half the overlap on each axis. Then entity-vs-tile: for each entity, check all 4 corner tiles; if any corner is in a Blocked tile, push entity out.
- `drawDebug`: for each entity, draw a `sf::RectangleShape` outline at `getBounds()` with no fill, green stroke.

### 2c. `QuadtreeCollisionSystem`

**File:** `src/Algorithms/Collision/QuadtreeCollisionSystem.h/.cpp`

- Internal `Quadtree` struct with: `sf::FloatRect bounds_`, `int capacity_` (e.g. 4), `std::vector<Entity*> entities_`, four `unique_ptr<Quadtree>` children.
- `insert(Entity*)`: if not subdivided and under capacity, store; else subdivide and redistribute.
- `queryPairs(std::vector<std::pair<Entity*,Entity*>>&)`: collect pairs within same node.
- `update`: rebuild quadtree from map bounds each frame, insert all entities, query pairs, run AABB on pairs only, resolve. Same entity-vs-tile check as BruteForce.
- `drawDebug`: recurse tree, draw each node's `sf::FloatRect` as a rectangle outline (distinct color, e.g. `sf::Color::Yellow`).

### 2d. Game integration

Add to `Game.h`:
```cpp
#include "Algorithms/Collision/ICollisionSystem.h"
std::unique_ptr<ICollisionSystem> collisionSystem_;
bool debugCollision_ = false;
```

In `Game` constructor (before `initializeEntities()`):
```cpp
#ifdef USE_QUADTREE_COLLISION
    collisionSystem_ = std::make_unique<QuadtreeCollisionSystem>();
#else
    collisionSystem_ = std::make_unique<BruteForceCollisionSystem>();
#endif
```

In `Game::update(dt)` — **order matters**:
```cpp
entityManager_.updateAll(dt);              // 1. Move all entities
collisionSystem_->update(entityManager_,   // 2. Resolve positions
                          *tileMap_);
entityManager_.removeDeadEntities();       // 3. Purge dead
```

In `Game::render()`, after tilemap and entities:
```cpp
if (debugCollision_) collisionSystem_->drawDebug(window_);
```

In `Game::processEvents()`, add toggle: `F1` key flips `debugCollision_`.

---

## 3. Pathfinding system

### 3a. Interface

**File:** `src/Algorithms/Pathfinding/IPathfindingSystem.h`

```cpp
#pragma once
#include "../../World/TileMap.h"
#include <SFML/System/Vector2.hpp>
#include <vector>

class IPathfindingSystem {
  public:
    virtual ~IPathfindingSystem() = default;
    // Returns list of tile coordinates from start to end (inclusive), empty if no path
    virtual std::vector<sf::Vector2i> findPath(
        sf::Vector2i start, sf::Vector2i end, const TileMap& map) = 0;
    // Store last search state for debug drawing
    virtual void drawDebug(sf::RenderWindow& window, const TileMap& map) = 0;
};
```

### 3b. `PathNode` helper (shared internal struct)

Define in a common header `src/Algorithms/Pathfinding/PathNode.h`:
```cpp
struct PathNode {
    sf::Vector2i pos;
    int gCost;      // cost from start
    int fCost;      // g + h (A*); same as g for Dijkstra
    sf::Vector2i parent;
    bool operator>(const PathNode& o) const { return fCost > o.fCost; }
};
```

### 3c. `DijkstrasPathfindingSystem`

**File:** `src/Algorithms/Pathfinding/DijkstrasPathfindingSystem.h/.cpp`

- `findPath`: standard Dijkstra with `std::priority_queue<PathNode, std::vector<PathNode>, std::greater<>>`. 4-connected neighbours (can extend to 8). Store `visitedNodes_` and `openNodes_` for debug. Reconstruct path by tracing `parent` pointers. Return empty vector if `end` is not walkable or unreachable.
- `drawDebug`: draw visited (closed) nodes as semi-transparent red squares; open nodes as semi-transparent yellow; path tiles as bright white line segments. All sized to `tileSize`.

### 3d. `AStarPathfindingSystem`

**File:** `src/Algorithms/Pathfinding/AStarPathfindingSystem.h/.cpp`

- Same structure as Dijkstra but `fCost = gCost + heuristic(pos, end)`. Use Manhattan distance: `h = abs(pos.x - end.x) + abs(pos.y - end.y)` for 4-connectivity. Same `drawDebug` approach.

### 3e. Game integration

Add to `Game.h`:
```cpp
#include "Algorithms/Pathfinding/IPathfindingSystem.h"
std::unique_ptr<IPathfindingSystem> pathfindingSystem_;
bool debugPathfinding_ = false;
```

In `Game` constructor (before `initializeEntities()`):
```cpp
#ifdef USE_ASTAR_PATHFINDING
    pathfindingSystem_ = std::make_unique<AStarPathfindingSystem>();
#else
    pathfindingSystem_ = std::make_unique<DijkstrasPathfindingSystem>();
#endif
```

In `Game::render()`:
```cpp
if (debugPathfinding_) pathfindingSystem_->drawDebug(window_, *tileMap_);
```

In `processEvents()`: `F2` toggles `debugPathfinding_`.

**Minion path request trigger:** In `Game::update(dt)`, after entity update, iterate entities; if a `Minion*` (dynamic_cast or use a typed list — see note below) has no active path and `captureTarget_` is set, call `minion->setTarget(captureTarget_)`.

> **Design note:** `dynamic_cast` in the update loop is fine at small scale but adds coupling. For a better separation, give `EntityManager` a typed accessor `getEntitiesOfType<T>()` using `dynamic_cast` internally, or maintain separate `std::vector<Minion*> minions_` in Game. For this project scope, the separate typed pointer vector in Game is the simplest correct approach.

---

## 4. Quadtree collision system (build and wire)

This is already covered structurally in Section 2c. What changes versus the original plan ordering: **implement and test QuadtreeCollisionSystem immediately after BruteForce**, not after pathfinding. The `USE_QUADTREE_COLLISION` CMake flag is already in place. Testing both collision variants early means the benchmarking comparison (critical for the Mar 15 report) is available with the simplest possible scene (just moving entities, no pathfinding required).

Build and validate by:
```bash
cmake -DUSE_QUADTREE_COLLISION=ON .. && make
cmake -DUSE_QUADTREE_COLLISION=OFF .. && make
```
Run both builds with F1 debug on and confirm quadtree node subdivisions are visible.

---

## 5. Gameplay: Nexus War loop

### 5a. Nexus capture logic

In `Game::update(dt)`, after collision resolution, iterate Minions:
- For each Minion, call `tileMap_->worldToTile(minion->getPosition())` to get its tile.
- For each NexusCore, call `tileMap_->worldToTile(nexus->getPosition())`.
- If Minion tile == Nexus tile and Nexus is not yet captured: call `nexus->startCapture(dt)`.

### 5b. Commander orders (keyboard-driven for simplicity)

- `C` key: for all Minions, set target to the nearest uncaptured NexusCore.
- `M` key + mouse click: set Minion target to the clicked tile (convert `sf::Mouse::getPosition(window_)` using `worldToTile`).
- `Space`: spawn a new Minion at Commander's current position (up to `maxMinions_` cap, e.g. 20).

Store `std::vector<NexusCore*> nexusCores_` and `std::vector<Minion*> minions_` in `Game` as non-owning views into `EntityManager` (populated during `initializeEntities()`).

### 5c. Win/Lose conditions

In `Game::update(dt)`:
```cpp
// Win: all nexuses captured
bool allCaptured = std::all_of(nexusCores_.begin(), nexusCores_.end(),
    [](const NexusCore* n) { return n->isCaptured(); });
if (allCaptured) { gameState_ = GameState::Won; }

// Lose: time limit exceeded or no minions left
gameTimer_ += dt;
if (gameTimer_ > timeLimitSeconds_ || minions_.empty()) {
    gameState_ = GameState::Lost;
}
```

Add `enum class GameState { Playing, Won, Lost }` to `Game.h`. In `render()`, if `gameState_ != Playing`, draw a centered `sf::Text` message ("You Win!" / "Game Over"). In `run()`, if not Playing, stop the loop after one more render.

---

## 6. Debug visualizations

Toggleable via function keys — keep state in `Game`:

| Toggle | Key | What it draws |
|--------|-----|---------------|
| `debugCollision_` | F1 | Entity bounding boxes (green); Quadtree node boundaries (yellow, only when Quadtree build active) |
| `debugPathfinding_` | F2 | Path tiles (white); visited/open sets (red/yellow) for last search |
| `debugGrid_` | F3 | Tile grid lines (thin grey) |

All debug drawing happens in `render()`, after entities, before `window_.display()`.

---

## 7. Performance overlay and benchmarking

### 7a. `Core/Timer`

**File:** `src/Core/Timer.h` (header-only)

```cpp
#pragma once
#include <chrono>

class Timer {
  public:
    void start() { start_ = std::chrono::high_resolution_clock::now(); }
    double elapsedMicros() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::micro>(end - start_).count();
    }
  private:
    std::chrono::high_resolution_clock::time_point start_;
};
```

### 7b. `UI/DebugOverlay`

**File:** `src/UI/DebugOverlay.h/.cpp`

- Constructor: takes `sf::Font&` reference (font must be loaded by caller — see note).
- `update(float fps, int entityCount, double collisionMicros, double pathfindingMicros)`.
- `draw(sf::RenderWindow&)`: draw an `sf::Text` block in top-left corner, dark semi-transparent background rectangle behind it for readability.

**Font loading note:** Add a font file (e.g. a monospace TTF) to `assets/` and load it in `Game` constructor via `font_.loadFromFile(...)`. Pass `font_` to `DebugOverlay`. The `sf::Font` object must outlive all `sf::Text` objects that use it — store it as a member of `Game`.

### 7c. Game integration

In `Game.h`:
```cpp
#include "Core/Timer.h"
#include "UI/DebugOverlay.h"
sf::Font font_;
DebugOverlay debugOverlay_;
Timer collisionTimer_;
Timer pathfindingTimer_;
double lastCollisionMicros_ = 0.0;
double lastPathfindingMicros_ = 0.0;
bool showOverlay_ = true;  // toggled with F4
```

Wrap collision call with timer:
```cpp
collisionTimer_.start();
collisionSystem_->update(entityManager_, *tileMap_);
lastCollisionMicros_ = collisionTimer_.elapsedMicros();
```

Similarly wrap pathfinding requests.

In `update(dt)`: call `debugOverlay_.update(1.f / dt, entityManager_.count(), lastCollisionMicros_, lastPathfindingMicros_)`.
Add `count()` method to `EntityManager` (returns `entities_.size()`).

### 7d. CSV logging (essential for report, not optional)

**File:** `src/Core/CsvLogger.h` (header-only)

```cpp
#pragma once
#include <fstream>
#include <string>

class CsvLogger {
  public:
    explicit CsvLogger(const std::string& filename) : file_(filename) {
        file_ << "timestamp_s,fps,entity_count,collision_us,pathfinding_us\n";
    }
    void log(float ts, float fps, int entities, double col, double path) {
        file_ << ts << "," << fps << "," << entities
              << "," << col << "," << path << "\n";
    }
  private:
    std::ofstream file_;
};
```

In Game, create `CsvLogger csvLogger_("benchmark.csv")` and call `csvLogger_.log(...)` every second (track with a `logTimer_` accumulator in `update`). This produces the data needed for the Chapter 5 graphs in the report.

---

## 8. ScenarioManager

A minimal but functional `ScenarioManager` is needed for repeatable benchmarks. Plain text config is sufficient — no JSON library dependency needed.

**File:** `src/World/ScenarioManager.h/.cpp`

Config format (plain text, one key=value per line):
```
map_width=40
map_height=22
tile_size=32
minion_count=50
nexus_positions=20,5;35,18;10,18
time_limit=120
```

`ScenarioManager::load(const std::string& path)` returns a `ScenarioConfig` struct. `Game` optionally accepts a path argument; if none provided, uses hardcoded defaults (current behaviour).

For the report: create at minimum three scenario files:
- `scenarios/sparse_10.txt` — 10 Minions, open map
- `scenarios/medium_50.txt` — 50 Minions, some obstacles
- `scenarios/stress_200.txt` — 200 Minions, maze-like map

These are the data points for the collision and pathfinding performance graphs.

---

## Revised Implementation Order

The order below is driven by: (a) unblock the next step, (b) minimize rework, (c) have something benchmarkable for the Mar 15 report.

1. **Pre-condition fixes (0a–0d):** Fix `setTile`, add `color_` accessor, add `Entity::destroy()`/`EntityManager::removeDeadEntities()`, add coordinate helpers. ~2 hours.
2. **Algorithm system construction in Game constructor + `initializeEntities()`:** Wire both `collisionSystem_` and `pathfindingSystem_` into Game (stubs or real), add `initializeEntities()`. ~1 hour.
3. **`ICollisionSystem` + `BruteForceCollisionSystem` + Game integration:** Commander and Minions stop passing through walls. Enable F1 debug. ~3–4 hours.
4. **`QuadtreeCollisionSystem`:** Wire with `USE_QUADTREE_COLLISION`. Both collision variants now benchmarkable. ~4–5 hours. — **Report-ready: collision comparison available.**
5. **`PlayerCommander` + basic movement:** Player can move and is collision-resolved. ~1–2 hours.
6. **`IPathfindingSystem` + `Dijkstra` + `A*` + `PathNode`:** Both pathfinding variants implemented. Wire into Game with `USE_ASTAR_PATHFINDING`. Enable F2 debug. ~4–5 hours.
7. **`Minion` AI with pathfinding:** Minions pathfind to targets. Both pathfinding variants benchmarkable. ~2–3 hours. — **Report-ready: pathfinding comparison available.**
8. **`NexusCore` + capture logic + win/lose + Commander orders:** Full Nexus War loop. ~3–4 hours.
9. **`Timer` + `DebugOverlay` + font:** FPS, entity count, collision/pathfinding microseconds on screen. ~2–3 hours.
10. **CSV logging:** `CsvLogger` writing every second. ~1 hour.
11. **`ScenarioManager` + scenario files:** Repeatable benchmark scenarios. ~2–3 hours.
12. **Debug visualizations (F3 grid, path open/closed, quadtree nodes):** Polish and completeness. ~2 hours.

**Total estimate:** ~28–38 hours. With ~20h/week, this is achievable by mid-March for the core benchmarkable engine (steps 1–7), with the full game loop and reporting infrastructure by end of March.

---

## Key Architectural Invariants (do not violate)

1. **Ownership:** `Game` owns all systems via `unique_ptr`. Entities hold raw non-owning pointers to systems.
2. **Update order each frame:** `updateAll(dt)` → `collisionSystem_->update(...)` → `removeDeadEntities()`.
3. **Algorithm selection:** Only via CMake `#ifdef`, never runtime polymorphism switch. This keeps builds clean and separate for benchmarking.
4. **No `dt` in collision or pathfinding interfaces.** Collision resolves current state; pathfinding is stateless per call.
5. **Coordinate system:** all world positions in pixels (`float`); all tile positions in tile indices (`int`). Always use `TileMap::worldToTile` / `tileCentre` to convert. Never inline the multiplication.
6. **Debug state lives in `Game`.** No debug flags in entity or algorithm classes.

---

## 9. Tile System Overhaul: 5-Type Textured Map with Map File Format

This section replaces the current 2-type (`Walkable`/`Blocked`) hardcoded TileMap with a full 5-type textured tile system loaded from plain-text map files. It also mandates the camera and deployment zone logic.

---

### 9a. Tile Type Redesign

**Current state:** `enum class TileType { Walkable, Blocked }` — two types, solid colour rendering, no textures.

**New design:** 5 tile types with distinct gameplay properties:

| Symbol | TileType enum | Passable | Speed Modifier | Role |
|--------|--------------|----------|----------------|------|
| `G` | `Grass` | Yes | 1.0× | Normal walkable ground; default open area |
| `M` | `Mud` | Yes | 0.6× | Slow terrain; pathfinding cost = ~1.67 vs 1.0 for Grass |
| `T` | `Tree` | No | — | Impassable; defines maze walls |
| `L` | `Lava` | No | — | Impassable blocker; can be used at maze boundaries or as hazard |
| `F` | `Flag` | Yes | 1.0× | Capture objective; replaces NexusCore entity (see note below) |

**Note on Flag/NexusCore:** The Flag tile is a *capturable tile* not an entity. This is a design shift from the original NexusCore entity approach. Flag tiles are part of the TileMap and have per-tile capture state (`float captureProgress[tileIndex]`). This avoids the overhead of entities that never move and simplifies collision (no entity-vs-nexus AABB needed). The `NexusCore` entity class from Section 1c is **dropped**; capture logic operates directly on TileMap flag tiles.

**Pathfinding cost table** (used by `IPathfindingSystem`):
- `Grass` → cost 10
- `Mud` → cost 17 (≈ 10 / 0.6)
- `Tree`, `Lava` → impassable (skip in neighbour expansion)
- `Flag` → cost 10 (same as Grass)

Using integer costs (×10) avoids floating-point accumulation in Dijkstra/A*.

**Update `TileMap.h`:**
```cpp
enum class TileType { Grass, Mud, Tree, Lava, Flag };

// Replaces old isWalkable — returns false for Tree and Lava
bool isPassable(unsigned int x, unsigned int y) const;

// Movement cost for pathfinding (returns INT_MAX for impassable)
int movementCost(unsigned int x, unsigned int y) const;

// Capture state for Flag tiles (indexed by tileIndex; ignored for non-Flag tiles)
float getCaptureProgress(unsigned int x, unsigned int y) const;
void advanceCapture(unsigned int x, unsigned int y, float dt, float rate);
bool isCaptured(unsigned int x, unsigned int y) const;
```

Replace the old `isWalkable` call sites in collision systems with `isPassable`. Replace the old NexusCore entity logic with TileMap capture queries.

---

### 9b. Asset Management: `TextureManager`

All tile sprites are 256×256 source images (except `grass_tl.png` which is 64×64 — treat as anomaly; scale uniformly to `tileSize_` on load). Flag is 1280×1280; Tree is 64×64. All will be scaled to `tileSize_` via `sf::Sprite::setScale`.

The directional variants (tl, tr, bl, br, horizontal, vertical, intersection, tjunction) are **border transition tiles** — they represent the edge of one terrain type where it meets a different neighbour. This is an **auto-tiling** system.

**File:** `src/World/TextureManager.h/.cpp`

```cpp
class TextureManager {
  public:
    // Load all tile textures from assets/Tiles/
    bool loadAll(const std::string& assetsRoot);

    // Returns the correct texture for a tile type + its 4-neighbour context
    // neighbourMask: bit 0=N, bit 1=E, bit 2=S, bit 3=W (1 = same type, 0 = different)
    const sf::Texture& getTileTexture(TileType type, uint8_t neighbourMask) const;

    const sf::Texture& getFlagTexture() const;
    const sf::Texture& getTreeTexture() const;

  private:
    // Indexed by TileType (Grass/Mud/Lava) then by variant
    std::unordered_map<std::string, sf::Texture> textures_;
};
```

**Auto-tiling variant selection** (for Grass, Mud, Lava which have directional assets):

The 4-bit neighbour mask (N/E/S/W same-type neighbours) maps to variants:

| Mask (N E S W) | Variant | Meaning |
|---|---|---|
| `0000` | `_tl` (or full tile — use tl as standalone) | Isolated tile |
| `0010` (S only) | `_tl` | Top-left cap |
| `0001` (W only) | `_tr` | Top-right cap |
| `0110` (E+S) | `_tl` | Corner |
| `1001` (N+W) | `_br` | Corner |
| `0101` (E+W) | `_horizontal` | Horizontal corridor |
| `1010` (N+S) | `_vertical` | Vertical corridor |
| `1110` (N+E+S) | `_tjunction` | T-junction |
| `1111` (all) | `_intersection` | Full cross |
| … | … | … |

> **Simplification option:** Auto-tiling is visually important but non-trivial to get perfect. For initial implementation, use a single representative texture per type (e.g. `grass_tl.png` for all Grass tiles) and add full auto-tiling as a polish pass. Mark this as a TODO in the code with a `// TODO: auto-tile` comment.

**Tree and Flag** have single sprites — no auto-tiling needed.

---

### 9c. TileMap Rendering: Textured VertexArray with TextureAtlas or Per-Tile Sprites

**Option A (recommended): Per-tile `sf::Sprite` batch via `sf::RenderStates`**

For a 512×512 map, drawing 262,144 individual `sf::Sprite` objects per frame is too slow. Instead, build a `sf::VertexArray` per texture (one draw call per distinct texture variant used):

```cpp
// In TileMap, instead of a single VertexArray with colours,
// maintain one VertexArray per TileType variant (up to ~20 entries):
std::unordered_map<std::string, sf::VertexArray> variantVertices_;
// key = "grass_tl", "mud_horizontal", etc.

void draw(sf::RenderWindow& window, const TextureManager& textures);
// Iterates variantVertices_, calls window.draw(va, sf::RenderStates(&texture))
```

This keeps draw calls to ~20 max regardless of map size.

**Dirty flag for partial updates:** Since `setTile` now calls `updateTileVertices(x, y)` (from fix 0a), it must also rebuild the correct entry in `variantVertices_`. Add `markDirty(unsigned int x, unsigned int y)` and rebuild only affected entries on next `draw()` call.

**Option B (simpler fallback):** Keep the existing single VertexArray with solid colours per type for algorithm correctness, add sprite rendering as a separate visual layer. This is acceptable for the submission given time constraints.

**Decision for plan:** Implement Option B first (colours only, fast), then add Option A textured rendering once the gameplay loop is stable. The map file format and tile type system are independent of which rendering option is active.

---

### 9d. Map File Format

**Location:** `maps/` directory in project root.

**Format:** Plain text, `.map` extension. Each line is one row of the map. Each character is one tile symbol. Whitespace and `#`-prefixed lines are comments/metadata.

**Symbol table:**

| Symbol | Tile |
|--------|------|
| `G` | Grass |
| `M` | Mud |
| `T` | Tree (maze wall) |
| `L` | Lava |
| `F` | Flag (capture point) |
| `D` | Deployment zone (Grass tile tagged as valid Minion spawn) |
| `C` | Commander start position (Grass tile + commander spawn marker) |
| `E` | Entrance tile (Grass at the boundary between deploy zone and maze interior) |

`D`, `C`, `E` are **metadata overlays** — the underlying tile is Grass; they additionally set flags in a parallel `uint8_t flags_[]` array in TileMap.

**Example map header block** (lines starting with `#`):
```
# name=Nexus Siege
# width=512
# height=512
# tile_size=32
# time_limit=180
# minion_cap=200
```

**Example map fragment (top-left corner):**
```
TTTTTTTTTTTTTTTTTTT...
TDDDDDDDDDDDDDDDDDDT...
TDDDDDDDDDDDDDDDDDDT...
TDDCEEEEEEGGGGGDDDT...
TGGGGGGGGGGGGGGGGGGT...
TGGMMMMMMMGGGGGGGGT...
TGGMTTTTTMGGGGGGGGT...
...
```

**Parser:** `src/World/MapLoader.h/.cpp`

```cpp
struct MapData {
    unsigned int width;
    unsigned int height;
    unsigned int tileSize;
    std::string name;
    int timeLimitSeconds;
    int minionCap;
    std::vector<TileType> tiles;
    std::vector<uint8_t> flags;       // bitmask: FLAG_DEPLOY=1, FLAG_ENTRANCE=2, FLAG_COMMANDER_START=4
    std::vector<sf::Vector2i> flagTiles;      // positions of all Flag tiles
    std::vector<sf::Vector2i> deployZone;     // positions of all Deploy tiles
    sf::Vector2i commanderStart;
};

class MapLoader {
  public:
    static MapData load(const std::string& path);
};
```

**Validation rules enforced at load time:**
1. Width and height must be declared and match the actual character grid dimensions.
2. Unknown symbols → `Grass` with a warning logged via `Logger`.
3. Exactly one `C` tile required (commander start); log error if missing or multiple.
4. At least one `F` tile required (capture objective).
5. `D` tiles must only appear at the map border band (outer N rows/columns as defined by `deployBorderWidth`, default 3). Log a warning if a `D` tile is found in the interior.

---

### 9e. Map Layout Rules and Gameplay Zones

The map has three logical zones:

```
+--------------------------------------------------+
|  TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT |
|  T  DEPLOYMENT ZONE  (D tiles, Grass)           T |
|  T  Commander spawns here; Minions deploy here  T |
|  TEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEET |  <- Entrance row
|  T                                              T |
|  T         MAZE INTERIOR                       T |
|  T   Trees form walls; Lava = hazard blocker   T |
|  T   Mud patches = slow terrain                T |
|  T   Flag tiles = capture objectives           T |
|  T                                              T |
|  TTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTTT |
+--------------------------------------------------+
```

**Rules:**
- Deployment zone: outer band (configurable depth, default 3 tiles) of `D`-flagged Grass tiles. Commander starts here. Minions can only be spawned on `D` tiles (enforce in `Game::spawnMinion()`).
- Entrance tiles (`E`): Grass tiles at the inner edge of the deployment zone that are adjacent to the maze interior. There must be at least 2 entrances so Minions can enter the maze. Pathfinding routes all go through these.
- Maze interior: everything inside the entrance row. Tree tiles form walls; the pathfinder routes through Grass/Mud/Flag only.
- Commander movement: Commander can move freely through D tiles and E tiles. Whether Commander can enter the maze is a gameplay choice — **for this project, allow it** (lets the player scout). Commander should not be able to pass through Tree or Lava.
- Minion spawning: only on `D`-flagged tiles. Check `tileMap_->hasFlag(x, y, FLAG_DEPLOY)` before spawning.

**Enforce in `Game::spawnMinion()`:**
```cpp
void Game::spawnMinion() {
    if (minions_.size() >= maxMinions_) return;
    // Find a random deploy tile near commander
    sf::Vector2i cmdTile = tileMap_->worldToTile(commander_->getPosition());
    // Search nearby D-flagged tiles within radius 3
    // ... pick closest available
    auto minion = std::make_unique<Minion>(spawnPos, pathfindingSystem_.get(), tileMap_.get());
    minions_.push_back(minion.get());
    entityManager_.addEntity(std::move(minion));
}
```

---

### 9f. Map Scale: 512×512 Tiles and Camera

A 512×512 tile map at 32px/tile = **16,384×16,384 pixel world**. The window is 1280×720. A camera (`sf::View`) is mandatory.

**Camera design:**
- `sf::View gameView_` member in `Game`, sized to window (1280×720).
- Each frame in `update(dt)`: centre the view on the Commander's world position, clamped so the view never shows outside the map bounds:
```cpp
sf::Vector2f centre = commander_->getPosition();
float halfW = gameView_.getSize().x / 2.f;
float halfH = gameView_.getSize().y / 2.f;
float mapW = static_cast<float>(tileMap_->getWidth() * tileMap_->getTileSize());
float mapH = static_cast<float>(tileMap_->getHeight() * tileMap_->getTileSize());
centre.x = std::clamp(centre.x, halfW, mapW - halfW);
centre.y = std::clamp(centre.y, halfH, mapH - halfH);
gameView_.setCenter(centre);
```
- In `render()`: apply `window_.setView(gameView_)` before drawing TileMap and entities; apply `window_.setView(window_.getDefaultView())` before drawing DebugOverlay (HUD stays in screen space).
- Mouse-to-world conversion for click orders: `window_.mapPixelToCoords(sf::Mouse::getPosition(window_), gameView_)`.

**TileMap rendering optimisation for large maps:** Drawing all 262,144 tiles every frame is wasteful. Add **view frustum culling** to `TileMap::draw()`:

```cpp
void TileMap::draw(sf::RenderWindow& window) {
    // Compute visible tile range from current view
    sf::FloatRect viewBounds = /* window.getView().getViewport mapped to world */;
    unsigned int xMin = std::max(0u, (unsigned int)(viewBounds.position.x / tileSize_));
    unsigned int yMin = std::max(0u, (unsigned int)(viewBounds.position.y / tileSize_));
    unsigned int xMax = std::min(width_,  (unsigned int)((viewBounds.position.x + viewBounds.size.x) / tileSize_) + 1);
    unsigned int yMax = std::min(height_, (unsigned int)((viewBounds.position.y + viewBounds.size.y) / tileSize_) + 1);
    // Only draw tiles in [xMin,xMax) x [yMin,yMax)
}
```

At 1280×720 with 32px tiles, the visible area is at most 41×23 = ~943 tiles out of 262,144. This is the difference between a smooth 60fps and unplayable slideshow.

With the VertexArray approach: rebuild a temporary `sf::VertexArray` each frame for visible tiles only (943 × 6 = ~5,658 vertices — trivially fast).

---

### 9g. Provided Map Files

Create the following map files in `maps/`:

**`maps/nexus_siege_512.map`** — primary gameplay and benchmark map:
- 512×512 tiles
- Outer 3-tile border: `T` (Tree)
- Deploy band: rows 1–3 and cols 1–3 (and mirrored bottom/right): `D` tiles
- Entrances: multiple `E` tiles connecting deploy band to maze at regular intervals
- Maze interior: procedural-style Tree walls forming corridors (define a pattern in the file — a large grid-of-rooms layout works well)
- Mud patches scattered in maze corridors (every 3rd corridor section)
- Lava in dead-end chambers as hazards
- Flag tiles: 5–8 locations deep inside the maze

**`maps/benchmark_open_512.map`** — open field for stress-testing collision with many entities:
- 512×512
- Outer Tree border
- Deploy band
- Interior: mostly Grass, a few Lava pillars, no maze
- 2 Flag tiles
- Used for: collision benchmark (BruteForce vs Quadtree with 200+ Minions)

**`maps/benchmark_maze_512.map`** — dense maze for pathfinding stress:
- 512×512
- Dense Tree maze (thin corridors, many dead ends)
- Mud throughout (stress-tests weighted pathfinding)
- 4 Flag tiles at far corners
- Used for: Dijkstra vs A* comparison

**Note on file size:** A 512×512 `.map` file is 262,144 characters of symbols + newlines = ~780 KB plain text. This is fine to commit. Generate these files with a simple Python script (provide as `scripts/generate_maps.py`) rather than hand-editing, then commit the generated `.map` files.

**`scripts/generate_maps.py`** (to be created):
```python
# Generates maps/nexus_siege_512.map, benchmark_open_512.map, benchmark_maze_512.map
# Run once: python3 scripts/generate_maps.py
```

The script is provided so maps can be regenerated with different seeds/parameters for varied benchmarks.

---

### 9h. Updated `ScenarioManager` to use MapLoader

Replace the key=value config format from Section 8 with direct `.map` file loading. The scenario IS the map file — all scenario parameters (width, height, tile_size, time_limit, minion_cap) are declared in the map header block.

Updated `ScenarioManager::load`:
```cpp
ScenarioConfig ScenarioManager::load(const std::string& mapPath) {
    MapData data = MapLoader::load(mapPath);
    ScenarioConfig cfg;
    cfg.mapData = std::move(data);
    // cfg fields populated from data.timeLimitSeconds, data.minionCap, etc.
    return cfg;
}
```

Remove the old `.txt` scenario files (they are superseded by `.map` files).

---

### 9i. Updated Pathfinding for Weighted Tiles

The pathfinding interfaces from Section 3 do not need to change. What changes is the cost lookup inside `DijkstrasPathfindingSystem` and `AStarPathfindingSystem`:

Replace fixed cost `1` per step with `tileMap.movementCost(nx, ny)`:

```cpp
// In neighbour expansion loop:
int cost = map.movementCost(nx, ny);
if (cost == INT_MAX) continue;  // impassable
int newG = current.gCost + cost;
```

This means Mud tiles are automatically 1.67× more expensive, so pathfinders naturally route around Mud when a Grass alternative exists. This is a **significant benchmark differentiator** between Dijkstra and A* — A* will avoid Mud more efficiently due to its heuristic guidance.

---

### 9j. Updated `TileMap.h/.cpp` — full change summary

The existing `TileMap` needs these changes (cumulative with fixes 0a–0d):

1. `enum class TileType` → 5 values (Grass, Mud, Tree, Lava, Flag)
2. `isWalkable` → renamed `isPassable` (returns false for Tree/Lava)
3. Add `movementCost(x, y)` → returns int cost (10/17/INT_MAX)
4. Add `flags_` parallel array (`std::vector<uint8_t>`) for deploy/entrance/commander-start metadata
5. Add `hasFlag(x, y, flagBit)` / `setFlag(x, y, flagBit)` accessors
6. Add `captureProgress_` array (`std::vector<float>`) for Flag tiles
7. Add `advanceCapture(x, y, dt, rate)`, `getCaptureProgress(x, y)`, `isCaptured(x, y)`
8. `getTileColor` updated for 5 types (for fallback colour rendering):
   - Grass: `(34, 139, 34)` — keep existing
   - Mud: `(139, 90, 43)` — brown
   - Tree: `(34, 80, 34)` — dark green
   - Lava: `(200, 60, 10)` — orange-red
   - Flag: `(200, 200, 50)` — gold
9. `draw(window)` updated with frustum culling (Section 9f)
10. `setTile` uses single-tile vertex update (fix 0a) and now also updates `captureProgress_` default (0.0 for non-Flag tiles)

---

### 9k. Updated Implementation Order (incorporating Section 9)

Insert map system work between step 1 (entities) and step 2 (collision) in the Revised Implementation Order:

**Revised full order:**

1. Pre-condition fixes (0a–0d) — ~2h
2. **Map system: `TileType` enum expansion + `TileMap` updates (9j) + `MapLoader` (9d) + generate_maps.py + first `.map` file** — ~4–5h. This gives a real 512×512 world to work in for all subsequent steps.
3. **Camera (`sf::View`) + frustum culling** (9f) — ~2h. Required to navigate the large map.
4. Algorithm system construction in Game + `initializeEntities()` — ~1h
5. `ICollisionSystem` + `BruteForceCollisionSystem` + Game integration — ~3–4h
6. `QuadtreeCollisionSystem` — ~4–5h. **Report-ready: collision comparison.**
7. `PlayerCommander` + movement — ~1–2h
8. `IPathfindingSystem` + `Dijkstra` + `A*` + weighted costs (9i) — ~4–5h
9. `Minion` AI + pathfinding — ~2–3h. **Report-ready: pathfinding comparison.**
10. Flag tile capture logic + win/lose (replaces NexusCore entity) — ~2h
11. Commander deploy-zone enforcement + `spawnMinion()` — ~1h
12. `TextureManager` + textured rendering (Option A, 9c) — ~4–5h
13. `Timer` + `DebugOverlay` + font — ~2–3h
14. CSV logging — ~1h
15. `ScenarioManager` using MapLoader (9h) — ~1–2h
16. Debug visualizations (F1–F3) — ~2h

**Total estimate:** ~38–50h. Prioritise steps 1–9 for Mar 15 report (benchmarkable engine with real map); steps 10–16 for Apr 19 final submission.

---

### 9l. File and Directory Changes Summary

New directories and files to create:

```
maps/
├── nexus_siege_512.map
├── benchmark_open_512.map
└── benchmark_maze_512.map

scripts/
└── generate_maps.py

src/World/
├── TileMap.h          (heavily modified)
├── TileMap.cpp        (heavily modified)
├── MapLoader.h        (new)
├── MapLoader.cpp      (new)
├── TextureManager.h   (new)
└── TextureManager.cpp (new)
```

Removed/superseded:
- `scenarios/` directory (replaced by `maps/`)
- Hardcoded `initializeTileMap()` body (replaced by `MapLoader::load()`)
- `NexusCore` entity class (replaced by Flag tile capture state in TileMap)
