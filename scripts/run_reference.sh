#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DEFAULT_BIN="$ROOT_DIR/third_party/reference_impl/reference_runner"
REF_BIN="${REF_BIN:-$DEFAULT_BIN}"

if [[ ! -x "$REF_BIN" ]]; then
  echo "Reference binary not found or not executable: $REF_BIN"
  echo
  echo "Setup instructions:"
  echo "1) Place the external reference repo at:"
  echo "   third_party/reference_impl/"
  echo "2) Build it manually following its own README."
  echo "3) Re-run with:"
  echo "   REF_BIN=/absolute/path/to/reference_binary scripts/run_reference.sh [args]"
  echo
  echo "See docs/reference_setup.md for full details."
  exit 1
fi

exec "$REF_BIN" "$@"
