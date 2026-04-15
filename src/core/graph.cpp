#include "core/graph.h"

#include <algorithm>

namespace seq {

Graph::Graph(std::size_t num_vertices)
    : adjacency_set_(num_vertices), adjacency_cache_(num_vertices) {}

void Graph::ensure_vertex(VertexId v) {
  if (v < adjacency_set_.size()) {
    return;
  }
  adjacency_set_.resize(v + 1);
  adjacency_cache_.resize(v + 1);
}

bool Graph::add_edge(VertexId u, VertexId v) {
  if (u == v) {
    return false;
  }
  ensure_vertex(std::max(u, v));
  const bool inserted_uv = adjacency_set_[u].insert(v).second;
  const bool inserted_vu = adjacency_set_[v].insert(u).second;
  if (!inserted_uv || !inserted_vu) {
    return false;
  }
  adjacency_cache_[u].assign(adjacency_set_[u].begin(), adjacency_set_[u].end());
  adjacency_cache_[v].assign(adjacency_set_[v].begin(), adjacency_set_[v].end());
  return true;
}

bool Graph::remove_edge(VertexId u, VertexId v) {
  if (u >= adjacency_set_.size() || v >= adjacency_set_.size()) {
    return false;
  }
  const bool erased_uv = adjacency_set_[u].erase(v) > 0;
  const bool erased_vu = adjacency_set_[v].erase(u) > 0;
  if (!erased_uv || !erased_vu) {
    return false;
  }
  adjacency_cache_[u].assign(adjacency_set_[u].begin(), adjacency_set_[u].end());
  adjacency_cache_[v].assign(adjacency_set_[v].begin(), adjacency_set_[v].end());
  return true;
}

bool Graph::has_edge(VertexId u, VertexId v) const {
  if (u >= adjacency_set_.size() || v >= adjacency_set_.size()) {
    return false;
  }
  return adjacency_set_[u].find(v) != adjacency_set_[u].end();
}

const std::vector<VertexId>& Graph::neighbors(VertexId v) const {
  return adjacency_cache_[v];
}

std::size_t Graph::num_vertices() const { return adjacency_set_.size(); }

}  // namespace seq
