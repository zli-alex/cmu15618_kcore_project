# A Systems Study on Parallel Batch-Dynamic k-Core Maintenance of Dynamic Graph with Asynchronous Reads
**CMU 15-618 S26 Project Web page**

**Instructors:** Prof. Todd C. Mowry, and Prof. Brian Railing

**Students:** Zhengfei Li (zhengfel) and Max Wang (junkaiw)

## Summary

This project implements and analyzes a parallel system for maintaining an approximate k-core decomposition of a dynamic graph under batched edge updates. The project is based on recent work on parallel batch-dynamic k-core maintenance and asynchronous reads ([Liu, Shun, & Zablotchi, 2024](https://arxiv.org/abs/2401.08015)), but uses our own C++/`std::thread` implementation rather than the paper’s ParlayLib/GBBS codebase.

Our implementation includes a sequential LDS oracle, a conservative parallel update path, profiling-guided graph-update optimizations, and a conservative one-root CPLDS-inspired asynchronous read mechanism. We evaluate tradeoffs among safe synchronized reads, unsafe non-synchronized reads, and descriptor-based asynchronous reads, focusing on throughput, scalability, tail latency, and update-side bottlenecks.

## Schedule and Progress

### Week 1: Replication and Sequential Baseline
- [x] Set up project structure and validation workflow
- [x] Implement sequential LDS baseline/oracle
- [x] Build fixed-trace and randomized correctness tests
- [x] Add sequential/parallel oracle comparison workflow
- [ ] Replicate ParlayLib/GBBS reference baseline  
  - Reference replication was partially investigated but not completed; local build was blocked by platform/toolchain issues, so we focused on our own controlled implementation and evaluation.

### Week 2: Conservative Parallel Update Path
- [x] Implement batch update logic for insertion and deletion
- [x] Build conservative PLDS-style parallel update path
- [x] Parallelize read-only scan phases using `std::thread`
- [x] Preserve serial mutation/apply phases for correctness and deterministic parity with the sequential oracle
- [x] Add descriptor lifecycle scaffolding for update tracking
- [ ] Implement full dependency-DAG CPLDS protocol  
  - Deferred; current implementation uses a conservative one-root CPLDS-inspired protocol.

### Week 3: Milestone, Correctness, and Initial Profiling
- [x] Produce milestone report draft
- [x] Validate sequential and parallel correctness on fixed traces and randomized parity tests
- [x] Add fine-grained profiling infrastructure
- [x] Identify initial bottleneck: graph update and adjacency-cache materialization dominated runtime
- [x] Implement batched graph updates to rebuild each touched vertex cache once per batch

### Week 4: Profiling-Guided Optimization and Async Reads
- [x] Add Stage 2 fine-grained graph-update profiling
- [x] Add neighbor-demand profiling to evaluate lazy cache rebuild potential
- [x] Show that lazy/targeted cache rebuild is not helpful on current workloads because nearly all dirty/touched vertices are read
- [x] Add parallel cache rebuild over touched vertices
- [x] Implement conservative descriptor mark-before-move lifecycle
- [x] Add atomic read-facing descriptor and live-level state
- [x] Implement `async_read_estimate(...)` for ConservativeCPLDS
- [x] Add controlled read/update overlap correctness tests

### Week 5: Final Evaluation and Report
- [x] Implement read-overlap benchmark with three modes:
  - `SyncReads`: safe blocking baseline
  - `NonSync`: unsafe/non-linearizable lower-bound baseline
  - `ConservativeCPLDS`: one-root descriptor-based async read mode
- [x] Run repeated medium read-overlap experiments
- [x] Add DBLP insertion-only dataset conversion and smoke validation
- [x] Run final paper-aligned experiments on PSC/Linux
- [x] Add update-worker scaling support and reporting
- [x] Generate final tables, plots, and result summaries
- [x] Summarize key findings:
  - ConservativeCPLDS achieves much lower tail latency than SyncReads while staying close to unsafe NonSync.
  - No NaN/unstable reads were observed in final experiments.
  - Update-worker scaling is limited by graph/cache materialization and serial update-side work.
- [ ] Finalize poster, report, and presentation artifacts

## Proposals and Reports

- [15618 Project Proposal](https://docs.google.com/document/d/1DOFGf5Bqde4KWEMQ51hkiGXs0TmVSFQqT2caB3JY6sw/edit?tab=t.0)
- [15618 Project Milestone](https://docs.google.com/document/d/1c5QkXZkq5Si0WwRXrSnD_5bmo698YJ_y0YF0EsTFs_E/edit?usp=sharing)
- [15618 Final Report](https://docs.google.com/document/d/1wjhSxFLeR3z5XOmeKlEE1agyRTZnzWMTZzBZbusNtbU/edit?usp=sharing)

## References

Liu, Q. C., Shun, J., & Zablotchi, I. (2024). *Parallel k-core decomposition with batched updates and asynchronous reads*. arXiv. https://arxiv.org/abs/2401.08015