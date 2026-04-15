# Midterm Milestone Progress (Draft)

Project: **Parallel k-Core Decomposition with Batched Updates and Asynchronous Reads**  
Proposal goals (original):
1. Replicate ParlayLib/GBBS implementation as a reference baseline
2. Implement a sequential baseline
3. Build our own parallel update system (C++ shared-memory / OpenMP-style)
4. Evaluate performance and analyze bottlenecks

This draft is written to be **milestone-report-ready** and to point to **concrete, reproducible artifacts** in this repo.

## What we have completed so far

### Sequential baseline (semantic oracle)
- **Sequential LDS/PLDS update engine is implemented and frozen** under `src/sequential/`.
- It is treated as a **semantic oracle** for the rest of the project (we compare parallel outputs to it).
- Audit + semantics alignment work is documented in `docs/week1_semantic_audit.md`.

### Parallel update system (PLDS update path)
- **Parallel batch update engine exists** under `src/parallel` / `include/parallel`.
- Current parallelization is deliberately conservative: we parallelize **read-only scanning phases** and keep `LDSState` mutations serialized to preserve ordering/semantics (details in `docs/week2_plds_status.md`).

### Descriptor lifecycle (update-side CPLDS scaffolding)
- Update path maintains descriptor lifecycle fields (`marked`, `first_mutation_recorded`, `old_level`, root-first unmark ordering) and tests assert they finalize correctly (see `docs/week2_plds_status.md`).

### Read scaffold progress (first meaningful old-vs-live branch)
- `read_estimate(...)` supports the **first meaningful “old vs live” return branch** in the CPLDS-style control-flow scaffold (see `docs/week1_read_scaffold.md` for algorithm mapping).
- Full CPLDS linearizability protocol is not complete yet (see “What is still missing”).

### External reference wiring (integration work done; build blocked on this machine)
- We have scripts/docs to run the external Parlay/GBBS reference on tiny traces (`scripts/run_reference_trace.sh`, `scripts/run_tiny_trace_suite.sh`, `docs/tiny_trace_baseline.md`).
- On **arm64 macOS**, building/running the external binary is currently blocked due to a GBBS compile issue related to `-mcx16` (see “Blockers and risks”).

## Current architecture + validation status

### Architecture at a glance
- **Core data structures**: `seq::Graph`, `seq::LDSState`, and batch update orchestration in `src/core` + `src/sequential`.
- **Sequential engine**: `seq::SequentialBatchEngine` + `seq::process_batch`.
- **Parallel engine wrapper**: `seq::parallel::ParallelBatchEngine` wraps a `SequentialBatchEngine` and parallelizes selected scan phases, then applies mutations serially to preserve deterministic semantics (see `docs/week2_plds_status.md`).
- **Descriptor state**: `seq::DescriptorState` is shared between sequential/parallel engines; update hooks capture `old_level` on first mutation and enforce root-first unmark ordering.

### Validation status (what we check today)
We currently validate semantic parity using both **unit tests** and **trace-based regression scripts**:
- **Unit tests** (`make test`):
  - Sequential correctness and invariants
  - Descriptor lifecycle finalization
  - Parallel parity tests against sequential oracle
- **Fixed tiny traces** (scripted end-to-end JSON outputs):
  - Sequential trace runner produces `results/tiny_trace_compare/*.seq.json`
  - Parallel trace runner is compared to sequential via normalization (`scripts/compare_parallel_oracle.sh`)

## What is working end-to-end today

### End-to-end: fixed tiny traces (sequential output artifacts)
Running:

```bash
./scripts/run_tiny_trace_suite.sh
```

produces end-to-end sequential JSON outputs for the fixed trace suite under:
- `results/tiny_trace_compare/*.seq.json`

Example artifact (normalized trace output):
- `results/tiny_trace_compare/trace_triangle_insert.seq.json`

### End-to-end: sequential oracle parity vs parallel update engine
Running:

