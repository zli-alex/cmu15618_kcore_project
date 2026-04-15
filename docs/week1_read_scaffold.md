# Week 1 Read Scaffold (Algorithm 3/4 Mapping)

This note documents how the Week 1 read scaffold maps to the CPLDS read path
in *Parallel k-Core Decomposition with Batched Updates and Asynchronous Reads*
(arXiv:2401.08015v1, Algorithms 3 and 4).

## Purpose

Week 1 keeps reads sequential and finalized-state only, but provides stable
interfaces so Week 2+ can add asynchronous/linearizable behavior without API
churn.

## Current files

- `include/sequential/read_interface.h`
- `src/sequential/read_interface.cpp`
- `tests/sequential/test_read_interface.cpp`

## Interface summary

- `read_estimate(...)`:
  returns coreness estimate for a vertex.
- `find_root(...)`:
  parent-pointer traversal scaffold for dependency-DAG root lookup.
- `check_dag(...)`:
  marked/unmarked status helper scaffold.

## Algorithm 4 mapping (read)

Paper Algorithm 4 pseudocode shape:

1. `b1 = batch_number`
2. `l1 = LDS.get_level(v)`
3. `desc = desc_array[v]`
4. `status = check_DAG(desc)`
5. `l2 = LDS.get_level(v)`
6. `b2 = batch_number`
7. if `b1 != b2`: retry
8. else if `status == MARKED`: return estimate from `old_level`
9. else if `l1 == l2`: return estimate from live level
10. else: retry

Current Week 1 `read_estimate(...)` follows this same control-flow skeleton,
with bounded retry and finalized-state fallback.

## Algorithm 3 mapping (check_DAG)

Paper Algorithm 3 uses root traversal and marked/unmarked checks.

Current Week 1 mapping:

- `find_root(...)` performs parent traversal only.
- `check_dag(...)` always returns `Unmarked` in Week 1 (no active concurrent
  update visibility model yet).

This preserves the structure while deferring linearizability machinery.

## Week 1 behavior (intentional)

- Reads happen on finalized state after batch processing.
- Returned estimate is based on live level.
- Descriptor fields (`old_level`, `parent`, `batch_number`) are read-capable
  but not yet used to enforce concurrent read correctness.

## Week 2+ extension points

1. **Descriptor state model**
   - Add marked/unmarked semantics in descriptor entries.
2. **Root-aware DAG status**
   - Implement true `check_dag(...)` root traversal status logic.
3. **Batch-number sandwich correctness**
   - Keep retry loop semantics as in Algorithm 4.
4. **Old-level return path**
   - Return estimate from `old_level` when DAG/root is marked.
5. **Path compression optimization**
   - Optionally add path compression in `find_root(...)`.

## Test coverage

`tests/sequential/test_read_interface.cpp` currently verifies:

- finalized-state estimate path (`read_estimate == live-level estimate`)
- parent traversal behavior in `find_root(...)`
- Week 1 `check_dag(...) == Unmarked`

This test should remain stable while the internals evolve in Week 2+.
