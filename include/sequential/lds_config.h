#pragma once

#include <algorithm>
#include <utility>

namespace seq {

struct LDSConfig {
  double delta = 0.2;
  double lambda = 9.0;
  int num_levels = 16;
  int group_size = 4;

  int clamp_level(int level) const {
    return std::max(0, std::min(level, num_levels - 1));
  }

  int level_to_group(int level) const {
    return clamp_level(level) / std::max(1, group_size);
  }

  std::pair<int, int> group_bounds(int group_id) const {
    const int safe_group_size = std::max(1, group_size);
    const int start = std::max(0, group_id) * safe_group_size;
    const int end = std::min(num_levels - 1, start + safe_group_size - 1);
    return {start, end};
  }

  int num_groups() const {
    const int safe_group_size = std::max(1, group_size);
    return (num_levels + safe_group_size - 1) / safe_group_size;
  }
};

}  // namespace seq
