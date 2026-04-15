#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ -z "${REF_BIN:-}" ]]; then
  echo "[fail] REF_BIN is not set."
  echo "[hint] Example: REF_BIN=/absolute/path/to/reference_binary ./scripts/check_reference_smoke.sh"
  exit 1
fi

if [[ ! -x "$REF_BIN" ]]; then
  echo "[fail] REF_BIN is not executable: $REF_BIN"
  exit 1
fi

graph="$ROOT_DIR/datasets/tiny/fixed_traces/trace_triangle_insert.graph"
batch="$ROOT_DIR/datasets/tiny/fixed_traces/trace_triangle_insert.batch"
out="${1:-$ROOT_DIR/results/tiny_trace_compare/reference_smoke.ref.json}"
mkdir -p "$(dirname "$out")"

if REF_BIN="$REF_BIN" "$ROOT_DIR/scripts/run_reference_trace.sh" "$graph" "$batch" "$out"; then
  python3 - <<'PY' "$out"
import json
import pathlib
import sys
doc = json.loads(pathlib.Path(sys.argv[1]).read_text())
summary = doc.get("summary", {})
print(
    f"[pass] reference smoke succeeded: parse_status={summary.get('parse_status')} "
    f"exit_status={summary.get('exit_status')} elapsed_ms={summary.get('elapsed_ms'):.3f}"
)
print(f"[pass] normalized output: {sys.argv[1]}")
PY
else
  echo "[fail] reference smoke failed."
  echo "[fail] inspect raw_output in: $out"
  exit 1
fi
