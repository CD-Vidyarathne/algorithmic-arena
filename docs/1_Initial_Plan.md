# Algorithmic Arena: A Comparative Study of Data Structures & Algorithms in a 2D Game Engine

## Project Overview

This project aims to practically demonstrate the impact of different data structures and algorithms on the performance and reliability of real-time software, specifically within a 2D game engine context. We will implement multiple versions of key game systems (entity management, collision detection, pathfinding), benchmark their performance (FPS, CPU, memory), and analyze their effectiveness.

**Game Concept:** "Algorithmic Arena" - a top-down, grid-based 2D strategy/puzzle game. A player controls a commander unit and dispatches numerous minion units to navigate, interact with, and potentially collide within a dynamic environment. The game acts as a configurable testbed, prioritizing clear visual feedback and performance metrics over complex gameplay or narrative.

## Assessment Requirements & Alignment

This project is structured to meet specific academic assessment criteria:

### Assessment 1: Reflective Report (20% - Due: 15th March / Week 8)

**Focus:** Demonstrate foundational work, critical thinking, and initial project scope.

- **Project Idea & Scope:** Clearly articulate the project's purpose: comparing algorithms in game systems.
- **Aims & Objectives:** Define measurable goals (e.g., implement X vs. Y for collisions, benchmark performance metrics).
- **Initial Literature Review:** Research the importance of algorithms in games, existing methods for collision/pathfinding, and selected libraries (SFML/SDL2).
- **Initial Design:** Outline preliminary architecture for swappable algorithms and performance metric collection.
- **Reflective Aspect:** Discuss project choice, personal experience, anticipated challenges (game dev learning curve, rigorous benchmarking), mitigation strategies, and learning aspirations.

**Progress Target by 15th March:** Completion of Phase 0 (Setup) and significant progress in Phase 1 (Basic Game Framework), including initial entity rendering and movement. Ideally, a very simple algorithm (e.g., brute-force collision) should be implemented to provide concrete experiences for reflection.

### Assessment 2: Report, Poster, Artefact & Viva (80% - Due: 19th April / Week 13)

**Focus:** Full project demonstration, comprehensive analysis, and clear communication of findings.

#### Artefact (The Game/Engine)

- **Functionality:** A working 2D game engine demonstrating the implemented algorithms.
- **Algorithm Switching:** Ability to switch between different algorithm implementations (e.g., brute-force vs. quadtree) at runtime or via configuration.
- **Visualizations:** Essential for understanding.
  - Collision bounding boxes.
  - Quadtree/Spatial Hash grid boundaries.
  - Pathfinding grids, search progress (open/closed lists), and final paths.
- **Performance Overlay:** Real-time display of FPS, CPU usage, memory footprint, and algorithm-specific timings.
- **Test Scenarios:** Pre-defined maps and entity configurations to stress-test algorithms.

#### Report Structure (Following Provided Format)

- **Title page, Abstract, Acknowledgements, Dedication, Key words.**
- **Chapter 1 Introduction:** Project context, significance, and overview.
- **Chapter 1.1 Aims and Objectives:** Refined, clear, and measurable project goals.
- **Chapter 2 Market Research/Literature Review:** In-depth review of game engine architectures, collision detection (brute-force, spatial hashing, quadtrees), pathfinding (Dijkstra, A\*), benchmarking methodologies. Academic and industry references.
- **Chapter 3 Artefact Design:** Detailed technical design: overall architecture, class diagrams, data structures for algorithms, benchmarking framework design, justification of C++, SFML, CMake.
- **Chapter 4 Development & Testing:** Implementation process, challenges, solutions, detailed algorithm explanations, correctness testing, code snippets.
- **Chapter 5 Evaluation:** Crucial section. Presentation of benchmark results (graphs, tables for FPS, CPU, Memory, error rates). In-depth analysis interpreting _why_ algorithms performed as they did, linking to theoretical complexity. Evaluation against initial objectives.
- **Chapter 6 Conclusions & Further Work:** Summary of findings, implications for real-time software, personal learning reflections, suggestions for future project extensions.
- **References:** Academic referencing of all sources.
- **Appendices:** Code listings (if required), raw data, additional diagrams, user manual.

#### Poster

- **Visual Summary:** Concise display of project title, aims, methodology, key results (graphs with strong visual impact), and conclusions.
- **Images:** Screenshots of the artefact, especially algorithmic visualizations.

#### Viva

- **Demonstration:** Live demonstration of the artefact, showcasing algorithm switching and performance differences.
- **Discussion:** Answer questions on design, implementation, challenges, results interpretation, theoretical understanding, and learning outcomes.

### Progress Report Submission: End of Every Month

- **Content:** Planned vs. actual achievements, deviations, challenges, solutions, next month's plan.
- **Evidence:** Include screenshots/short videos of progress.

## Development Strategy & Phased Plan (Approx. 20 hours/week)

**Total Estimated Time:** 340-500 hours (17-25 weeks) for full project lifecycle (development, analysis, reporting).

### Phase 0: Foundation & Environment (20-40 hours / 1-2 weeks)

- **Goal:** Setup development environment, basic SFML window, stable game loop.
- **Setup:**
  - Install C++ compiler (`gcc-c++`), `make`, `cmake`, `git`.
  - Install SFML development packages (`SFML-devel`) on Fedora.
  - Create project structure (`~/algorithmic_arena/src`, `build`).
- **Initial Code:**
  - `src/main.cpp`: Simple SFML window with a drawing loop and event handling.
  - `CMakeLists.txt`: Configure CMake to find SFML and build the executable.
