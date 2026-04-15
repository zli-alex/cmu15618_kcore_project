#include "sequential/coreness.h"

#include <cmath>

namespace seq {

double estimate_coreness_from_level(const LDSConfig& config, int level,
                                    std::size_t num_vertices) {
  level = config.clamp_level(level);
  const double safe_delta = config.delta > 0.0 ? config.delta : 0.2;
  const double base = 1.0 + safe_delta;
  const std::size_t safe_n = num_vertices > 1 ? num_vertices : 2;
  const int levels_per_group = std::max(
      1, static_cast<int>(4.0 * std::ceil(std::log(static_cast<double>(safe_n)) /
                                          std::log(base))));
  const int exponent = std::max(((level + 1) / levels_per_group) - 1, 0);
  return std::pow(base, exponent);
}

std::vector<double> estimate_all_coreness(const LDSConfig& config,
                                          const LDSState& state) {
  std::vector<double> out(state.num_vertices(), 0.0);
  for (VertexId v = 0; v < state.num_vertices(); ++v) {
    out[v] = estimate_coreness_from_level(config, state.level(v),
                                          state.num_vertices());
  }
  return out;
}

}  // namespace seq
