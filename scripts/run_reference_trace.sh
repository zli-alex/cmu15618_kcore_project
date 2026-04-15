#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [[ $# -lt 3 ]]; then
  echo "Usage: scripts/run_reference_trace.sh <graph.txt> <batch.txt> <out.json> [reference args...]"
  exit 1
fi

GRAPH_PATH="$1"
BATCH_PATH="$2"
OUT_PATH="$3"
shift 3

mkdir -p "$(dirname "$OUT_PATH")"
TMP_RAW="$(mktemp)"

start_ns=$(python3 - <<'PY'
import time
print(time.time_ns())
PY
)

set +e
"$ROOT_DIR/scripts/run_reference.sh" "$GRAPH_PATH" "$BATCH_PATH" "$@" >"$TMP_RAW" 2>&1
status=$?
set -e

end_ns=$(python3 - <<'PY'
import time
print(time.time_ns())
PY
)

python3 - <<'PY' "$GRAPH_PATH" "$BATCH_PATH" "$OUT_PATH" "$TMP_RAW" "$status" "$start_ns" "$end_ns"
import json
import pathlib
import re
import sys

graph, batch, out_path, raw_path, status, start_ns, end_ns = sys.argv[1:]
raw_text = pathlib.Path(raw_path).read_text()
elapsed_ms = (int(end_ns) - int(start_ns)) / 1_000_000.0

num_pattern = re.compile(r"^-?\d+(?:\.\d+)?(?:[eE][+-]?\d+)?$")

def parse_value(raw):
    token = raw.strip()
    if num_pattern.match(token):
        if "." in token or "e" in token.lower():
            return float(token)
        return int(token)
    return token

batch_keys = {
    "Batch Running Time": "batch_running_time",
    "Insertion Running Time": "insertion_running_time",
    "Deletion Running Time": "deletion_running_time",
    "Coreness Estimate": "coreness_estimate",
    "Number Insertion Flips": "num_insertion_flips",
    "Number Deletion Flips": "num_deletion_flips",
    "Max Outdegree": "max_outdegree",
}

total_keys = {
    "Total Time": "total_time",
    "Running Time": "running_time",
    "Throughput": "throughput",
    "Num Reader Threads": "num_reader_threads",
    "Latency Percentile 0.9999": "latency_p9999",
    "Latency Percentile 0.99": "latency_p99",
    "Average Latency": "latency_avg",
}

batches = []
totals = {}
current_batch = {}

for line in raw_text.splitlines():
    if not line.startswith("### "):
        continue
    payload = line[4:]
    if ":" not in payload:
        continue
    key, raw_val = payload.split(":", 1)
    key = key.strip()
    value = parse_value(raw_val)

    if key in batch_keys:
        current_batch[batch_keys[key]] = value
        continue
    if key == "Batch Num":
        if "batch_num" in current_batch:
            batches.append(current_batch)
            current_batch = {}
        current_batch["batch_num"] = value
        continue
    if key in total_keys:
        totals[total_keys[key]] = value

if current_batch:
    batches.append(current_batch)

final_coreness = None
for b in reversed(batches):
    if "coreness_estimate" in b:
        final_coreness = b["coreness_estimate"]
        break

normalized = {
    "implementation": "reference_parlaylib",
    "trace": {"graph": graph, "batch": batch},
    "summary": {
        "exit_status": int(status),
        "elapsed_ms": elapsed_ms,
    },
    "batches": batches,
    "totals": totals,
    "levels": {"before": None, "after": None},
    "coreness_estimate": final_coreness,
    "moved_vertices": None,
    "raw_output": raw_text,
}

pathlib.Path(out_path).write_text(json.dumps(normalized, indent=2) + "\n")
print(f"Wrote normalized reference output: {out_path}")
PY

rm -f "$TMP_RAW"

if [[ "$status" -ne 0 ]]; then
  exit "$status"
fi
