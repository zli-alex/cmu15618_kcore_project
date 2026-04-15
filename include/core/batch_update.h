#pragma once

#include <vector>

#include "core/graph.h"

namespace seq {

enum class UpdateType { Insert, Delete };

struct EdgeUpdate {
  UpdateType type;
  VertexId u;
  VertexId v;
};

struct BatchUpdate {
  std::vector<EdgeUpdate> updates;
};

struct PartitionedUpdates {
  std::vector<EdgeUpdate> insertions;
  std::vector<EdgeUpdate> deletions;
};

PartitionedUpdates partition_insertions_first(const BatchUpdate& batch);

}  // namespace seq
