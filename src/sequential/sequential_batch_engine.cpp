#include "sequential/sequential_batch_engine.h"

#include <cassert>
#include <vector>

#include "sequential/coreness.h"
#include "sequential/deletion_phase.h"
#include "sequential/insertion_phase.h"
#include "sequential/invariants.h"

namespace seq {
namespace {

std::vector<int> snapshot_levels(const LDSState& state) {
  std::vector<int> levels(state.num_vertices(), 0);
  for (VertexId v = 0; v < state.num_vertices(); ++v) {
    levels[v] = state.level(v);
  }
  return levels;
}

void count_directional_moves(const std::vector<int>& before,
                             const std::vector<int>& after,
                             std::size_t* upward, std::size_t* downward) {
  *upward = 0;
  *downward = 0;
  for (std::size_t i = 0; i < before.size(); ++i) {
    if (after[i] > before[i]) {
      ++(*upward);
    } else if (after[i] < before[i]) {
      ++(*downward);
    }
  }
}

}  // namespace

SequentialBatchEngine::SequentialBatchEngine(Graph graph_, LDSConfig config_, LDSState state_,
                                             DescriptorState descriptors_)
    : graph(std::move(graph_)),
      config(std::move(config_)),
      state(std::move(state_)),
      descriptors(std::move(descriptors_)) {}

BatchResult process_batch(SequentialBatchEngine& engine, const BatchUpdate& batch) {
  engine.descriptors.ensure_vertices(engine.state.num_vertices());

  BatchResult result;
  result.levels_before = snapshot_levels(engine.state);
  engine.descriptors.begin_batch(result.levels_before);
  const PartitionedUpdates parts = partition_insertions_first(batch);

  const auto before_insertions = snapshot_levels(engine.state);
  apply_insertions(engine.graph, engine.state, engine.descriptors, engine.config,
                   parts.insertions, &result.num_promotions,
                   &result.invariant_fixups);
  const auto after_insertions = snapshot_levels(engine.state);
  count_directional_moves(before_insertions, after_insertions,
                          &result.insertion_upward_moves,
                          &result.insertion_downward_moves);
  result.invariants_hold_after_insertions =
      all_vertices_satisfy_invariants(engine.graph, engine.config, engine.state);

  const auto before_deletions = after_insertions;
  apply_deletions(engine.graph, engine.state, engine.descriptors, engine.config,
                  parts.deletions, &result.num_demotions,
                  &result.invariant_fixups);
  const auto after_deletions = snapshot_levels(engine.state);
  count_directional_moves(before_deletions, after_deletions,
                          &result.deletion_upward_moves,
                          &result.deletion_downward_moves);
  result.invariants_hold_after_deletions =
      all_vertices_satisfy_invariants(engine.graph, engine.config, engine.state);

  engine.descriptors.unmark_all_roots_first();
  result.levels_after = snapshot_levels(engine.state);
  for (VertexId v = 0; v < result.levels_before.size(); ++v) {
    if (result.levels_before[v] != result.levels_after[v]) {
      result.moved_vertices.push_back(v);
#ifndef NDEBUG
      assert(v < engine.descriptors.first_mutation_recorded.size());
      assert(engine.descriptors.first_mutation_recorded[v] == 1 &&
             "descriptor lifecycle hook missing for moved vertex");
#endif
    }
  }

  result.coreness_estimate = estimate_all_coreness(engine.config, engine.state);
  return result;
}

}  // namespace seq
