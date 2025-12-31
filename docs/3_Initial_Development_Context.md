Development Context: "Algorithmic Arena" 2D Game Engine

This document outlines the detailed development context for the "Algorithmic Arena" project, a custom 2D game engine designed for the comparative analysis of data structures and algorithms. The core objective is to create a robust, extensible, and analytically focused platform that allows for rigorous benchmarking and clear visualization of algorithmic performance in a real-time environment. 1. Custom 2D Game Engine Architecture

The project will build a lightweight, custom 2D game engine from the ground up using C++ and SFML. This deliberate choice avoids the overhead and black-box nature of larger engines, providing full control over the implementation details crucial for a deep algorithmic study.

1.1 Core Engine Components:

Game Class: The central orchestrator, managing the main game loop, event handling, and state transitions. It will maintain ownership of EntityManager, TileMap, CollisionSystem, and PathfindingSystem instances.

Game Loop: Implemented with a fixed-timestep logic update (update(dt)) for deterministic behavior, decoupled from a variable-timestep rendering loop (render()). This ensures consistent physics and algorithm execution intervals, vital for accurate benchmarking.

EntityManager: Responsible for creating, storing, updating, and rendering all active Entity objects.

Entity Storage: Initially, std::vector<std::unique_ptr<Entity>> for simplicity. Future work could explore std::list or custom object pools for comparative analysis in entity management itself (Phase 1 target).

Entity Base Class: An abstract base class defining common properties (Vector2f position, Vector2f size, float rotation, sf::Color color) and virtual methods (update(dt), draw(sf::RenderWindow&)).

Derived Entity Types: PlayerCommander (user-controlled, keyboard input), Minion (simple AI, movement based on pathfinding, collision response), Obstacle.

TileMap: Represents the game world as a grid of tiles.

Grid Data: Stores tile properties (walkable/blocked, type) in a 2D array or std::vector<TileType>.

Rendering: Efficiently draws visible tiles using sf::VertexArray or sf::Sprite batches.

Pathfinding Integration: Provides an adapter interface to allow pathfinding algorithms to query node traversability and costs.

Rendering System: Utilizes SFML's sf::RenderWindow for drawing. Debug visualizations will be a high priority, allowing toggling of bounding boxes, grid overlays, path segments, and algorithm-specific data (e.g., Quadtree nodes, A\* open/closed lists).

Input Handling: Event-driven input processing via SFML (keyboard, mouse).

Math Utilities: Custom Vector2f (if SFML's isn't sufficient), basic geometric functions (AABB intersection).

1.2 Resource Management:

Smart pointers (std::unique_ptr, std::shared_ptr) for robust memory management.

Basic texture/font manager (if needed) to avoid redundant loading.

2. Configurable Algorithms (Build-Time Selection)

The project's core uniqueness lies in its modular algorithm implementation and build-time selection, enabling distinct performance characteristics across different game builds.

2.1 Design Pattern for Algorithmic Systems:

Strategy Pattern: Both collision detection and pathfinding systems will employ a strategy pattern.

ICollisionSystem Interface: Defines the contract for collision detection (e.g., void update(EntityManager& entities, float dt), void drawDebug(sf::RenderWindow&)).

IPathfindingSystem Interface: Defines the contract for pathfinding (e.g., std::vector<Vector2i> findPath(Vector2i start, Vector2i end, const TileMap& map)).

Concrete Implementations:

Collision: BruteForceCollisionSystem, QuadtreeCollisionSystem.

Pathfinding: DijkstrasPathfindingSystem, AStarPathfindingSystem.

2.2 Build-Time Selection Mechanism:

CMake-driven: The primary mechanism for selecting algorithms will be through CMake options.

Example: cmake -DCOLLISION_ALGORITHM=QUADTREE -DPATHFINDING_ALGORITHM=ASTAR ..

