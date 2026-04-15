#pragma once

#include <vector>

#include "core/batch_update.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

namespace seq {

struct BatchResult {
  std::size_t num_promotions = 0;
  std::size_t num_demotions = 0;
  std::size_t invariant_fixups = 0;
  std::size_t insertion_upward_moves = 0;
  std::size_t insertion_downward_moves = 0;
  std::size_t deletion_upward_moves = 0;
  std::size_t deletion_downward_moves = 0;
  bool invariants_hold_after_insertions = false;
  bool invariants_hold_after_deletions = false;
  std::vector<double> coreness_estimate;
  std::vector<int> levels_before;
  std::vector<int> levels_after;
  std::vector<VertexId> moved_vertices;
};

struct SequentialBatchEngine {
  SequentialBatchEngine(Graph graph, LDSConfig config, LDSState state,
                        DescriptorState descriptors);

  Graph graph;
  LDSConfig config;
  LDSState state;
  DescriptorState descriptors;
};

BatchResult process_batch(SequentialBatchEngine& engine, const BatchUpdate& batch);

}  // namespace seq
