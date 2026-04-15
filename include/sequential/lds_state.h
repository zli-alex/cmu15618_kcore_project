#pragma once

#include <cstddef>
#include <vector>

#include "core/graph.h"
#include "sequential/lds_config.h"

namespace seq {

struct LDSState {
  LDSState(std::size_t num_vertices, const LDSConfig& config, int initial_level = 0);

  int level(VertexId v) const;
  void set_level(VertexId v, int new_level);

  int group_of(VertexId v) const;
  const std::vector<VertexId>& vertices_in_level(int level) const;
  std::size_t num_vertices() const;

  int group_count(int group_id) const;

 private:
  const LDSConfig& config_;
  std::vector<int> levels_;
  std::vector<std::vector<VertexId>> level_buckets_;
  std::vector<std::size_t> position_in_bucket_;
  std::vector<int> group_counts_;
};

}  // namespace seq
