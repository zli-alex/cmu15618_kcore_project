# Tiny Trace Baseline Workflow

This document defines a lightweight baseline comparison workflow between:

- Week 1 sequential engine (`seq_trace_runner`)
- Parlay/GBBS reference implementation (via `scripts/run_reference.sh`)

## Fixed traces

- `datasets/tiny/fixed_traces/trace_triangle_insert.*`
- `datasets/tiny/fixed_traces/trace_triangle_delete.*`
- `datasets/tiny/fixed_traces/trace_path_mixed.*`
- `datasets/tiny/fixed_traces/trace_batch_only_edges.*`
- `datasets/tiny/fixed_traces/trace_star_mixed.*`
- `datasets/tiny/fixed_traces/trace_cycle4_mixed.*`
- `datasets/tiny/fixed_traces/trace_path_chord.*`
- `datasets/tiny/fixed_traces/trace_ks4_one_del.*`

Each trace has two files:

- `.graph`: initial edge list (`u v` per line)
- `.batch`: update list (`I u v` or `D u v`)

## Run all traces

```bash
scripts/run_tiny_trace_suite.sh
```

With external reference enabled:

```bash
REF_BIN=/absolute/path/to/reference_binary scripts/run_tiny_trace_suite.sh
```

Outputs are written to:

- `results/tiny_trace_compare/*.seq.json`
- `results/tiny_trace_compare/*.ref.json`

## Normalized dump format

Sequential outputs (`*.seq.json`) include:

- `levels.before`
- `levels.after`
- `coreness_estimate`
- `moved_vertices`
- `descriptor.old_level`
- timing and move summary

Reference outputs (`*.ref.json`) include:

- normalized metadata fields
- parsed `batches` entries from `### ...` lines (e.g., coreness, flips, batch times)
- parsed `totals` fields when present (`total_time`, latency, throughput, etc.)
- `raw_output` (captured stdout/stderr)
- placeholder `levels/moved_vertices` fields (`null` for now)

Reference parse assumptions:

- input graph: `u v` per line
- input batch: `I u v` or `D u v` per line
- parsed metrics: reference output lines like `### Key: Value`

Failure modes:

- missing/non-executable reference binary
- non-zero reference process exit
- incompatible output schema (no recognized `### Key: Value` metrics)

This is intentionally lightweight: the goal is side-by-side monitoring and
semantic drift detection, not full automated equivalence of all fields.