```bash
PAR_CMP_THREADS=4 ./scripts/compare_parallel_oracle.sh
```

verifies (a) **sequential == parallel@1**, and (b) **parallel@1 == parallel@N** on the full fixed trace suite, using normalized JSON comparison (strips fields like `elapsed_ms`, `num_threads`).

### End-to-end: randomized parity (seeded, reproducible)
The randomized parity test (`tests/parallel/test_parallel_random_parity.cpp`) runs **96 seeded trials** and asserts:
- sequential == parallel@1
- parallel@1 == parallel@4
- descriptor lifecycle is finalized (no remaining marks; moved vertices recorded)

## Milestone-friendly deliverables (reproducible)

All deliverables below are generated **from existing scripts/tests** on this repo state.

### Deliverable A: fixed tiny trace parity table

Data source: `results/milestone/fixed_traces_parity.log` (captured from `scripts/compare_parallel_oracle.sh`).

| Trace | Sequential run | Parallel run | Seq == Par@1 | Par@1 == Par@4 |
|---|---:|---:|---:|---:|
| `trace_triangle_insert` | ✅ | ✅ | ✅ | ✅ |
| `trace_triangle_delete` | ✅ | ✅ | ✅ | ✅ |
| `trace_path_mixed` | ✅ | ✅ | ✅ | ✅ |
| `trace_batch_only_edges` | ✅ | ✅ | ✅ | ✅ |
| `trace_star_mixed` | ✅ | ✅ | ✅ | ✅ |
| `trace_cycle4_mixed` | ✅ | ✅ | ✅ | ✅ |
| `trace_path_chord` | ✅ | ✅ | ✅ | ✅ |
| `trace_ks4_one_del` | ✅ | ✅ | ✅ | ✅ |

Notes:
- This is **semantic parity** on fixed traces under a strict normalized JSON diff (timing fields removed).
- This does **not** claim performance speedup; it claims **correctness parity** on a controlled regression suite.

### Deliverable B: randomized parity test status table

Data source: the test implementation at `tests/parallel/test_parallel_random_parity.cpp` and the successful run captured by `make test` in `results/milestone/make_test.log`.

| Test | Trials | Seed | Comparisons | Status |
|---|---:|---:|---|---:|
| `test_parallel_random_parity` | 96 | `0xC001D00D` | seq vs par@1; par@1 vs par@4; descriptor finalized | ✅ |

### Deliverable C (output-oriented): example normalized trace JSON

Data source: `results/tiny_trace_compare/trace_triangle_insert.seq.json` (produced by `scripts/run_tiny_trace_suite.sh`).

What it concretely demonstrates:
- Per-vertex `levels.before` / `levels.after`
- `coreness_estimate`
- `moved_vertices`
- Descriptor snapshot fields (`batch_number`, `old_level`)

This artifact is useful for the milestone report because it shows a **machine-checkable end-to-end result format** we can later compare against the external reference (once the build blocker is cleared).

### Optional: timing on tiny traces (not meaningful yet)

The JSON includes `summary.elapsed_ms`, but tiny traces are too small for honest performance evaluation (dominated by overhead/measurement noise). We will **not** present these as performance results; performance work is scheduled once we can run larger graphs and/or the external reference on Linux.

## Current external reference blocker (Parlay/GBBS)

We have reference integration wiring (scripts + output parser) but cannot currently build/run the external binary on **arm64 macOS** due to a GBBS compile issue involving `-mcx16`.

Current observed situation (on this machine):
- `./scripts/run_tiny_trace_suite.sh` skips reference runs unless `REF_BIN` is provided.
- `./scripts/check_week2_readiness.sh` also skips the reference smoke test unless `REF_BIN` is set.

## What is still missing (to reach proposal goals)

