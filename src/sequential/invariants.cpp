#include "sequential/invariants.h"

#include <cmath>

namespace seq {
namespace {

int induced_degree_in_z(const Graph& graph, const LDSState& state, VertexId v,
                        int z_level) {
  int support = 0;
  for (VertexId u : graph.neighbors(v)) {
    if (state.level(u) >= z_level) {
      ++support;
    }
  }
  return support;
}

double invariant1_upper_bound(const LDSConfig& config, int group_id) {
  const double safe_delta = config.delta > 0.0 ? config.delta : 0.2;
  const double safe_lambda = config.lambda > 0.0 ? config.lambda : 1.0;
  return (2.0 + 3.0 / safe_lambda) * std::pow(1.0 + safe_delta, group_id);
}

double invariant2_lower_bound(const LDSConfig& config, int group_id) {
  const double safe_delta = config.delta > 0.0 ? config.delta : 0.2;
  return std::pow(1.0 + safe_delta, group_id);
}

}  // namespace

bool check_invariant1_vertex(const Graph& graph, const LDSConfig& config,
                             const LDSState& state, VertexId v) {
  // Invariant 1 (paper): if v in V_l and l in g_i,
  // deg_{Z_l}(v) <= (2 + 3/lambda) * (1 + delta)^i.
  const int l = state.level(v);
  const int group_i = config.level_to_group(l);
  const int deg_zl = induced_degree_in_z(graph, state, v, l);
  return static_cast<double>(deg_zl) <= invariant1_upper_bound(config, group_i);
}

bool check_invariant2_vertex(const Graph& graph, const LDSConfig& config,
                             const LDSState& state, VertexId v) {
  // Invariant 2 (paper): if v in V_l, l > 0, and l-1 in g_i,
  // deg_{Z_{l-1}}(v) >= (1 + delta)^i.
  const int l = state.level(v);
  if (l <= 0) {
    return true;
  }
  const int group_i = config.level_to_group(l - 1);
  const int deg_zl_minus_1 = induced_degree_in_z(graph, state, v, l - 1);
  return static_cast<double>(deg_zl_minus_1) >=
         invariant2_lower_bound(config, group_i);
}

bool all_vertices_satisfy_invariants(const Graph& graph, const LDSConfig& config,
                                     const LDSState& state) {
  for (VertexId v = 0; v < state.num_vertices(); ++v) {
    if (!check_invariant1_vertex(graph, config, state, v) ||
        !check_invariant2_vertex(graph, config, state, v)) {
      return false;
    }
  }
  return true;
}

std::vector<VertexId> collect_invariant_violations(const Graph& graph,
                                                   const LDSConfig& config,
                                                   const LDSState& state,
                                                   const std::vector<VertexId>& candidates) {
  std::vector<VertexId> out;
  out.reserve(candidates.size());
  for (VertexId v : candidates) {
    if (!check_invariant1_vertex(graph, config, state, v) ||
        !check_invariant2_vertex(graph, config, state, v)) {
      out.push_back(v);
    }
  }
  return out;
}

bool promote_vertex_one_level(const Graph&, const LDSConfig& config, LDSState& state,
                              VertexId v) {
  const int level = state.level(v);
  if (level >= config.num_levels - 1) {
    return false;
  }
  state.set_level(v, level + 1);
  return true;
}

bool demote_vertex_one_level(const Graph&, const LDSConfig&, LDSState& state, VertexId v) {
  const int level = state.level(v);
  if (level <= 0) {
    return false;
  }
  state.set_level(v, level - 1);
  return true;
}

}  // namespace seq
