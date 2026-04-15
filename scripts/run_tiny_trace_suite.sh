#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${1:-$ROOT_DIR/results/tiny_trace_compare}"
mkdir -p "$OUT_DIR"
REF_REQUIRED="${REF_REQUIRED:-0}"
REF_ENABLED=0
if [[ -n "${REF_BIN:-}" ]]; then
  REF_ENABLED=1
fi

traces=(
  "trace_triangle_insert"
  "trace_triangle_delete"
  "trace_path_mixed"
  "trace_batch_only_edges"
  "trace_star_mixed"
  "trace_cycle4_mixed"
  "trace_path_chord"
  "trace_ks4_one_del"
)

for trace in "${traces[@]}"; do
  graph="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.graph"
  batch="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.batch"

  "$ROOT_DIR/scripts/run_seq_trace.sh" "$graph" "$batch" "$OUT_DIR/${trace}.seq.json"

  if [[ "$REF_ENABLED" -eq 0 ]]; then
    echo "[skip] reference trace: $trace (REF_BIN not set)"
    continue
  fi

  if REF_BIN="$REF_BIN" "$ROOT_DIR/scripts/run_reference_trace.sh" "$graph" "$batch" "$OUT_DIR/${trace}.ref.json"; then
    echo "[ok] reference trace: $trace"
  else
    echo "[warn] reference trace failed for $trace (see ${trace}.ref.json raw_output)"
    if [[ "$REF_REQUIRED" == "1" ]]; then
      echo "[error] REF_REQUIRED=1 and a reference trace failed; aborting suite."
      exit 1
    fi
  fi
done

echo "Wrote trace outputs to: $OUT_DIR"
