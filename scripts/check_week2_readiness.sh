#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"

echo "[check] oracle policy doc"
test -f docs/oracle_policy.md

echo "[check] semantic audit doc"
test -f docs/week1_semantic_audit.md

echo "[check] week2 PLDS status doc"
test -f docs/week2_plds_status.md

echo "[check] make test"
make test

echo "[check] tiny trace suite (sequential + optional reference)"
./scripts/run_tiny_trace_suite.sh

echo "[check] sequential vs parallel@1 vs parallel@N on fixed tiny traces (PAR_CMP_THREADS=${PAR_CMP_THREADS:-4})"
PAR_CMP_THREADS="${PAR_CMP_THREADS:-4}" ./scripts/compare_parallel_oracle.sh

if [[ -n "${REF_BIN:-}" ]]; then
  echo "[check] REF_BIN is set; requiring one successful reference smoke trace"
  if [[ ! -x "$REF_BIN" ]]; then
    echo "[fail] REF_BIN is not executable: $REF_BIN" >&2
    exit 1
  fi
  out="$(mktemp "${TMPDIR:-/tmp}/week2_ref_smoke.XXXXXX.json")"
  cleanup() { rm -f "$out"; }
  trap cleanup EXIT
  if REF_BIN="$REF_BIN" "$ROOT_DIR/scripts/check_reference_smoke.sh" "$out"; then
    echo "[ok] reference smoke succeeded with REF_BIN=$REF_BIN"
  else
    echo "[fail] reference smoke failed with REF_BIN=$REF_BIN" >&2
    echo "[fail] failure categories: process exit, missing executable, or incompatible output schema." >&2
    exit 1
  fi
else
  echo "[skip] REF_BIN not set; skipping external reference binary gate"
fi

echo "[ok] week2 readiness checks passed"
