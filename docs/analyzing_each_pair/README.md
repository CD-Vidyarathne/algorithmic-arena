# Analyzing each algorithm pair

Step-by-step guides for benchmarking and analysing the two **pairwise comparisons** required by the marking scheme:

| Pair | Guide |
|------|--------|
| **Quadtree vs Brute-Force** (collision) | [collision_quadtree_vs_brute.md](collision_quadtree_vs_brute.md) |
| **A\* vs Dijkstra** (pathfinding) | [pathfinding_astar_vs_dijkstra.md](pathfinding_astar_vs_dijkstra.md) |

All run commands assume the **repository root** and use **`./run.sh`**, which configures CMake, builds, and launches `AlgorithmicArena`. See `./run.sh --help` for build flags.

**Shared references**

- `docs/Analyzation_guide.md` — full pipeline, pseudocode, thesis checklist  
- `docs/Performance Analyze.md` — fair controls and benchmark matrix  
- `docs/Algorithm_Performance_Marking_Scheme.md` — rubric  
- `scripts/analyze_benchmark_csv.py` — CSV analysis after each run  
