#include "sequential/read_interface.h"

#include "sequential/coreness.h"

namespace seq {

VertexId find_root(const DescriptorState& descriptors, VertexId v) {
  if (descriptors.parent.empty() || v >= descriptors.parent.size()) {
    return v;
  }

  VertexId current = v;
  // Traversal-only Week 1 scaffold; Week 2+ can add path compression.
  for (std::size_t steps = 0; steps < descriptors.parent.size(); ++steps) {
    const VertexId parent = descriptors.parent[current];
    if (parent == current || parent >= descriptors.parent.size()) {
      return current;
    }
    current = parent;
  }
  return current;
}

DagMarkStatus check_dag(const DescriptorState& descriptors, VertexId v) {
  (void)find_root(descriptors, v);
  // Week 1 behavior: all reads are on finalized batch state.
  // Therefore, descriptors are treated as logically UNMARKED.
  return DagMarkStatus::Unmarked;
}

double read_estimate(const LDSConfig& config, const LDSState& state,
                     const DescriptorState& descriptors, VertexId v,
                     ReadDebugInfo* debug) {
  // This retry skeleton mirrors Algorithm 4 shape for Week 2+ upgrades.
  for (int attempt = 0; attempt < 4; ++attempt) {
    const std::size_t b1 = descriptors.batch_number;
    const int l1 = state.level(v);
    const int old_level = (v < descriptors.old_level.size()) ? descriptors.old_level[v] : l1;
    const VertexId root = find_root(descriptors, v);
    const DagMarkStatus status = check_dag(descriptors, root);
    const int l2 = state.level(v);
    const std::size_t b2 = descriptors.batch_number;

    if (debug != nullptr) {
      debug->batch_before = b1;
      debug->batch_after = b2;
      debug->live_level_first = l1;
      debug->live_level_second = l2;
      debug->descriptor_old_level = old_level;
      debug->root = root;
      debug->dag_status = status;
    }

    if (b1 != b2) {
      continue;
    }
    if (status == DagMarkStatus::Marked) {
      return estimate_coreness_from_level(config, old_level, state.num_vertices());
    }
    if (l1 == l2) {
      return estimate_coreness_from_level(config, l1, state.num_vertices());
    }
  }

  // Finalized-state fallback for Week 1.
  return estimate_coreness_from_level(config, state.level(v), state.num_vertices());
}

double read_estimate(const SequentialBatchEngine& engine, VertexId v,
                     ReadDebugInfo* debug) {
  return read_estimate(engine.config, engine.state, engine.descriptors, v, debug);
}

}  // namespace seq
