#pragma once

#include <cstddef>
#include <unordered_set>
#include <vector>

namespace seq {

using VertexId = std::size_t;

struct Graph {
  explicit Graph(std::size_t num_vertices = 0);

  bool add_edge(VertexId u, VertexId v);
  bool remove_edge(VertexId u, VertexId v);
  bool has_edge(VertexId u, VertexId v) const;

  const std::vector<VertexId>& neighbors(VertexId v) const;
  std::size_t num_vertices() const;

 private:
  void ensure_vertex(VertexId v);

  std::vector<std::unordered_set<VertexId>> adjacency_set_;
  std::vector<std::vector<VertexId>> adjacency_cache_;
};

}  // namespace seq
