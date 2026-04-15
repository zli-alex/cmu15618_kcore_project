#include "sequential/descriptor_state.h"

namespace seq {

DescriptorState::DescriptorState(std::size_t num_vertices)
    : parent(num_vertices, 0),
      old_level(num_vertices, 0),
      last_touched_batch(num_vertices, 0) {
  for (VertexId v = 0; v < num_vertices; ++v) {
    parent[v] = v;
  }
}

void DescriptorState::ensure_vertices(std::size_t num_vertices) {
  const std::size_t before = parent.size();
  if (num_vertices <= before) {
    return;
  }
  parent.resize(num_vertices);
  old_level.resize(num_vertices, 0);
  last_touched_batch.resize(num_vertices, 0);
  for (VertexId v = before; v < num_vertices; ++v) {
    parent[v] = v;
  }
}

void DescriptorState::begin_batch(const std::vector<int>& levels_before) {
  ++batch_number;
  old_level = levels_before;
  if (last_touched_batch.size() < levels_before.size()) {
    last_touched_batch.resize(levels_before.size(), 0);
  }
}

void DescriptorState::note_vertex_touched(VertexId v) {
  if (v >= last_touched_batch.size()) {
    return;
  }
  last_touched_batch[v] = batch_number;
}

}  // namespace seq
