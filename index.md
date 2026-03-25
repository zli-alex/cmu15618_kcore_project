# A Systems Study on Parallel Batch-Dynamic k-Core Maintenance of Dynamic Graph with Asynchronous Reads
**CMU 15-618 S26 Project Web page**

**Instructors:** Prof. Todd C. Mowry, and Prof. Brian Railing

**Students:** Zhengfei Li (zhengfel) and Max Wang (junkaiw)

## Summary

The project will implement and analyze a parallel system for maintaining an approximate k-core decomposition of a dynamic graph under batched edge updates.
While based on a recent publication on asynchronous k-core algorithms ([Liu, Shun, & Zablotchi, 2024](https://arxiv.org/abs/2401.08015)),
the project focuses on optimizing parallelized system performance for the level data structure (LSD) and update dependency tracking mechanism.
We will study tradeoffs in throughput, scalability, and read latency between different design choices.

## Proposals and reports

- [15618 Project Proposal](https://docs.google.com/document/d/1DOFGf5Bqde4KWEMQ51hkiGXs0TmVSFQqT2caB3JY6sw/edit?tab=t.0)

## References

Liu, Q. C., Shun, J., & Zablotchi, I. (2024). *Parallel k-core decomposition with batched updates and asynchronous reads*. arXiv. https://arxiv.org/abs/2401.08015