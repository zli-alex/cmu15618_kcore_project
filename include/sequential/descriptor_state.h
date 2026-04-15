#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>

#include "core/graph.h"

namespace seq {

struct DescriptorState {
  explicit DescriptorState(std::size_t num_vertices = 0);

  std::vector<VertexId> parent;
  std::vector<int> old_level;
  std::vector<std::uint8_t> marked;
  std::vector<std::uint8_t> first_mutation_recorded;
  std::vector<std::size_t> last_touched_batch;
  std::vector<VertexId> last_unmark_order;
  std::size_t batch_number = 0;
  VertexId last_mutated_vertex = 0;
  bool has_last_mutated_vertex = false;

  void ensure_vertices(std::size_t num_vertices);
  void begin_batch(const std::vector<int>& levels_before);
  void on_level_mutation_start(VertexId v, int level_before);
  void unmark_all_roots_first();
  void note_vertex_touched(VertexId v);
};

}  // namespace seq
