# Revised Schedule (Half-week increments)

Last updated: 2026-04-14  
Owners: **Zhengfei**, **Max**

## Assumptions

- This schedule uses **half-week increments** (roughly 3–4 days).
- **Final submission date is not encoded in the repo**, so below we assume a target of **Mon 2026-05-04** for “final package ready”. Adjust the last 1–2 increments as needed once the official deadline is confirmed.
- External reference (Parlay/GBBS) replication is assumed to require a **Linux x86_64 environment**, due to the current arm64 macOS `-mcx16` build blocker.

## Guiding priorities (based on current state)

1. **Keep correctness locked**: sequential oracle + parallel parity must remain green.
2. **Unblock external reference on Linux early**: so we can truthfully claim replication progress.
3. **Complete CPLDS read semantics** enough to talk about “asynchronous reads” beyond scaffolding.
4. **Start performance experiments** only after (2) and (3) have a minimal, stable pipeline.

## Schedule

### Tue 2026-04-14 → Thu 2026-04-16
- **Zhengfei**: Package milestone artifacts for report:
  - keep `results/milestone/` logs up to date
  - finalize `docs/milestone_progress_draft.md` + `docs/revised_schedule.md`
- **Max**: External reference triage plan:
  - identify minimal Linux build instructions for GBBS/Parlay reference
  - document required toolchain + flags; enumerate `-mcx16` failure mode and known workarounds (if any)

### Fri 2026-04-17 → Sun 2026-04-19
- **Zhengfei**: CPLDS read semantics design pass:
  - map exact remaining Algorithm 3/4 requirements to code touchpoints (descriptor DAG status, marked root logic, retry conditions)
  - identify the minimal correctness target we can claim for “async reads”
- **Max**: Secure Linux execution path (blocker removal):
  - obtain access to a Linux machine/cluster account (note: may not have sudo)
  - attempt reference build; if build requires privileged deps, explore container/conda/user-local builds

### Mon 2026-04-20 → Wed 2026-04-22
- **Zhengfei**: Implement remaining read-path semantics (phase 1):
  - implement/check DAG-root status logic needed for the old-vs-live decision
  - add focused unit tests that enforce the intended read behavior (no performance work yet)
- **Max**: Reference smoke run on Linux:
  - produce one successful run on `trace_triangle_insert` and save output artifact
  - wire the artifact back into the repo workflow (document `REF_BIN` path + expected output schema)

### Thu 2026-04-23 → Sat 2026-04-25
- **Zhengfei**: Implement remaining read-path semantics (phase 2):
  - finalize retry/“batch-number sandwich” behavior under simulated concurrent mutations (as far as our current model supports)
  - ensure no regressions to update semantics/parity
- **Max**: Small-to-medium dataset replication plan:
  - choose 1–2 manageable datasets (graph + batch generation approach) that can run on Linux quickly
  - document commands and expected metrics to extract from reference output

### Sun 2026-04-26 → Tue 2026-04-28
- **Zhengfei**: Performance experiment harness (minimal, honest):
  - add a repeatable benchmark script plan (no claims yet): dataset selection, batch sizes, warmup, repetitions
  - decide what “performance” means for us (throughput, latency per batch, scaling)
- **Max**: Run reference on medium dataset(s):
  - collect reference metrics and store as raw logs + parsed summaries
  - confirm our input formats match reference expectations (graph/batch files)

### Wed 2026-04-29 → Fri 2026-05-01
- **Zhengfei**: First performance runs (our implementation):
  - run sequential vs parallel on the same medium dataset(s)
  - collect thread-scaling data (1,2,4,8,…) and confirm correctness invariants remain green
- **Max**: Bottleneck profiling prep:
  - identify likely hotspots to measure (scan phases vs serial apply; graph adjacency access; sorting/merge overhead)
  - set up profiler workflow on Linux (e.g., `perf`, `time`, optional flamegraphs) compatible with permissions

### Sat 2026-05-02 → Mon 2026-05-04
- **Zhengfei**: Draft final report/poster content skeleton:
  - “what we built” architecture figure + correctness story
  - performance charts + honest discussion of bottlenecks and limits
- **Max**: Reference-vs-ours comparison narrative:
  - summarize where we match (semantics/metrics), where results differ, and why (implementation choices, environment)

## Blockers & explicit contingency

- **If Linux access remains blocked by permissions**:
  - fallback to a user-space toolchain (no sudo), or a prebuilt binary workflow
  - in parallel, keep our correctness + read semantics progress moving so we still have a strong internal story
- **If CPLDS read semantics take longer than expected**:
  - lock a minimal correct subset (clearly stated) and focus remaining time on external replication + performance evaluation

