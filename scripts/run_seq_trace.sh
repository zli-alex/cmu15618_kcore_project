#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SEQ_BIN="${SEQ_BIN:-$ROOT_DIR/seq_trace_runner}"

if [[ $# -lt 3 ]]; then
  echo "Usage: scripts/run_seq_trace.sh <graph.txt> <batch.txt> <out.json> [extra seq_trace_runner args]"
  exit 1
fi

GRAPH_PATH="$1"
BATCH_PATH="$2"
OUT_PATH="$3"
shift 3

if [[ ! -x "$SEQ_BIN" ]]; then
  echo "Building seq_trace_runner..."
  make -C "$ROOT_DIR" seq_trace_runner >/dev/null
fi

mkdir -p "$(dirname "$OUT_PATH")"
"$SEQ_BIN" --graph "$GRAPH_PATH" --batch "$BATCH_PATH" --out "$OUT_PATH" "$@"
