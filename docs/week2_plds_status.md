# Week 2 PLDS + update-side CPLDS descriptor status

This document summarizes the **parallel batch update engine** under `include/parallel` and `src/parallel`, aligned with the frozen sequential oracle in `src/sequential` and the PLDS/LDS scheduling discipline in *Parallel k-Core Decomposition with Batched Updates and Asynchronous Reads* (arXiv:2401.08015v1).

## What is parallelized

### Insertion phase (`parallel_insertion.cpp`)

- **Per-level increasing sweep** `l = 0 .. K-1`, same outer structure as `src/sequential/insertion_phase.cpp`.
- **Within each level `l`**: a **read-only violation scan** for Invariant 1 over the active bucket’s vertices, partitioned across worker threads by index ranges on the snapshot list.
- Workers only append to **thread-local** vertex lists; after `join`, lists are concatenated and **sorted** so the serial promotion pass matches oracle ordering discipline.

### Deletion phase (`parallel_deletion.cpp`)

- **Mover discovery** at a fixed target level `l`: parallel **read-only** scan over vertex ids `0 .. n-1` (partitioned by ranges), filtering with the current round’s `desire[]`, `moved[]`, and `state.level`.
- Thread-local id lists are merged and **sorted** to match the sequential oracle’s increasing-vertex scan order.

## What remains serial

### Insertion

- Applying all edge **insertions** from the batch (before the level sweep).
- **Copy + sort** of `vertices_in_level(l)` into `active_level_snapshot`.
- **All** `LDSState::set_level` / promotions, descriptor mutation hook (`on_level_mutation_start`), and promotion counters.

### Deletion

- Edge **removals** and initial `touched` / `desire` setup.
- **Entire** inner deletion round: demotions, `moved` / `desire` updates, and **neighbor `recompute_desire` loops** — kept textually aligned with `src/sequential/deletion_phase.cpp` iteration order.

### Batch orchestration (`parallel_batch_engine.cpp`)

- Same structure as `src/sequential/sequential_batch_engine.cpp`: `partition_insertions_first`, level snapshots for metrics, invariant checks, `moved_vertices` sweep, `estimate_all_coreness`.

**Rationale:** `LDSState` maintains level buckets with non–thread-safe updates (`lds_state.cpp`). Parallelism is intentionally restricted to phases that do not mutate shared LDS structures concurrently.

## Snapshot and round semantics (explicit)

### Insertion

- **Active-level snapshot**: at the start of processing level `l`, the engine copies `state.vertices_in_level(l)` into a local vector and sorts it ascending. That list defines the vertex set considered for level `l` in this sweep (oracle: candidates from bucket `l`).
- **Scan phase**: only reads `LDSState` / `Graph` for Invariant 1 evaluation; **no** `set_level`, no descriptor updates, no graph edge changes.
- **Apply phase**: runs on one thread after all scan workers finish.

### Deletion

- **Round boundary for mover collection**: mover lists are built only when no worker is mutating `LDSState` — specifically after any previous demotion round and its serial neighbor recomputes have completed, so `desire`, `moved`, and levels are consistent.
- **Mover scan phase**: read-only with respect to `desire`, `moved`, and `state.level`.
- **Demotion / recompute**: serial.

See the block comments at the top of `src/parallel/parallel_insertion.cpp` and `src/parallel/parallel_deletion.cpp`.

## Update-side CPLDS descriptor lifecycle (implemented)

- `DescriptorState` now carries explicit byte-like lifecycle fields:
  - `marked` (`std::vector<uint8_t>`, 0/1)
  - `first_mutation_recorded` (`std::vector<uint8_t>`, 0/1)
  - `last_unmark_order` for test observability
- On each first level mutation in a batch, `on_level_mutation_start(v, level_before)`:
  - captures `old_level[v] = level_before` exactly once
  - marks the vertex
  - updates parent/root linkage using deterministic serial mutation order
- End of batch performs `unmark_all_roots_first()`:
  - roots are unmarked first (sorted by id), then non-roots (sorted by id)
  - tests assert this ordering
- Final moved-vertex sweep in sequential/parallel engines no longer performs descriptor mutation fallback; it uses debug assertions to guarantee phase hooks captured every moved vertex.

## Parity guarantees (current validation)

Observables checked against the sequential oracle (same inputs, same `LDSConfig`):

- Final **per-vertex levels** (`levels_before` / `levels_after` in `BatchResult`).
- **Promotion / demotion / fixup** counters and directional move counts.
- **Invariant satisfaction flags** after insertions and after deletions.
- **`moved_vertices`** list (increasing vertex order).
- **`coreness_estimate`** (bitwise via tight tolerance in tests).
- **Normalized trace JSON** parity via `scripts/compare_parallel_oracle.sh` (strips `implementation`, `elapsed_ms`, `num_threads`).
- Descriptor lifecycle invariants in unit/parity tests:
  - pre-move `old_level` capture
  - marked-to-unmarked transition by end of batch
  - root-first unmark ordering

**Guarantees:**

- `parallel @ 1 thread` matches the sequential oracle on all **fixed tiny traces** in `datasets/tiny/fixed_traces/` listed by that script, and on the **seeded randomized** scenarios in `tests/parallel/test_parallel_random_parity.cpp` (`seq` vs `par@1` vs `par@4`).
- `parallel @ N threads` matches `parallel @ 1 thread` on those fixed traces (same normalization), for default `N` in the script (configurable via `PAR_CMP_THREADS`).

**Not guaranteed (out of scope for Week 2):** bitwise identity of floating-point outputs across compilers/platforms beyond current test tolerance; linearizability of reads under concurrent updates; full CPLDS descriptor DAG union/merge semantics.

## Deferred before full read-path CPLDS work

Per project plan, the following are **not** implemented in Week 2:

- Lock-free or asynchronous **reads**; reader/writer coordination.
- Full **CPLDS** descriptor DAG lifecycle: DAG union/merge policy, complete Algorithm 3/4 linearizability protocol, and lock-free retry behavior.
- Parallelizing **`LDSState::set_level`** without a scan/apply split (would need locking, epochs, or structural redesign).
- **SNAP-scale** performance tuning, memory-layout optimization, and work-stealing schedulers.

## How to run checks

- Unit + parallel tests: `make test` (includes `test_parallel_trace_parity`, `test_parallel_random_parity`).
- Fixed trace JSON gate: `./scripts/compare_parallel_oracle.sh` (optional `PAR_CMP_THREADS`).
- Broader readiness: `./scripts/check_week2_readiness.sh`.

## Fixed regression traces (`datasets/tiny/fixed_traces/`)

Shell suites (`compare_parallel_oracle.sh`, `run_tiny_trace_suite.sh`) include the original three traces plus:

| Trace | Intent |
|------|--------|
| `trace_batch_only_edges` | Empty initial graph file; batch inserts build a triangle from scratch. |
| `trace_star_mixed` | Star graph; mixed insert + delete on leaves / hub. |
| `trace_cycle4_mixed` | 4-cycle; delete + insert rewires the ring. |
| `trace_path_chord` | Path 0–1–2–3; chord insert closes a quadrilateral. |
| `trace_ks4_one_del` | Clique on four vertices (K4); single deletion stresses dense deletion scheduling. |
