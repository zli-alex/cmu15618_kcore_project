# Reference Baseline Setup

This project expects an external reference implementation to be placed locally
for manual comparison runs. Week 1 keeps this integration manual on purpose.

## 1) Place the reference repository

Place the reference code under:

- `third_party/reference_impl/`

Example:

```bash
cd /path/to/cmu15618_kcore_project
# copy or clone manually outside this project workflow
# so that the repo contents end up at:
# third_party/reference_impl/
```

Do not modify this project to wrap or patch the external source yet.

## 2) Record the reference commit hash

After placing the repository, record the exact commit used for reproducibility.

From the external repo root (`third_party/reference_impl/`):

```bash
git rev-parse HEAD
```

Copy the output hash and store it in your experiment notes (for example in
`docs/` notes or your run log). If desired, also save:

```bash
git status --short
```

to confirm whether the reference checkout was clean.

### Current local checkout note (2026-04-14)

- Reference repo path used: `third_party/batch-dynamic-kcore-decomposition/`
- Commit hash: `b0b7a1753679c05e33982815adb621028cd9d68e`
- `git status --short` output: empty (clean working tree)

### Known blocker (deferred)

On current machines, the upstream setup/build flow is Linux-oriented and may
require `sudo` + `apt` or Linux-specific toolchain/runtime assumptions.
Reference execution is deferred for Week 1 unless we later choose to patch
submodule build scripts or run in a compatible Linux environment.

## 3) Build the reference manually

Build commands depend on that repository's own build system. Run the commands
documented by the reference repository from:

- `third_party/reference_impl/`

Example placeholders:

```bash
cd third_party/reference_impl
# e.g., make
# e.g., cmake -S . -B build && cmake --build build -j
```

## 4) Configure `REF_BIN`

All helper scripts use:

- `REF_BIN` (optional): absolute path to the external reference executable.
- default if unset: `third_party/reference_impl/reference_runner`

Example:

```bash
export REF_BIN=/absolute/path/to/reference_binary
```

If `REF_BIN` is unset, reference runs are skipped in tiny-trace helper scripts.
If `REF_BIN` is set, scripts expect the binary to be executable and fail clearly
when invocation or parsing fails.

## 5) Run the reference manually

Run the reference binary according to its own CLI format.

Example placeholders:

```bash
cd third_party/reference_impl
# e.g., ./reference_binary <graph_input> <update_trace>
```

For this project, a helper script exists at `scripts/run_reference.sh` that
checks for a configured binary path and prints instructions if missing.

For a first wiring check, use:

```bash
REF_BIN=/absolute/path/to/reference_binary ./scripts/check_reference_smoke.sh
```

See `docs/reference_wiring.md` for the tiny-trace workflow and output details.
