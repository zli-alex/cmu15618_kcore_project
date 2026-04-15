#pragma once

#include <vector>

#include "core/batch_update.h"
#include "sequential/descriptor_state.h"
#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

namespace seq {

void apply_insertions(Graph& graph, LDSState& state, DescriptorState& descriptors,
                      const LDSConfig& config,
                      const std::vector<EdgeUpdate>& insertions,
                      std::size_t* num_promotions,
                      std::size_t* num_invariant_fixups);

}  // namespace seq
