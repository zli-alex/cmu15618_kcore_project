# A Systems Study on Parallel Batch-Dynamic k-Core Maintenance of Dynamic Graph with Asynchronous Reads
**CMU 15-618 S26 Project Web page**

**Instructors:** Prof. Todd C. Mowry, and Prof. Brian Railing

**Students:** Zhengfei Li (zhengfel) and Max Wang (junkaiw)

## Summary

The project will implement and analyze a parallel system for maintaining an approximate k-core decomposition of a dynamic graph under batched edge updates.
While based on a recent publication on asynchronous k-core algorithms ([Liu, Shun, & Zablotchi, 2024](https://arxiv.org/abs/2401.08015)),
the project focuses on optimizing parallelized system performance for the level data structure (LSD) and update dependency tracking mechanism.
We will study tradeoffs in throughput, scalability, and read latency between different design choices.

## Schedule and Progress

### Week 1: Replication and Sequential Baseline
- [x] Set up project structure and validation workflow
- [x] Implement sequential baseline algorithm
- [x] Build fixed-trace and randomized correctness tests
- [ ] Replicate ParlayLib/GBBS reference baseline  (local build blocked on arm64 macOS `-mcx16`, moving to Linux)

### Week 2: Parallelization
- [x] Implement batch update logic
- [x] Build parallel PLDS-style update path
- [x] Add descriptor logic for CPLDS update path
- [ ] Implement basic worklist
- [ ] Parallel scheduling (current version uses conservative parallel scan + serial apply; more advanced scheduling deferred)

### Week 3: Milestone + Initial Validation
- [x] Produce milestone report draft
- [ ] Run and analyze initial experiments (correctness/parity experiments complete; broader performance experiments not started yet)
- [ ] Refine CPLDS read semantics

### Week 4: Analysis and Improvement
- [ ] Unblock external reference run on Linux/x86_64
- [ ] Run reference test
- [ ] Run medium-scale experiments (sequential vs parallel)
- [ ] Add profiling and identify bottlenecks
- [ ] Improve main bottleneck (likely scan/apply overhead, serialization, or memory behavior)
- [ ] Optionally test alternative scheduling/worklist variants

### Week 5: Wrap-up and Final Report
- [ ] Run final experiment suite
- [ ] Summarize system findings and best-performing configuration
- [ ] Compare our implementation against reference where possible
- [ ] Finalize poster, report, and presentation artifacts

## Proposals and reports

- [15618 Project Proposal](https://docs.google.com/document/d/1DOFGf5Bqde4KWEMQ51hkiGXs0TmVSFQqT2caB3JY6sw/edit?tab=t.0)
- [15618 Project Milestone](https://docs.google.com/document/d/1c5QkXZkq5Si0WwRXrSnD_5bmo698YJ_y0YF0EsTFs_E/edit?usp=sharing)

## References

Liu, Q. C., Shun, J., & Zablotchi, I. (2024). *Parallel k-core decomposition with batched updates and asynchronous reads*. arXiv. https://arxiv.org/abs/2401.08015