# Project Status: Algorithmic Arena (Phase: World & Rendering)

## 1. Current Technical Architecture

The project is a C++20 custom game engine using SFML for rendering and CMake for build management. The foundation for the game loop, window management, and efficient tile-based rendering is complete.

### Directory & File Structure

- **Root:** `CMakeLists.txt`, `run.sh`
- **src/**
  - `main.cpp`: Entry point. Initializes `spdlog` and runs the `Game` instance.
  - `Game.h / .cpp`: Core engine class. Manages `sf::RenderWindow`, the main loop, and ownership of the world.
  - **World/**
    - `TileMap.h / .cpp`: Handles grid data and rendering.
  - **Util/**
    - `Logger.h`: (Inferred) Wrapper for `spdlog`.
  - **Entities/** (Placeholder): Intended for Entity base classes and implementations.
  - **Algorithms/** (Placeholder): Intended for Collision and Pathfinding logic.
  - **Core/** (Placeholder): Intended for timing and math utilities.
  - **UI/** (Placeholder): Intended for debug overlays.

### Build System (CMake)

- **Standard:** C++20.
- **Dependencies:**
  - **SFML:** Found via PkgConfig (system, window, graphics).
  - **spdlog:** Found via `find_package`.
- **Build Options (Configurable Logic):**
  - `USE_QUADTREE_COLLISION` (Default: ON) $\rightarrow$ Defines `USE_QUADTREE_COLLISION` or `USE_BRUTE_FORCE_COLLISION`.
  - `USE_ASTAR_PATHFINDING` (Default: ON) $\rightarrow$ Defines `USE_ASTAR_PATHFINDING` or `USE_DIJKSTRAS_PATHFINDING`.
- **Compilation:** Enables `-Wall -Wextra -Wpedantic`. Release builds use `-O3`.

### Class Breakdown

#### 1. `Game` Class

- **Role:** Application orchestration.
- **Members:** `sf::RenderWindow`, `std::unique_ptr<TileMap>`.
- **Flow:**
  1. Initializes window (1280x720, 60 FPS limit).
  2. Calls `initializeTileMap()` to generate a test grid.
  3. Runs `processEvents`, `update(dt)`, and `render` loop.
- **Current State:** `update(dt)` is currently empty.

#### 2. `TileMap` Class

- **Role:** Stores grid data and handles batch rendering.
- **Data Structure:** Flattened 1D `std::vector<TileType>` representing a 2D grid. Access logic: `index = y * width + x`.
- **Rendering:** Uses a single `sf::VertexArray` (`sf::PrimitiveType::Triangles`) to draw the entire map in one draw call (Performance optimized).
  - Each tile consists of 6 vertices (2 triangles).
- **Functionality:**
  - `setTile`: Updates the logical type and immediately updates the vertex colors.
  - `getTile / isWalkable`: Safety-checked accessors.
  - `getTileColor`: Maps `Walkable` to Green and `Blocked` to Dark Grey.

## 2. Functionality Checklist

- [x] **Project Setup:** CMake configuration with SFML/spdlog linking.
- [x] **Build Variants:** Preprocessor definitions set up for switching algorithms.
- [x] **Windowing:** SFML window creation and event polling (close).
- [x] **World Data:** Grid storage implemented (1D vector).
- [x] **Rendering:** Efficient VertexArray batch rendering for the map.
- [x] **Test Map:** Hardcoded test scenario (borders and a small obstacle block).
- [ ] **Game Loop Timing:** Delta time (dt) calculation is missing in `main` or `Game::run`.
- [ ] **Entities:** No Player or Minion classes exist yet.
- [ ] **Camera:** No `sf::View` implementation (world is fixed to window size).

---

## 3. Immediate Development Roadmap

To move from a static map renderer to an interactive engine, follow these steps in order:

### Step 1: Fix the Game Loop (Delta Time)

The current `Game::run()` loop calls `update(dt)` but does not calculate `dt`.

- **Action:** Instantiate an `sf::Clock` in the `run` method.
- **Logic:** Calculate `dt = clock.restart().asSeconds()` at the start of every frame and pass it to `update()`. This is crucial for frame-rate independent movement later.

### Step 2: Entity System Foundation

We need objects to inhabit the world.

- **Action:** Create `src/Entities/Entity.h` (Abstract Base Class).
- **Properties:** Position (`sf::Vector2f`), Velocity, Size, Color.
- **Virtual Methods:** `update(float dt)`, `render(sf::RenderWindow&)`, `getBounds()`.
- **Manager:** Create `src/Entities/EntityManager.h` to hold a `std::vector<std::unique_ptr<Entity>>`.

### Step 3: Player Implementation (Commander)

Implement the user-controlled character.

- **Action:** Create `PlayerCommander` inheriting from `Entity`.
- **Input:** Inside `PlayerCommander::update`, check `sf::Keyboard` states (WASD/Arrows) to set velocity.
- **Integration:** Instantiate the Player in `Game`, update it, and render it.

### Step 4: Basic Collision Interface (Preparation for Algorithms)

Before the player walks through walls, set up the structure for the comparative study.

- **Action:** Create `src/Algorithms/Collision/ICollisionSystem.h`.
- **Interface:** Define a pure virtual method `update(EntityManager& entities, const TileMap& map)`.
- **Implementation:** Create `BruteForceCollisionSystem` (initially just checking Player vs TileMap bounds/obstacles).

### Step 5: Connect Collision to Game

- **Action:** In `Game.h`, add a `std::unique_ptr<ICollisionSystem>`.
- **Logic:** In the constructor, use the `#ifdef USE_QUADTREE_COLLISION` macros to instantiate the specific system (even if Quadtree isn't written yet, map it to BruteForce temporarily or leave it null). Call the collision system in `Game::update`.

---

**Instruction for LLM:** Use the context above to assist the user in implementing **Step 1 (Delta Time)** and **Step 2 (Entity System Foundation)**. Maintain the existing code style (Google C++) and folder structure.