- **Core Utilities:** Basic `Timer` class using `<chrono>`, `Vector2D`.
- **Verification:** Successfully build and run the SFML test program. Initialize Git repository.

### Phase 1: Basic Game Framework & Entity Management (40-60 hours / 2-3 weeks)

- **Goal:** Establish a robust game loop, entity system, and tile-based world.
- **Game Loop:** Fixed-timestep logic updates, variable-timestep rendering.
- **Entity System:**
  - `Entity` base class (`position`, `size`, `color`, `update()`, `draw()`).
  - `EntityManager` (e.g., `std::vector<std::unique_ptr<Entity>>`).
  - Implement `PlayerCommander` (keyboard input) and `Minion` (simple AI, e.g., random walk).
  - **Benchmarking Target 1:** Implement a system to efficiently spawn/despawn many entities. Compare `std::vector` vs. `std::list` (or custom allocator) for entity storage and iteration.
- **Tile Grid:** `TileMap` class, load/generate grid data, render walkable/blocked tiles.
- **Debug Overlay:** Display real-time FPS, entity count.

### Phase 2: Collision Detection (80-120 hours / 4-6 weeks)

- **Goal:** Implement and benchmark multiple collision detection algorithms.
- **Collision Component:** Add `AABB` to relevant entities.
- **Algorithms:** Implement as separate classes adhering to an `ICollisionSystem` interface:
  - **Brute-Force Collision:** O(N^2) complexity.
  - **Spatial Hashing / Grid Collision:** O(N\*k) or O(N).
  - **Quadtree Collision:** O(N log N) or O(N\*log(N/k)), handles dynamic objects.
- **Visualization:** Draw collision bounding boxes for all entities. Draw grid cell boundaries for spatial hashing, and quadtree node boundaries.
- **Algorithm Switching:** In-game mechanism (e.g., hotkey) to swap collision systems.
- **Benchmarking:**
  - Measure CPU time for collision phase using `Timer`.
  - Track memory usage (approximate or via system tools).
  - Create diverse test scenarios (sparse, dense, moving objects).

### Phase 3: Pathfinding (80-120 hours / 4-6 weeks)

- **Goal:** Implement and benchmark multiple pathfinding algorithms.
- **Grid Adapter:** Convert `TileMap` to a graph structure for pathfinding.
- **PathNode Structure:** Define `PathNode` (coordinates, costs, parent).
- **Algorithms:** Implement as separate classes adhering to an `IPathfindingSystem` interface:
  - **Dijkstra's Algorithm:** Using `std::priority_queue`.
  - **A\* Search Algorithm:** Dijkstra's with a heuristic (Manhattan/Euclidean distance).
- **Visualization:** Draw calculated paths. Optionally, visualize "open" and "closed" sets during search.
- **Algorithm Switching:** In-game mechanism to swap pathfinding systems.
- **Benchmarking:**
  - Measure time taken for path requests.
  - Track number of nodes visited.
  - Track memory footprint of algorithm data structures.
  - Create diverse test maps (open, maze-like, varying obstacle density).

### Phase 4: Refinement, Analysis & Documentation (80-120 hours / 4-6 weeks)

- **Goal:** Finalize benchmarking, analyze results, and prepare all assessment deliverables.
- **Benchmarking Refinement:**
  - Automate test runs, average results over multiple iterations.
  - Integrate C++ profiling tools (e.g., `perf`, Visual Studio Profiler via VS Code/CLion).
  - Export data to CSV for external charting (e.g., Python/Matplotlib, Excel).
- **User Interface:** Simple in-game UI for metrics display, algorithm switching, and debug visualization toggles.
- **Report Writing:** Comprehensive report following the specified structure. Focus heavily on Chapter 5 (Evaluation) with clear graphs and analysis.
- **Poster Creation:** Visually summarize project, methodology, and key results.
- **Viva Preparation:** Practice demonstration and Q&A.

## Expected Performance Differences & Project Clarity

**Yes, there will be very clear and significant performance differences.** This is the core strength and clarity driver of the project.

- **Collision Detection:**
  - **Brute-Force (O(N^2))** will rapidly become a bottleneck with increasing entities, showing severe FPS drops.
  - **Spatial Hashing/Quadtree (O(N) or O(N log N))** will maintain significantly higher and more stable FPS at scale.
- **Pathfinding:**
  - **Dijkstra's (O(E + V log V), exploring broadly)** will be noticeably slower and explore far more nodes, especially in complex, maze-like environments.
  - **A\* Search (O(E + V log V), guided by heuristic)** will be significantly faster due to its targeted search, exploring fewer nodes.

**Impact on Clarity & Conclusions:**

- **Visual Proof:** Direct, undeniable visual evidence of performance improvements when switching algorithms.
- **Quantifiable Data:** Concrete numbers (FPS, CPU, Memory) will directly support visual observations and theoretical Big O complexity.
- **Strong Conclusions:** Ability to draw firm conclusions about the practical impact of algorithmic choices and inherent trade-offs (e.g., speed vs. memory).
- **Educational Value:** Powerful demonstration of theoretical computer science principles in a practical, real-time application.

## Development Environment (Fedora Linux)

- **Operating System:** Fedora Linux
- **Hardware:** NVIDIA RTX 3050 6GB, 24GB RAM, Intel Core i7 (16 CPUs)
- **Display Server:** Wayland (with XWayland compatibility layer for SFML)
- **Tools:**
  - GCC/G++ compiler
  - Make
  - CMake (for build system management)
  - Git (for version control)
  - SFML (Simple and Fast Multimedia Library for 2D graphics)
