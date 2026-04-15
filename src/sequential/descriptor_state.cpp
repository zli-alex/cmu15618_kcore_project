#include "sequential/descriptor_state.h"

#include <algorithm>
#include <unordered_set>

namespace seq {
namespace {

VertexId find_root_vertex(const std::vector<VertexId>& parent, VertexId v) {
  if (v >= parent.size()) {
    return v;
  }
  VertexId current = v;
  for (std::size_t steps = 0; steps < parent.size(); ++steps) {
    const VertexId p = parent[current];
    if (p >= parent.size() || p == current) {
      return current;
    }
    current = p;
  }
  return current;
}

}  // namespace

DescriptorState::DescriptorState(std::size_t num_vertices)
    : parent(num_vertices, 0),
      old_level(num_vertices, 0),
      marked(num_vertices, 0),
      first_mutation_recorded(num_vertices, 0),
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
  marked.resize(num_vertices, 0);
  first_mutation_recorded.resize(num_vertices, 0);
  last_touched_batch.resize(num_vertices, 0);
  for (VertexId v = before; v < num_vertices; ++v) {
    parent[v] = v;
  }
}

void DescriptorState::begin_batch(const std::vector<int>& levels_before) {
  ++batch_number;
  old_level = levels_before;
  marked.assign(levels_before.size(), 0);
  first_mutation_recorded.assign(levels_before.size(), 0);
  last_unmark_order.clear();
  has_last_mutated_vertex = false;
  if (last_touched_batch.size() < levels_before.size()) {
    last_touched_batch.resize(levels_before.size(), 0);
  }
  if (parent.size() < levels_before.size()) {
    parent.resize(levels_before.size());
  }
  for (VertexId v = 0; v < levels_before.size(); ++v) {
    parent[v] = v;
  }
}

void DescriptorState::on_level_mutation_start(VertexId v, int level_before) {
  if (v >= old_level.size()) {
    return;
  }
  if (first_mutation_recorded[v] == 0) {
    old_level[v] = level_before;
    first_mutation_recorded[v] = 1;
    if (!has_last_mutated_vertex) {
      parent[v] = v;
      last_mutated_vertex = v;
      has_last_mutated_vertex = true;
    } else if (v == last_mutated_vertex) {
      parent[v] = v;
    } else {
      parent[v] = last_mutated_vertex;
      last_mutated_vertex = v;
    }
  }
  marked[v] = 1;
  note_vertex_touched(v);
}

void DescriptorState::unmark_all_roots_first() {
  std::vector<VertexId> roots;
  std::vector<VertexId> non_roots;
  roots.reserve(marked.size());
  non_roots.reserve(marked.size());
  std::unordered_set<VertexId> seen_roots;

  for (VertexId v = 0; v < marked.size(); ++v) {
    if (marked[v] == 0) {
      continue;
    }
    const VertexId root = find_root_vertex(parent, v);
    if (root == v) {
      if (seen_roots.insert(v).second) {
        roots.push_back(v);
      }
    } else {
      non_roots.push_back(v);
    }
  }

  std::sort(roots.begin(), roots.end());
  std::sort(non_roots.begin(), non_roots.end());

  last_unmark_order.clear();
  last_unmark_order.reserve(roots.size() + non_roots.size());
  for (VertexId v : roots) {
    marked[v] = 0;
    last_unmark_order.push_back(v);
  }
  for (VertexId v : non_roots) {
    marked[v] = 0;
    last_unmark_order.push_back(v);
  }
}

void DescriptorState::note_vertex_touched(VertexId v) {
  if (v >= last_touched_batch.size()) {
    return;
  }
  last_touched_batch[v] = batch_number;
}

}  // namespace seq
