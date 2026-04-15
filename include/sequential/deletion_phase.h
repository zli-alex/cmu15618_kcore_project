#pragma once

#include <vector>

#include "core/batch_update.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

namespace seq {

std::vector<int> compute_desire_levels(const Graph& graph, const LDSConfig& config,
                                       const LDSState& state,
                                       const std::vector<VertexId>& affected);

void apply_deletions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                     const LDSConfig& config,
                     const std::vector<EdgeUpdate>& deletions,
                     std::size_t* num_demotions,
                     std::size_t* num_invariant_fixups);

}  // namespace seq
