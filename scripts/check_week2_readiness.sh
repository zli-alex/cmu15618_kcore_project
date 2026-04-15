#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[check] oracle policy doc"
test -f docs/oracle_policy.md

echo "[check] semantic audit doc"
test -f docs/week1_semantic_audit.md

echo "[check] make test"
make test

echo "[check] tiny trace suite (sequential + optional reference)"
./scripts/run_tiny_trace_suite.sh

echo "[check] sequential vs parallel (1 thread) on fixed tiny traces"
./scripts/compare_parallel_oracle.sh

if [[ -n "${REF_BIN:-}" ]]; then
  echo "[check] REF_BIN is set; requiring one successful reference trace"
  if [[ ! -x "$REF_BIN" ]]; then
    echo "REF_BIN is not executable: $REF_BIN" >&2
    exit 1
  fi
  graph="$ROOT_DIR/datasets/tiny/fixed_traces/trace_triangle_insert.graph"
  batch="$ROOT_DIR/datasets/tiny/fixed_traces/trace_triangle_insert.batch"
  out="$(mktemp "${TMPDIR:-/tmp}/week2_ref_trace.XXXXXX.json")"
  cleanup() { rm -f "$out"; }
  trap cleanup EXIT
  REF_BIN="$REF_BIN" "$ROOT_DIR/scripts/run_reference_trace.sh" "$graph" "$batch" "$out"
  echo "[ok] reference trace succeeded with REF_BIN=$REF_BIN"
else
  echo "[skip] REF_BIN not set; skipping external reference binary gate"
fi

echo "[ok] week2 readiness checks passed"
