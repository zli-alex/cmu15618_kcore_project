# Semantic oracle policy (Week 2+)

## Frozen reference

The directory `src/sequential` (together with its public headers under `include/sequential`) is the **semantic reference implementation** for this project. It encodes the paper-aligned batch semantics, insertion-then-deletion batch structure, invariants, and coreness estimates that downstream work must treat as authoritative unless deliberately revised through the process below.

## Changing the oracle

Any change intended to alter observable batch semantics (graph updates, level updates, descriptor fields, trace JSON fields used for validation, or mixed-batch preprocessing) is an **oracle change**, not a refactor. Such work must include:

- An update to `docs/week1_semantic_audit.md` (or its successor audit doc) explaining the delta from the paper or prior behavior.
- Targeted unit tests and/or golden fixtures under `tests/sequential` that lock the new behavior.
- Regenerated or updated tiny-trace expectations under `datasets/tiny/fixed_traces` and `results/tiny_trace_compare` as appropriate.

Treat `src/sequential` as frozen by default: parallel and performance work should not silently “fix” the oracle.

## Parallel lane

Code under `include/parallel` and `src/parallel` is the **parallel implementation lane**. It must consume the same batch and graph input model as the sequential engine (including insertion-then-deletion structure for each batch).

## Validation rule

Parallel behavior must be validated against the sequential oracle: same inputs should yield the same normalized trace outputs (modulo explicitly excluded non-semantic fields such as timing and implementation labels). Use `scripts/compare_parallel_oracle.sh` on the fixed tiny traces as a minimal gate; extend with additional traces and tests as the parallel path grows.
