# Project Master Summary: "Algorithmic Arena: The Nexus War"

## 1. Project Overview

**Algorithmic Arena** is a dual-purpose project: it is both a playable **Real-Time Strategy (RTS)/Puzzle game** and a rigorous **comparative analysis testbed** for data structures and algorithms.

The core objective is to empirically demonstrate how algorithmic choices (specifically in collision detection and pathfinding) impact real-time software performance. Unlike standard games, the "engine" is designed to be modular, allowing different algorithms to be swapped in at **build-time** to create distinct performance benchmarks.

## 2. Game Concept: "The Nexus War"

The game serves as the "stress test" environment. The efficiency of the algorithms directly dictates the maximum unit count and frame rate stability the player experiences.

- **Genre:** Top-down, Grid-based RTS.
- **Premise:** You are a **Commander** controlling a swarm of **Minions** to capture and defend **Nexus Cores** in a hostile environment.
- **Key Mechanics:**
  - **Commander:** The player-controlled unit. Can move around the map and deploy
  - **Minions:** Autonomous AI units. They pathfind to objectives and require collision avoidance. Target is capturing the flag(Nexus Cores). The player spawns these to overwhelm the map.
  - **Nexus Cores:** Static objectives to capture.
  - **Obstacles:** Static walls and dynamic environmental hazards.
- **Win Condition:** Capture and hold all Nexus Cores.
- **Lose Condition:** Run out of Minions/Resources or fail to capture within a time limit.

## 3. Technical Architecture

You are building a lightweight, custom 2D engine from scratch to avoid the "black box" overhead of commercial engines.

- **Language & Tools:** C++ (C++20 standard), SFML (Graphics/Input), CMake (Build System).
- **Core Loop:** Fixed-timestep logic updates (`update(dt)`) for deterministic physics, decoupled from variable-timestep rendering (`render()`).
- **Entity Management:** `EntityManager` using `std::vector<std::unique_ptr<Entity>>` (initially) with plans for object pooling.
- **World:** `TileMap` system representing the grid, providing data for pathfinding adapters.

## 4. The Algorithmic Core (The Study)

The project focuses on comparing two specific systems. These are selected via **CMake options** (e.g., `-DCOLLISION_ALGORITHM=QUADTREE`) to compile different versions of the game.

### A. Collision Detection Systems

- **Brute-Force:** $O(N^2)$ complexity. Checks every entity against every other entity. Serves as the baseline.
- **Quadtree:** Spatial partitioning. Recursively subdivides the screen into quadrants. Reduces checks to roughly $O(N \log N)$.

### B. Pathfinding Systems

- **Dijkstra’s Algorithm:** Uninformed search. Explores all directions equally to find the shortest path based on edge weights.
- **A\* (A-Star) Search:** Informed search using Heuristics (Manhattan or Euclidean distance). Prioritizes nodes closer to the target.

## 5. Benchmarking & Visualization

The engine is built to measure and visualize the differences between the algorithms above.

- **Visual Debugging:**
  - Toggles for bounding boxes, Quadtree grid subdivisions, and Pathfinding open/closed lists.
- **In-Game Overlay:**
  - Real-time stats: FPS, Entity Count, CPU Usage, Memory Usage.
  - Algorithm specifics: Collision time per frame, Pathfinding time per request.
- **Data Logging:**
  - Metrics are logged to **CSV files** during automated test scenarios for external analysis (Python/Excel).
  - **Profiling:** Integration instructions for `perf` (CPU) and `valgrind/massif` (Memory).

## 6. Development Roadmap & Standards

- **Project Structure:** Modular layout separating `Core`, `Entities`, `Algorithms`, and `UI`.
- **Automation:** `ScenarioManager` will load test definitions (JSON/YAML) to ensure repeatable tests (e.g., "Spawn 500 minions in a maze layout").
- **Design Patterns:**
  - **Strategy Pattern:** Used for `ICollisionSystem` and `IPathfindingSystem` interfaces.
- **Coding Standards:** Google C++ Style Guide.

## Summary of Goals

1. **Build** a functional custom 2D engine.
2. **Implement** modular Collision and Pathfinding systems.
3. **Visualize** the internal logic of these algorithms.
4. **Benchmark** their performance under stress (high entity counts).
5. **Prove** the importance of algorithmic complexity in real-time systems through data.
