#pragma once

#include <cstddef>
#include <vector>

#include "core/graph.h"

namespace seq {

struct DescriptorState {
  explicit DescriptorState(std::size_t num_vertices = 0);

  std::vector<VertexId> parent;
  std::vector<int> old_level;
  std::vector<std::size_t> last_touched_batch;
  std::size_t batch_number = 0;

  void ensure_vertices(std::size_t num_vertices);
  void begin_batch(const std::vector<int>& levels_before);
  void note_vertex_touched(VertexId v);
};

}  // namespace seq
