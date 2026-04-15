# Week 1 Semantic Audit (Paper vs Current Code)

This audit compares the current Week 1 sequential baseline against the exact semantics in *Parallel k-Core Decomposition with Batched Updates and Asynchronous Reads* (arXiv:2401.08015v1).

## Reference points from the paper

- LDS/PLDS semantics and invariants: levels/groups and invariant definitions in `2401.08015v1.pdf` (see lines around `274-359`).
- Insertion-phase discipline ("increasing order", one-pass-per-level property): `2401.08015v1.pdf` (see lines around `338-345`).
- Deletion-phase desire-level discipline ("move once", no later move to `<= l`): `2401.08015v1.pdf` (see lines around `346-354`).
- CPLDS descriptor/read semantics (`parent`, `old_level`, `batch_number`, root-marked check, old-vs-live read behavior): `2401.08015v1.pdf` (see lines around `470-540` and `650-709`).

## Audit table

| Paper requirement | Current implementation | Status | Action needed before Week 2 |
|---|---|---|---|
| **LDS uses groups; invariants defined over induced neighborhoods in `Z_l` and `Z_{l-1}` with paper thresholds** (`2401.08015v1.pdf`, ~`311-325`) | `check_invariant1_vertex` and `check_invariant2_vertex` in `src/sequential/invariants.cpp` now compute induced degrees over `Z_l` / `Z_{l-1}` and enforce thresholds `(2+3/lambda)(1+delta)^i` and `(1+delta)^i` directly. | **Exact match (sequential)** | Keep regression tests around threshold boundaries; this logic is now ready to be reused in Week 2 parallel scheduling. |
| **Insertions only violate Invariant 1; deletions only violate Invariant 2** (`2401.08015v1.pdf`, ~`306-309`) | `apply_insertions` now promotes only on Invariant 1 violations; `apply_deletions` computes desire levels and demotes through Invariant 2 logic (`src/sequential/insertion_phase.cpp`, `src/sequential/deletion_phase.cpp`). | **Exact match (sequential)** | Keep this directional split strict; do not reintroduce cross-direction fixups outside phase schedulers. |
| **Insertion phase: process levels in increasing order; after processing level `l`, no future insertion step moves a vertex up from level `l`** (`2401.08015v1.pdf`, ~`338-345`) | `apply_insertions` is now an explicit increasing-level sweep (`l=0..K-1`) and only promotes vertices currently in the active level, so level `l` is closed once processed. | **Exact match (sequential)** | Parallelization can later add per-level worklists, but must preserve this ordering guarantee. |
| **Deletion phase: compute desire level; move to it once; after level `l`, no vertex later wants to move to `<= l`** (`2401.08015v1.pdf`, ~`346-354`) | `apply_deletions` now computes per-vertex desire levels, processes levels in increasing order, moves each vertex at most once, and recomputes neighbors with `min_allowed = l+1` to preserve monotonicity. | **Exact match (sequential)** | Preserve one-move-per-vertex and monotone frontier constraints in Week 2 parallel form. |
| **Batch update structure: batch can be mixed but conceptually split into insertion and deletion sub-batches** (`2401.08015v1.pdf`, ~`234-237`, `334-337`) | `partition_insertions_first` exists (`src/sequential/batch_update.cpp`) and `process_batch` runs insertion then deletion (`src/sequential/sequential_batch_engine.cpp`). | **Exact match (Week 1 intent)** | Keep this structure; later add stricter preconditions and tests that assert deterministic split behavior for duplicate/conflicting updates. |
| **Coreness estimate follows PLDS/LDS definition from level/group mapping (Definition 3.1)** (`2401.08015v1.pdf`, ~`355-359`) | `estimate_coreness_from_level` now implements Definition 3.1 directly in `src/sequential/coreness.cpp`, including `4*ceil(log_{1+delta}(n))` group span and `(1+delta)^exponent`. | **Exact match (sequential)** | Keep returning floating estimates; add more golden tests for larger `n` / higher levels as needed. |
| **Descriptor carries `parent` and `old_level`; global `batch_number` increments at batch start** (`2401.08015v1.pdf`, ~`490-498`, `521-528`) | `DescriptorState` has `parent`, `old_level`, `batch_number`; `begin_batch` increments batch number and snapshots old levels (`include/sequential/descriptor_state.h`, `src/sequential/descriptor_state.cpp`). | **Exact match (scaffold only)** | Keep fields stable; next step is to make parent/root semantics meaningful for dependency DAGs instead of placeholders. |
| **CPLDS read semantics rely on DAG root reachability + marked/unmarked root + batch sandwich + old-vs-live decision** (`2401.08015v1.pdf`, ~`630-709`) | Week 1 has no async read path, no check_DAG traversal, no root-marking protocol, and no batch-number/read-level sandwich logic. | **Intentionally missing (deferred)** | Week 2+: add descriptor state machine (marked/unmarked), root-first unmark protocol, DAG-root reachability checks, and read algorithm equivalent to Algorithm 4 semantics. |
| **Marking/unmarking order invariant: root marked before non-roots and unmarked before non-roots** (`2401.08015v1.pdf`, ~`605-613`) | Current code tracks touched vertices and sets `parent[v]=v` during final sweep (`src/sequential/sequential_batch_engine.cpp`), but does not implement marking/unmarking protocol. | **Missing (deferred)** | Introduce explicit marked state and unmark phases (roots first, then non-roots) to prepare for linearizable read behavior. |

## Bottom line

The current Week 1 code is now a **sequential LDS/PLDS-faithful baseline** for invariants, insertion scheduling, deletion desire-level scheduling, and Definition 3.1 coreness mapping. The remaining semantic gap is concentrated in **CPLDS concurrency/read linearizability mechanics**.

## Priority fixes before Week 2 concurrency work

1. Keep the sequential semantics frozen as oracle behavior (tests + traces).
2. Implement descriptor marked/unmarked lifecycle and root-first unmarking protocol.
3. Upgrade `check_dag`/`read_estimate` scaffold to Algorithm 3/4 concurrent semantics.
4. Add reference-comparison runs once the local Parlay baseline binary is wired.

## Pre-Week2 readiness checklist

- [x] Boundary/off-by-one tests added for:
  - Invariant 2 at `l=0`
  - Invariant 1 at top level
  - desire-level dropping to `0`
  - multi-level insertion climbs
- [x] Mixed-batch preprocessing tests added for:
  - duplicate insertions
  - duplicate deletions
  - insert+delete of same edge
  - self-loop input handling
- [ ] One successful side-by-side tiny-trace run against the wired reference binary

When the final unchecked box is completed, Week 2 should start from:
**“sequential update semantics trusted.”**