### Missing / incomplete technical goals
- **Full CPLDS read semantics / linearizability**:
  - complete Algorithm 3/4 behavior (true DAG-root reachability checks, root mark/unmark semantics under concurrency, retry logic under concurrent mutations).
  - currently we have the scaffold and partial old-vs-live branch, but not the full protocol.
- **External reference baseline replication**:
  - build/run Parlay/GBBS reference binary in a working environment (likely Linux x86_64).
  - produce side-by-side comparisons (at least parsed metrics; potentially deeper output parity where feasible).
- **Performance evaluation and bottleneck analysis**:
  - larger input graphs, meaningful batch sizes, repeated trials, profiling.
  - scaling curves vs threads, and identification of hotspots (e.g., scan costs vs serial apply costs, memory layout effects).

### Missing / incomplete project deliverables
- **Poster-session-ready performance plots**: not started yet (blocked on environment + scaling experiments).
- **Reproducible “one-command” Linux replication** of reference comparisons: not yet available due to lack of access to a Linux machine with the needed toolchain privileges.

## Current blockers and risks (midterm)

### Blocker: arm64 macOS external reference build
- **Symptom**: GBBS/Parlay reference build failure on arm64 macOS related to `-mcx16`.
- **Impact**: we cannot currently generate reference baseline artifacts on this machine, which blocks proposal goal (1) and delays cross-validation beyond our internal oracle.
- **Mitigation plan**:
  - move reference replication to a Linux environment (cluster machine / VM / shared lab machine) and keep macOS as a dev environment for our code.

### Risk: incomplete CPLDS read semantics
- **Status**: read path has scaffolding and a first meaningful branch, but **not** full linearizability semantics.
- **Impact**: delays proposal goal (3) completion (true asynchronous reads).
- **Mitigation plan**: prioritize minimal correct CPLDS read semantics first, then optimize.

### Risk: performance experiments not started
- **Status**: we have correctness parity and regression infrastructure, but **no scaling/perf study** yet.
- **Impact**: delays proposal goal (4).
- **Mitigation plan**: once Linux environment is available, run medium-scale datasets first to validate pipeline, then scale up.

### Risk: Linux/cluster dependency for final external replication
- **Status**: currently lacking guaranteed access to a Linux environment with required build permissions (e.g., no sudo on some machines).
- **Impact**: could compress the time window for external replication and performance evaluation.
- **Mitigation plan**: secure a Linux build path early (cluster account workflow, container approach, or prebuilt binary artifact).

## Poster-session goals (revised, milestone-grounded)

By the poster session, we want to show:
- **Correctness**: parallel update engine matches the sequential oracle on fixed traces and seeded random tests (with reproducible commands + artifacts).
- **Reference replication**: at least one successful reference binary run on Linux with parsed metrics on tiny traces (smoke), then on a medium dataset.
- **Performance**: early scaling plot(s) on at least one dataset and bottleneck explanation tied back to scan/apply split and LDS mutation serialization.

## Suggested “milestone deliverables” list (report-ready)

- **Sequential oracle**: LDS/PLDS-faithful sequential batch engine (`src/sequential`) with semantic audit doc (`docs/week1_semantic_audit.md`).
- **Parallel PLDS update engine**: shared-memory parallel scan/apply structure with deterministic parity (`src/parallel`, `docs/week2_plds_status.md`).
- **Descriptor lifecycle + read scaffold**: update-side lifecycle + read interface mapping to Algorithms 3/4 (`docs/week1_read_scaffold.md`).
- **Regression & parity suite**:
  - Fixed-trace parity gate: `scripts/compare_parallel_oracle.sh` + `results/milestone/fixed_traces_parity.log`
  - Seeded random parity: `tests/parallel/test_parallel_random_parity.cpp`
- **External reference wiring**: scripts + docs ready, currently blocked on arm64 (`docs/tiny_trace_baseline.md`, `scripts/run_reference_trace.sh`).
- **Concrete output artifacts**: JSON trace outputs under `results/tiny_trace_compare/` and milestone logs under `results/milestone/`.

