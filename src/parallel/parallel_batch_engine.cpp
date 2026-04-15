#include "parallel/parallel_batch_engine.h"

#include <cassert>
#include <vector>

#include "core/batch_update.h"
#include "parallel/parallel_deletion.h"
#include "parallel/parallel_insertion.h"
#include "sequential/coreness.h"
#include "sequential/invariants.h"

namespace seq {
namespace parallel {
namespace {

std::vector<int> snapshot_levels(const LDSState& state) {
  std::vector<int> levels(state.num_vertices(), 0);
  for (VertexId v = 0; v < state.num_vertices(); ++v) {
    levels[v] = state.level(v);
  }
  return levels;
}

void count_directional_moves(const std::vector<int>& before, const std::vector<int>& after,
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

ParallelBatchEngine::ParallelBatchEngine(SequentialBatchEngine sequential_engine,
                                         ParallelOptions options)
    : sequential(std::move(sequential_engine)), options(options) {}

BatchResult process_batch(ParallelBatchEngine& engine, const BatchUpdate& batch) {
  SequentialBatchEngine& e = engine.sequential;
  e.descriptors.ensure_vertices(e.state.num_vertices());

  BatchResult result;
  result.levels_before = snapshot_levels(e.state);
  e.descriptors.begin_batch(result.levels_before);
  const PartitionedUpdates parts = partition_insertions_first(batch);

  const auto before_insertions = snapshot_levels(e.state);
  apply_insertions(e.graph, e.state, e.descriptors, e.config, parts.insertions,
                   &result.num_promotions, &result.invariant_fixups, engine.options.num_threads);
  const auto after_insertions = snapshot_levels(e.state);
  count_directional_moves(before_insertions, after_insertions, &result.insertion_upward_moves,
                          &result.insertion_downward_moves);
  result.invariants_hold_after_insertions =
      all_vertices_satisfy_invariants(e.graph, e.config, e.state);

  const auto before_deletions = after_insertions;
  apply_deletions(e.graph, e.state, e.descriptors, e.config, parts.deletions, &result.num_demotions,
                  &result.invariant_fixups, engine.options.num_threads);
  const auto after_deletions = snapshot_levels(e.state);
  count_directional_moves(before_deletions, after_deletions, &result.deletion_upward_moves,
                          &result.deletion_downward_moves);
  result.invariants_hold_after_deletions =
      all_vertices_satisfy_invariants(e.graph, e.config, e.state);

  e.descriptors.unmark_all_roots_first();
  result.levels_after = snapshot_levels(e.state);
  for (VertexId v = 0; v < result.levels_before.size(); ++v) {
    if (result.levels_before[v] != result.levels_after[v]) {
      result.moved_vertices.push_back(v);
#ifndef NDEBUG
      assert(v < e.descriptors.first_mutation_recorded.size());
      assert(e.descriptors.first_mutation_recorded[v] == 1 &&
             "descriptor lifecycle hook missing for moved vertex");
#endif
    }
  }

  result.coreness_estimate = estimate_all_coreness(e.config, e.state);
  return result;
}

}  // namespace parallel
}  // namespace seq
