#include "sequential/deletion_phase.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "sequential/invariants.h"

namespace seq {
namespace {

int compute_desire_level_vertex(const Graph& graph, const LDSConfig& config,
                                const LDSState& state, VertexId v,
                                int min_allowed_level) {
  const int current = state.level(v);
  if (current <= min_allowed_level) {
    return current;
  }

  for (int target = current - 1; target >= min_allowed_level; --target) {
    if (target <= 0) {
      return 0;
    }

    int deg_z_target_minus_1 = 0;
    for (VertexId u : graph.neighbors(v)) {
      if (state.level(u) >= target - 1) {
        ++deg_z_target_minus_1;
      }
    }

    const int group_i = config.level_to_group(target - 1);
    const double threshold =
        std::pow(1.0 + std::max(0.0, config.delta), group_i);
    if (static_cast<double>(deg_z_target_minus_1) >= threshold) {
      return target;
    }
  }

  return min_allowed_level;
}

}  // namespace

std::vector<int> compute_desire_levels(const Graph& graph, const LDSConfig& config,
                                       const LDSState& state,
                                       const std::vector<VertexId>& affected) {
  std::vector<int> desire(affected.size(), 0);
  for (std::size_t i = 0; i < affected.size(); ++i) {
    const VertexId v = affected[i];
    if (check_invariant2_vertex(graph, config, state, v)) {
      desire[i] = state.level(v);
    } else {
      desire[i] = compute_desire_level_vertex(graph, config, state, v, 0);
    }
  }
  return desire;
}

void apply_deletions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                     const LDSConfig& config,
                     const std::vector<EdgeUpdate>& deletions,
                     std::size_t* num_demotions,
                     std::size_t* num_invariant_fixups) {
  std::vector<VertexId> touched;
  for (const auto& upd : deletions) {
    if (graph.remove_edge(upd.u, upd.v)) {
      touched.push_back(upd.u);
      touched.push_back(upd.v);
    }
  }

  std::sort(touched.begin(), touched.end());
  touched.erase(std::unique(touched.begin(), touched.end()), touched.end());

  std::vector<int> desire(state.num_vertices(), -1);
  std::vector<bool> moved(state.num_vertices(), false);

  auto recompute_desire = [&](VertexId v, int min_allowed_level) {
    if (moved[v] || state.level(v) <= min_allowed_level) {
      desire[v] = -1;
      return;
    }
    if (check_invariant2_vertex(graph, config, state, v)) {
      desire[v] = -1;
      return;
    }
    desire[v] = compute_desire_level_vertex(graph, config, state, v,
                                            min_allowed_level);
  };

  for (VertexId v : touched) {
    recompute_desire(v, 0);
  }

  // PLDS deletion discipline: process levels in increasing order and move
  // each vertex to its desire level at most once in this batch.
  for (int l = 0; l < config.num_levels; ++l) {
    std::vector<VertexId> movers;
    for (VertexId v = 0; v < state.num_vertices(); ++v) {
      if (!moved[v] && desire[v] == l && state.level(v) > l) {
        movers.push_back(v);
      }
    }

    while (!movers.empty()) {
      std::vector<VertexId> moved_this_round;
      for (VertexId v : movers) {
        if (moved[v] || state.level(v) <= l || desire[v] != l) {
          continue;
        }
        const int old_level = state.level(v);
        descriptors.on_level_mutation_start(v, old_level);
        state.set_level(v, l);
        moved[v] = true;
        desire[v] = -1;
        moved_this_round.push_back(v);
        *num_demotions += static_cast<std::size_t>(old_level - l);
        *num_invariant_fixups += static_cast<std::size_t>(old_level - l);
      }

      movers.clear();
      for (VertexId v : moved_this_round) {
        for (VertexId u : graph.neighbors(v)) {
          if (state.level(u) > l) {
            recompute_desire(u, l + 1);
          }
        }
      }

      for (VertexId v = 0; v < state.num_vertices(); ++v) {
        if (!moved[v] && desire[v] == l && state.level(v) > l) {
          movers.push_back(v);
        }
      }
    }
  }
}

}  // namespace seq
