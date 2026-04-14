# Dynamic Approximate k-Core Maintenance

Minimal Week 1 scaffold for a systems project on dynamic approximate k-core
maintenance under batched graph updates.

## Build

```bash
make
```

This builds the sequential placeholder executable:

- `./seq_runner`

## Run

```bash
./seq_runner
```

## Test

```bash
make test
```

## Reference baseline

This project supports using an external reference implementation as a manual
baseline.

- Place the external repository at `third_party/reference_impl/`
- Follow setup/build/run instructions in `docs/reference_setup.md`
- Use `scripts/run_reference.sh` as a helper launcher once you know the binary
  path (set `REF_BIN` if needed)

## Project Layout

- `include/` headers
- `src/common/` shared utilities
- `src/sequential/` sequential baseline implementation
- `src/reference/` external baseline integration
- `tests/` simple tests and checks
- `scripts/` helper scripts
- `datasets/tiny/` tiny graph inputs and traces
- `results/` output logs and timing artifacts
- `docs/` design notes and reports
- `third_party/` external code dependencies
