#pragma once

#include <vector>

#include "core/graph.h"
#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

namespace seq {

bool check_invariant1_vertex(const Graph& graph, const LDSConfig& config,
                             const LDSState& state, VertexId v);
bool check_invariant2_vertex(const Graph& graph, const LDSConfig& config,
                             const LDSState& state, VertexId v);
bool all_vertices_satisfy_invariants(const Graph& graph, const LDSConfig& config,
                                     const LDSState& state);

std::vector<VertexId> collect_invariant_violations(const Graph& graph,
                                                   const LDSConfig& config,
                                                   const LDSState& state,
                                                   const std::vector<VertexId>& candidates);

bool promote_vertex_one_level(const Graph& graph, const LDSConfig& config,
                              LDSState& state, VertexId v);
bool demote_vertex_one_level(const Graph& graph, const LDSConfig& config,
                             LDSState& state, VertexId v);

}  // namespace seq
