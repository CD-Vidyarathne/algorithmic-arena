# Finishing Up the Artefact

This note captures the agreed **finishing plan** and explicitly excludes changes to **game timer behaviour** (keep current design).

**Marking rubric:** [12_marking_scheme.md](12_marking_scheme.md)  
**Benchmark map plan:** [11_maps_for_performance_analysis.md](11_maps_for_performance_analysis.md)

---

## 1. Scope: game timer

**Decision:** Leave the **mission / game timer** as implemented now.

- `MapLoader` may still parse `timeLimitSeconds` from map headers for **documentation or future use**; the playable loop does **not** need to add a lose condition when that limit is exceeded unless you later change your mind.
- HUD may optionally show **elapsed** `gameTimer_` for player feedback; do **not** treat “time ran out” as loss unless you revise this document.

---

## 2. Finishing plan (artefact + deliverables)

### 2.1 Artefact hardening (code & repo)

| Priority | Item | Notes |
|----------|------|--------|
| P0 | **README** | Dependencies (SFML, CMake, spdlog), build, CMake flags (`USE_QUADTREE_COLLISION`, `USE_ASTAR_PATHFINDING`), CLI (`--map`, `--csv`, `--unlimited-fps`), controls summary, asset/font expectations. |
| P0 | **Benchmark repeatability** | Document or script a fixed matrix: builds × maps × minion counts → CSV under e.g. `results/`. Same seed/order where relevant. |
| P1 | **Docs hygiene** | `docs/6_1902_Process.md` is historical; `docs/9` frontmatter todos may be stale—update or annotate so markers are not misled. |
| P1 | **Viva demo script** | Two binaries (collision pair + pathfinding pair), one stress scenario each, F1/F2 off for fair FPS during timed runs. |
| P2 | **Optional polish** | F3 grid overlay; small regression checks on tiny grids—only if time remains. |

### 2.2 Assessment-aligned outputs (outside or beside the binary)

| Deliverable | Link to artefact |
|-------------|------------------|
| **Report Ch. 5** | Graphs/tables from CSV: `collision_us_sum_1s`, `pathfinding_us_sum_1s`, `path_calls_sum_1s`, `fps`, `entity_count`, `minion_count`. |
| **Poster** | Screenshots: map, overlay off/on, quadtree or path debug sparingly. |
| **User notes / appendix** | How to run each build and interpret the overlay and CSV columns. |

### 2.3 Explicitly out of scope (for this finishing pass)

- **No** mission-timer-based lose condition (see §1).
- **No** requirement to add runtime algorithm switching; build-time variants remain the design—viva uses multiple executables.

---

## 3. Revision log

| Date | Change |
|------|--------|
| (add as you edit) | |
