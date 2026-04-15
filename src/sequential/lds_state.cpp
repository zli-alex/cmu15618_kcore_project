#include "sequential/lds_state.h"

#include <algorithm>

namespace seq {

LDSState::LDSState(std::size_t num_vertices, const LDSConfig& config, int initial_level)
    : config_(config),
      levels_(num_vertices, config.clamp_level(initial_level)),
      level_buckets_(config.num_levels),
      position_in_bucket_(num_vertices, 0),
      group_counts_(config.num_groups(), 0) {
  for (VertexId v = 0; v < num_vertices; ++v) {
    const int level = levels_[v];
    position_in_bucket_[v] = level_buckets_[level].size();
    level_buckets_[level].push_back(v);
    group_counts_[config_.level_to_group(level)]++;
  }
}

int LDSState::level(VertexId v) const { return levels_[v]; }

void LDSState::set_level(VertexId v, int new_level) {
  new_level = config_.clamp_level(new_level);
  const int old_level = levels_[v];
  if (old_level == new_level) {
    return;
  }

  auto& old_bucket = level_buckets_[old_level];
  const std::size_t pos = position_in_bucket_[v];
  const VertexId moved = old_bucket.back();
  old_bucket[pos] = moved;
  position_in_bucket_[moved] = pos;
  old_bucket.pop_back();

  auto& new_bucket = level_buckets_[new_level];
  position_in_bucket_[v] = new_bucket.size();
  new_bucket.push_back(v);

  levels_[v] = new_level;
  group_counts_[config_.level_to_group(old_level)]--;
  group_counts_[config_.level_to_group(new_level)]++;
}

int LDSState::group_of(VertexId v) const {
  return config_.level_to_group(levels_[v]);
}

const std::vector<VertexId>& LDSState::vertices_in_level(int level) const {
  return level_buckets_[config_.clamp_level(level)];
}

std::size_t LDSState::num_vertices() const { return levels_.size(); }

int LDSState::group_count(int group_id) const {
  if (group_id < 0 || group_id >= static_cast<int>(group_counts_.size())) {
    return 0;
  }
  return group_counts_[group_id];
}

}  // namespace seq
