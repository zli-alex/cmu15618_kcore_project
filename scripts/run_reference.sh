#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEFAULT_BIN="$ROOT_DIR/third_party/reference_impl/reference_runner"
REF_BIN="${REF_BIN:-$DEFAULT_BIN}"

if [[ ! -x "$REF_BIN" ]]; then
  echo "[error] Reference binary is missing or not executable: $REF_BIN" >&2
  echo "[hint] Set REF_BIN to the external executable path, for example:" >&2
  echo "       REF_BIN=/absolute/path/to/reference_binary scripts/run_reference.sh <graph> <batch> [extra args...]" >&2
  echo "[hint] Default path checked when REF_BIN is unset:" >&2
  echo "       $DEFAULT_BIN" >&2
  echo "[hint] Setup docs: docs/reference_setup.md and docs/reference_wiring.md" >&2
  exit 1
fi

exec "$REF_BIN" "$@"
