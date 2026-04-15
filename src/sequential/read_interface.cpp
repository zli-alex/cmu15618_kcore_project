#include "sequential/read_interface.h"

#include "sequential/coreness.h"

namespace seq {

VertexId find_root(const DescriptorState& descriptors, VertexId v) {
  if (descriptors.parent.empty() || v >= descriptors.parent.size()) {
    return v;
  }

  VertexId current = v;
  // Traversal-only scaffold; path compression can be added with full CPLDS DAG work.
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
  const VertexId root = find_root(descriptors, v);
  if (root < descriptors.marked.size() && descriptors.marked[root] != 0) {
    return DagMarkStatus::Marked;
  }
  return DagMarkStatus::Unmarked;
}

double read_estimate(const LDSConfig& config, const LDSState& state,
                     const DescriptorState& descriptors, VertexId v,
                     ReadDebugInfo* debug) {
  const std::size_t b1 = descriptors.batch_number;
  const int l1 = state.level(v);
  const int old_level_v =
      (v < descriptors.old_level.size()) ? descriptors.old_level[v] : l1;
  const VertexId root = find_root(descriptors, v);
  const DagMarkStatus status = check_dag(descriptors, v);
  const int l2 = state.level(v);
  const std::size_t b2 = descriptors.batch_number;

  if (debug != nullptr) {
    debug->batch_before = b1;
    debug->batch_after = b2;
    debug->live_level_first = l1;
    debug->live_level_second = l2;
    debug->descriptor_old_level = old_level_v;
    debug->root = root;
    debug->dag_status = status;
  }

  if (status == DagMarkStatus::Marked) {
    return estimate_coreness_from_level(config, old_level_v, state.num_vertices());
  }
  return estimate_coreness_from_level(config, state.level(v), state.num_vertices());
}

double read_estimate(const SequentialBatchEngine& engine, VertexId v,
                     ReadDebugInfo* debug) {
  return read_estimate(engine.config, engine.state, engine.descriptors, v, debug);
}

}  // namespace seq
