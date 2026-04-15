# Reference Wiring Quick Guide

This guide covers lightweight wiring of the external Parlay/GBBS reference
binary into the tiny-trace workflow.

## 1) Set `REF_BIN`

Point `REF_BIN` to the external executable:

```bash
export REF_BIN=/absolute/path/to/reference_binary
```

If unset, scripts default to:

- `third_party/reference_impl/reference_runner`

## 2) Input and output assumptions

Inputs used by helper scripts:

- graph file: `u v` per line
- batch file: `I u v` or `D u v` per line

Reference parser assumptions:

- parsed metrics come from lines in the form `### Key: Value`
- unknown formats are preserved in `raw_output` but not normalized

Normalized output JSON (`*.ref.json`) includes:

- `summary.exit_status`
- `summary.elapsed_ms`
- `summary.parse_status` (`ok` or `incompatible_output_schema`)
- `summary.failure_reason` (when applicable)
- parsed `batches` and `totals`
- `raw_output`

## 3) One exact smoke command

```bash
REF_BIN=/absolute/path/to/reference_binary ./scripts/check_reference_smoke.sh
```

This runs one fixed trace (`trace_triangle_insert`) and prints compact PASS/FAIL.

## 4) Run tiny trace suite with reference enabled

```bash
REF_BIN=/absolute/path/to/reference_binary ./scripts/run_tiny_trace_suite.sh
```

Optional strict mode (fail fast on any reference trace failure):

```bash
REF_BIN=/absolute/path/to/reference_binary REF_REQUIRED=1 ./scripts/run_tiny_trace_suite.sh
```

## 5) Readiness gate

```bash
REF_BIN=/absolute/path/to/reference_binary ./scripts/check_week2_readiness.sh
```

When `REF_BIN` is set, readiness requires one successful smoke run.
When `REF_BIN` is unset, external reference gate is skipped.

## 6) Comparable vs not comparable today

Currently comparable:

- process success/failure
- parsed reference batch/total metrics when present
- final coreness estimate when emitted by reference parser path

Not directly comparable yet:

- per-vertex levels (`levels.before`, `levels.after`)
- `moved_vertices`
- exact schema parity with `seq_trace_runner` JSON
