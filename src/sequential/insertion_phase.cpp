#include "sequential/insertion_phase.h"

#include <algorithm>
#include <vector>

#include "sequential/invariants.h"

namespace seq {

void apply_insertions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                      const LDSConfig& config,
                      const std::vector<EdgeUpdate>& insertions,
                      std::size_t* num_promotions,
                      std::size_t* num_invariant_fixups) {
  for (const auto& upd : insertions) {
    (void)graph.add_edge(upd.u, upd.v);
  }

  // PLDS insertion discipline: visit levels in increasing order exactly once.
  for (int l = 0; l < config.num_levels; ++l) {
    std::vector<VertexId> candidates = state.vertices_in_level(l);
    std::sort(candidates.begin(), candidates.end());
    for (VertexId v : candidates) {
      if (state.level(v) != l) {
        continue;
      }
      if (!check_invariant1_vertex(graph, config, state, v)) {
        const int level_before = state.level(v);
        if (promote_vertex_one_level(graph, config, state, v)) {
          ++(*num_promotions);
          ++(*num_invariant_fixups);
          descriptors.on_level_mutation_start(v, level_before);
        }
      }
    }
  }
}

}  // namespace seq
