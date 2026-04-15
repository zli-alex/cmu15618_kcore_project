#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
PAR_BIN="${PAR_BIN:-$ROOT_DIR/par_trace_runner}"

if [[ $# -lt 3 ]]; then
  echo "Usage: scripts/run_par_trace.sh <graph.txt> <batch.txt> <out.json> [extra par_trace_runner args]"
  exit 1
fi

GRAPH_PATH="$1"
BATCH_PATH="$2"
OUT_PATH="$3"
shift 3

if [[ ! -x "$PAR_BIN" ]]; then
  echo "Building par_trace_runner..."
  make -C "$ROOT_DIR" par_trace_runner >/dev/null
fi

mkdir -p "$(dirname "$OUT_PATH")"
"$PAR_BIN" --graph "$GRAPH_PATH" --batch "$BATCH_PATH" --out "$OUT_PATH" --num-threads 1 "$@"
