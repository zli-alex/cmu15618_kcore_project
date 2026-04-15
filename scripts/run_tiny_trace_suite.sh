#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${1:-$ROOT_DIR/results/tiny_trace_compare}"
mkdir -p "$OUT_DIR"

traces=(
  "trace_triangle_insert"
  "trace_triangle_delete"
  "trace_path_mixed"
)

for trace in "${traces[@]}"; do
  graph="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.graph"
  batch="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.batch"

  "$ROOT_DIR/scripts/run_seq_trace.sh" "$graph" "$batch" "$OUT_DIR/${trace}.seq.json"

  if "$ROOT_DIR/scripts/run_reference_trace.sh" "$graph" "$batch" "$OUT_DIR/${trace}.ref.json"; then
    echo "[ok] reference trace: $trace"
  else
    echo "[warn] reference trace failed for $trace (see .ref.json raw_output)"
  fi
done

echo "Wrote trace outputs to: $OUT_DIR"
