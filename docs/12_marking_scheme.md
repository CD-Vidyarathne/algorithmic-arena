# Marking Scheme: Performance Analysis

Use this rubric to **grade the quality of performance analysis** in the report, viva, or poster (adapt band weights to your module’s percentage breakdown).

**Related:** [10_finishing_up_the_artefact.md](10_finishing_up_the_artefact.md) (finishing plan), [11_maps_for_performance_analysis.md](11_maps_for_performance_analysis.md) (benchmark maps).

---

## 1. Dimensions and criteria

Each dimension is scored **0–4** (or map to % within the performance section). Total for performance analysis = average of dimensions or weighted sum as you define.

### A. Data collection rigour

| Score | Descriptor |
|-------|------------|
| **4** | Multiple independent runs or clear averaging; controlled variables stated (map, build, entity count, FPS cap); CSV or equivalent raw data referenced; avoids benchmarking with heavy debug (F2) on without disclosure. |
| **3** | Repeatable procedure; one clear scenario per comparison; minor gaps (e.g. single run) acknowledged. |
| **2** | Some numbers but weak control of variables or unclear how data was produced. |
| **1** | Anecdotal (“felt faster”) or screenshots only. |
| **0** | No empirical performance content. |

### B. Metric relevance and completeness

| Score | Descriptor |
|-------|------------|
| **4** | Connects **frame cost** (collision/path sums), **throughput** (FPS, path calls), and **scale** (entity/minion count). Explains why each metric matters for the hypothesis. |
| **3** | Covers collision and pathfinding with at least one scaling dimension; minor omission (e.g. memory) noted. |
| **2** | Only FPS or only one subsystem; thin link to algorithms. |
| **1** | Vague “performance” without tying to implemented systems. |
| **0** | No metrics. |

### C. Theoretical linkage (Big-O and practice)

| Score | Descriptor |
|-------|------------|
| **4** | Relates observations to **expected complexity** (e.g. brute-force vs quadtree under clustering; Dijkstra vs A\* on weighted/large search front). Discusses **constant factors** and **implementation** (budgeting, map size) where results deviate from theory. |
| **3** | Correct high-level complexity story with at least one nuanced point (caching, cluster density, heuristic). |
| **2** | Names complexities but weak connection to measured data. |
| **1** | Incorrect or missing complexity discussion. |
| **0** | No theory. |

### D. Visual communication (graphs, tables)

| Score | Descriptor |
|-------|------------|
| **4** | Labelled axes, units, and **which build** each series represents; readable at poster distance; captions interpret the figure, not restate numbers only. |
| **3** | Clear plots/tables; small labelling gaps. |
| **2** | Present but hard to read or missing build/scenario labels. |
| **1** | Misleading or decorative charts. |
| **0** | No figures. |

### E. Critical interpretation (limitations and fairness)

| Score | Descriptor |
|-------|------------|
| **4** | States **limitations** (single machine, SFML vs pure CPU, path budget spreading load across frames, F2 cost). Compares **fair** conditions across algorithms; acknowledges when differences are **small** or **noise**. |
| **3** | At least one honest limitation; mostly fair comparisons. |
| **2** | Overclaims or ignores obvious confounders. |
| **1** | Misleading conclusions. |
| **0** | No reflection. |

---

## 2. Quick checklist (minimum viable performance analysis)

- [ ] At least **two** collision builds compared on the **same** map and entity count, with CSV or tabulated numbers.
- [ ] At least **two** pathfinding builds compared under **stress** (large map or many path requests over time), with path time/calls discussed.
- [ ] **Build flags** or binary names stated wherever numbers appear.
- [ ] **Chapter/poster** text explains *why* results match or diverge from expectations.

---

## 3. Suggested weighting (example)

If “performance analysis” is worth **25%** of the report mark:

| Dimension | Weight |
|-----------|--------|
| A. Data collection | 20% |
| B. Metrics | 25% |
| C. Theory | 25% |
| D. Visuals | 15% |
| E. Limitations | 15% |

Adjust to your module descriptor.

---

## 4. Revision log

| Date | Change |
|------|--------|
| (add as you edit) | |