CMake will then use preprocessor directives (#define, #ifdef) or generate header files to determine which concrete CollisionSystem and PathfindingSystem implementations are compiled and linked into the final executable.

Example Game.cpp instantiation:

```
# if defined(USE_QUADTREE_COLLISION)

m_collisionSystem = std::make_unique<QuadtreeCollisionSystem>();

# elif defined(USE_BRUTE_FORCE_COLLISION)

m_collisionSystem = std::make_unique<BruteForceCollisionSystem>();

# else

// Default or error

# endif

# if defined(USE_ASTAR_PATHFINDING)

m_pathfindingSystem = std::make_unique<AStarPathfindingSystem>();

# elif defined(USE_DIJKSTRAS_PATHFINDING)

m_pathfindingSystem = std::make_unique<DijkstrasPathfindingSystem>();

# else

// Default or error

# endif
```

This ensures that only the chosen implementations are included, leading to cleaner builds for specific tests.

2.3 Algorithm Specifics:

Brute-Force Collision: A straightforward O(N^2) approach, iterating through all possible pairs of collidable entities and performing AABB (Axis-Aligned Bounding Box) intersection tests.

Quadtree Collision: A spatial partitioning technique.

Each Quadtree node will have a maximum capacity of entities before it subdivides into four child nodes.

Entities will be inserted into appropriate nodes, reducing the number of collision checks to O(N log N) or O(N \* k) in many cases.

Dynamic object handling will involve re-inserting entities if they move significantly.

Dijkstra's Algorithm: A shortest path algorithm for non-negative edge weights.

Will use a std::priority_queue to efficiently retrieve the node with the lowest known cost, ensuring optimality.

Explores outwards from the start node, considering all directions equally until the target is reached.

A\* Search Algorithm: An extension of Dijkstra's, using a heuristic function to guide the search towards the target.

f(n) = g(n) + h(n), where g(n) is the cost from start to node n, and h(n) is the heuristic estimated cost from n to target.

Heuristics will include Manhattan Distance (for grid movement) and Euclidean Distance.

Expected to significantly outperform Dijkstra's in most practical scenarios due to its informed search.

3. Automated Test Scenarios

To ensure consistency and allow for repeatable, comparable measurements, the project will incorporate automated or semi-automated test scenarios.

3.1 Scenario Definition:

Configuration Files: Test scenarios will be defined in external files (e.g., JSON, YAML, or plain text). These files will specify:

Map layout (predefined TileMap structure).

Initial entity count and types.

Distribution of entities (sparse, dense clusters, choke points).

Minion behaviors (e.g., random walk, path to specific targets, follow paths to create sustained collision).

ScenarioManager: A component responsible for loading these configurations and setting up the game state accordingly.

3.2 Automated Execution (Future Enhancement/Reporting):

While not a full CI/CD pipeline, the ability to run a game build with a specific scenario for a set duration, record metrics, and then automatically exit would be beneficial for report generation.

For the core project, "automated" primarily means predefined, repeatable setups rather than fully hands-off execution.

4. Clear Measuring Options (Benchmarking & Analysis)

Accurate and comprehensive measurement is paramount for the comparative study.

4.1 In-Game Performance Overlay:

A dedicated debug UI layer will display real-time metrics:

FPS (Frames Per Second): Using sf::Clock to measure frame time.

CPU Usage: Approximate CPU time spent in game logic vs. rendering, or overall process CPU (may require system-specific calls or external tools like perf).

Memory Footprint: Current process memory usage (requires system-specific calls or valgrind --tool=massif for more detailed heap profiling).

Entity Count: Total number of active entities.

Collision Time: Average time (in microseconds/milliseconds) taken by the CollisionSystem::update() call per frame.

Pathfinding Time: Average time per pathfinding request, total pathfinding calls per second.

Algorithm Specifics: Quadtree depth, A\* nodes expanded/visited.

4.2 Data Logging & Export:

CSV Output: Performance metrics will be logged to a CSV file over the duration of a test scenario.

This allows for external analysis using tools like Python (Matplotlib, Pandas), Excel, or R for generating professional graphs and statistical analysis in the final report.

Logging will capture timestamps, FPS, CPU, Memory, and algorithm-specific timings.

4.3 Profiling Tools Integration:

perf (Linux): Leverage Linux's perf utility for detailed CPU profiling to identify bottlenecks at the function level.

valgrind (Linux): Utilize valgrind (specifically massif for heap memory profiling) to gain deeper insights into memory allocations and usage patterns for different algorithms.

The project will include instructions and potentially scripts for setting up and running these external profiling tools alongside the game builds.

5. Easy Development and Build Routines (Best Practices)

Maintaining a clean, robust, and easily manageable codebase is essential for efficient development and academic review.

5.1 Project Structure:

Logical Directory Layout:

algorithmic_arena/
├── src/
│ ├── main.cpp
│ ├── Game.h
│ ├── Game.cpp
│ ├── Entities/
│ │ ├── Entity.h
│ │ ├── PlayerCommander.h/.cpp
│ │ ├── Minion.h/.cpp
│ │ └── EntityManager.h/.cpp
│ ├── World/
│ │ ├── TileMap.h/.cpp
│ │ └── MapLoader.h/.cpp
│ ├── Algorithms/
│ │ ├── Collision/
│ │ │ ├── ICollisionSystem.h
│ │ │ ├── BruteForceCollisionSystem.h/.cpp
│ │ │ └── QuadtreeCollisionSystem.h/.cpp
│ │ └── Pathfinding/
│ │ ├── IPathfindingSystem.h
│ │ ├── DijkstrasPathfindingSystem.h/.cpp
│ │ └── AStarPathfindingSystem.h/.cpp
│ ├── Core/
│ │ ├── Timer.h
│ │ ├── Vector2D.h // Custom math
│ │ └── Logger.h/.cpp // For CSV output
│ └── UI/
│ └── DebugOverlay.h/.cpp
├── assets/
├── build/
├── docs/
├── CMakeLists.txt
└── README.md

This structure promotes modularity and makes it easy to locate specific components.

5.2 Build System (CMake):

Cross-Platform (Primarily Omarchy): CMake will be used to manage the build process. While targeting Omarchy Linux, the CMake setup will be robust enough to handle potential cross-platform compatibility if needed.

SFML Integration: CMake will be configured to correctly find and link against SFML libraries.

Compiler Flags: Use modern C++ standards (c++20), enable warnings (-Wall -Wextra -Wpedantic), and optimize for release builds (-O3).

Easy Algorithm Selection: The CMake options (as discussed in 2.2) will be clearly documented in the README.md for simple build-time algorithm switching.

5.3 Version Control (Git):

A Git repository will be initialized from the outset.

Commit Hygiene: Regular, small, descriptive commits documenting features, bug fixes, and algorithmic implementations.

Branching Strategy: A simple main branch for stable releases and feature branches for ongoing development (feature/collision-quadtree, feature/astar-pathfinding).

5.4 Coding Standards:

Follow Google C++ Style guide. (<https://google.github.io/styleguide/cppguide.html>)

Consistent naming conventions (e.g., CamelCase for classes, snake_case for variables, m_memberVariable for member fields).

Clear code comments for complex logic, particularly within algorithm implementations.

Adherence to C++ Core Guidelines where applicable.

6. Additional Considerations:

Error Handling: Robust error checking for file loading, invalid inputs, and edge cases within algorithms (e.g., unreachable pathfinding targets).

Extensibility: Design the interfaces for ICollisionSystem and IPathfindingSystem to be easily extendable, allowing for future integration of additional algorithms (e.g., R-trees, Octrees, Jump Point Search).

Documentation: Comprehensive internal code documentation (Doxygen style comments) and external README.md for setup, usage, and build instructions.

User Manual: A basic user manual detailing how to run the different builds, interact with the game, and interpret the performance overlay will be created as part of Assessment 2.

This detailed development context provides a clear roadmap for the project, ensuring a technically sound, analytically rigorous, and well-documented comparative study.
