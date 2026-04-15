#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${1:-$(mktemp -d "${TMPDIR:-/tmp}/par_oracle_cmp.XXXXXX")}"
mkdir -p "$OUT_DIR"

make -C "$ROOT_DIR" seq_trace_runner par_trace_runner >/dev/null

traces=(
  "trace_triangle_insert"
  "trace_triangle_delete"
  "trace_path_mixed"
)

normalize_json_file() {
  python3 - "$1" <<'PY'
import json
import sys

path = sys.argv[1]
with open(path, encoding="utf-8") as f:
    data = json.load(f)
data.pop("implementation", None)
if "summary" in data and isinstance(data["summary"], dict):
    data["summary"].pop("elapsed_ms", None)
if "config" in data and isinstance(data["config"], dict):
    data["config"].pop("num_threads", None)
json.dump(data, sys.stdout, indent=2, sort_keys=True)
PY
}

for trace in "${traces[@]}"; do
  graph="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.graph"
  batch="$ROOT_DIR/datasets/tiny/fixed_traces/${trace}.batch"
  seq_out="$OUT_DIR/${trace}.seq.json"
  par_out="$OUT_DIR/${trace}.par.json"
  "$ROOT_DIR/scripts/run_seq_trace.sh" "$graph" "$batch" "$seq_out"
  "$ROOT_DIR/scripts/run_par_trace.sh" "$graph" "$batch" "$par_out"
  if ! diff -q <(normalize_json_file "$seq_out") <(normalize_json_file "$par_out") >/dev/null; then
    echo "[fail] normalized output mismatch: $trace"
    diff -u <(normalize_json_file "$seq_out") <(normalize_json_file "$par_out") || true
    exit 1
  fi
  echo "[ok] oracle parity: $trace"
done

echo "All fixed tiny traces match sequential oracle (normalized). Outputs: $OUT_DIR"
