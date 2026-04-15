#pragma once

#include <cstddef>
#include <vector>

#include "sequential/lds_config.h"
#include "sequential/lds_state.h"

namespace seq {

double estimate_coreness_from_level(const LDSConfig& config, int level,
                                    std::size_t num_vertices);
std::vector<double> estimate_all_coreness(const LDSConfig& config,
                                          const LDSState& state);

}  // namespace seq
